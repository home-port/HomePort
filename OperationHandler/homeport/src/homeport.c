/*Copyright 2011 Aalborg University. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of
  conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list
  of conditions and the following disclaimer in the documentation and/or other materials
  provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed*/

/**
 * @file homeport.c
 * @brief  Methods for managing HomePort Daemon
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include <time.h>

#include "homeport.h"
#include "hpd_error.h"
#include "hp_macros.h"

/** Client Interface */
int homePortGetState(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len);
int homePortSetState(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len);
int homePortGetConfiguration(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len);

/** Private function declaration **/
int homePortRegisterService(HomePort *homeport, Service *service, char *uri);
int homePortUnregisterService( HomePort *homeport, char *uri );
char *timestamp ( void );
char* stateToXml(char *state);
char* xmlToState(char *xml);
char* stateToJson(char *state);
char* jsonToState(char *xml);
static void sig_cb ( struct ev_loop *loop, struct ev_signal *w, int revents );
char* generateUri( Device *device, Service *service );

/**
 * Creates a HomePort structure
 * 
Not Acceptable
 * @param option HPD option as specified in homeport.h
 *
 * @param hostname Name of the desired host
 *
 * @param ... va list of option, last option has to be HPD_OPTION_END
 *
 * @return A HPD error code
 */
HomePort*
homePortNew( struct ev_loop *loop, int port )
{
  HomePort *homeport;
  alloc_struct(homeport);

  struct lr_settings settings = LR_SETTINGS_DEFAULT;
  settings.port = port;

  homeport->loop = loop;

  homeport->rest_interface = lr_create(&settings, loop);
  if ( homeport->rest_interface == NULL )
  {
    goto cleanup;
  }

  homeport->configuration = configurationNew();
  if( homeport->configuration == NULL )
  {
    goto cleanup;
  }

  return homeport;
cleanup:
  homePortFree(homeport);
  return NULL;
}

/**
 * 
 * Destroys a HomePort struct
 * @return A HPD error code
 */
void 
homePortFree(HomePort *homeport)
{
  if(homeport != NULL)
  {
    lr_destroy(homeport->rest_interface);
    configurationFree(homeport->configuration);
    free(homeport);
  }
}

int
homePortStart(HomePort *homeport)
{
  if(lr_start(homeport->rest_interface))
    return HPD_E_MHD_ERROR;

  return lr_register_service(homeport->rest_interface,
      "/devices",
      homePortGetConfiguration, NULL, NULL, NULL,
      NULL, homeport);
}

void
homePortStop(HomePort *homeport)
{
  lr_stop(homeport->rest_interface);
}

int
homePortEasy( int (*init)(HomePort *homeport, void *data), void (*deinit)(HomePort *homeport, void *data), void *data, int port )
{
  int rc;

  // Create loop
  struct ev_loop *loop = EV_DEFAULT;

  HomePort *homeport = homePortNew(loop, port);

  // Create signal watchers
  struct ev_signal sigint_watcher;
  struct ev_signal sigterm_watcher;
  ev_signal_init(&sigint_watcher, sig_cb, SIGINT);
  ev_signal_init(&sigterm_watcher, sig_cb, SIGTERM);
  void **w_data = malloc(3*sizeof(void *));
  w_data[0] = homeport;
  w_data[1] = deinit;
  w_data[2] = data;
  sigint_watcher.data = w_data;
  sigterm_watcher.data = w_data;
  ev_signal_start(loop, &sigint_watcher);
  ev_signal_start(loop, &sigterm_watcher);

  // Call init
  if ((rc = init(homeport, data))) return rc;

  if( ( rc = homePortStart(homeport) ) ) return rc;

  // Start loop
  ev_run(loop, 0);

  return 0;
}

/*******************************************************************/
/*********************Configurator Interface************************/
/*******************************************************************/
int
homePortAddAdapter(HomePort *homeport, Adapter *adapter)
{
  return configurationAddAdapter(homeport->configuration, adapter);
}

int
homePortRemoveAdapter(HomePort *homeport, Adapter *adapter)
{
  return configurationRemoveAdapter(homeport->configuration, adapter);
}

int 
homePortAttachDevice( HomePort *homeport, Adapter *adapter, Device *device )
{
  if( homeport == NULL || adapter == NULL || device == NULL )
  {
    return HPD_E_NULL_POINTER;
  }
  ServiceElement *iterator;
  int rc;
  char *uri;

  if( findAdapter(homeport->configuration, adapter->id) == NULL )
  {
    printf("Adapter not in the systen\n");
    return -1;
  }

  char *deviceId = confGenerateDeviceId(homeport->configuration);
  if( deviceId == NULL )
  {
    return HPD_E_MALLOC_ERROR;
  }

  if( ( rc = adapterAddDevice(adapter, device, deviceId) ) )
  {
    return rc;
  }

  DL_FOREACH( device->service_head, iterator )
  {
    uri = generateUri( device, iterator->service );
    if( uri == NULL )
    {
      return HPD_E_MALLOC_ERROR;
    }

    serviceSetUri(iterator->service, uri);

    rc = homePortRegisterService(homeport, iterator->service, uri );
    if( rc < HPD_E_SUCCESS )
    {
      return rc;
    }
  }

  return HPD_E_SUCCESS;
}

int 
homePortDetachDevice( HomePort *homeport, Adapter *adapter, Device *device )
{
  if( homeport == NULL || adapter == NULL || device == NULL )
    return HPD_E_NULL_POINTER;

  ServiceElement *iterator;
  int rc;

  DL_FOREACH( device->service_head, iterator )
  {
    rc = homePortUnregisterService( homeport, iterator->service->uri );
    if(rc < HPD_E_SUCCESS)
    {
      return rc;
    }
  }

  adapterRemoveDevice(adapter, device);

  return HPD_E_SUCCESS;
}

/*******************************************************************/
/************************Client Interface***************************/
/*******************************************************************/

void
homePortSendState(Service *service, void *req_in, const char *val, size_t len)
{
   char *buffer, *state;
   struct lr_request *req = req_in;
   struct lm *headersIn = lr_request_get_headers(req);
   struct lm *headers;

   // Call callback and send response
   buffer = malloc((len+1) * sizeof(char));
   if (len) {
     buffer[len] = '\0';
     /*TODO Check header for XML or jSON*/
     char *accept = lm_find( headersIn, "Accept" );
    if( strcmp(accept, "application/json") == 0 )
    {
      state = stateToJson(buffer);
      headers =  lm_create();
      lm_insert(headers, "Content-Type", "application/json");
      lr_sendf(req, WS_HTTP_200, headers, state);
    }
    else
    { 
      state = stateToXml(buffer);
      headers =  lm_create();
      lm_insert(headers, "Content-Type", "application/xml");
      lr_sendf(req, WS_HTTP_200, headers, state);
    }
     lm_destroy(headers);
     free(state);
   } else {
     lr_sendf(req, WS_HTTP_500, NULL, "Internal Server Error");
   }
   free(buffer);
}

int
homePortGetState(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
  Service *service = (Service*) srv_data;

  if(service->getFunction == NULL)
  {
    lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
    return 1;
  }

  service->getFunction(service, req);

  // Stop parsing request, we don't need the body anyways
  return 1;
}

int 
homePortSetState(void *srv_data, void **req_data,
    struct lr_request *req,
    const char *body, size_t len)
{
  Service *service = srv_data;
  char *req_str = *req_data;
  char *str=NULL;

  if(service->putFunction == NULL)
  {
    lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
    return 1;
  }

  // Recieve data
  if (body) {
    if (*req_data) len += strlen(req_str);
    str = realloc(*req_data, (len+1)*sizeof(char));
    if (!str) {
      printf("Failed to allocate memory\n");
      lr_sendf(req, WS_HTTP_500, NULL, "Internal server error");
      return 1;
    }
    strncpy(str, body, len);
    str[len] = '\0';
    *req_data = str;
    return 0;
  }
  struct lm *headers;
  struct lm *headersIn = lr_request_get_headers(req);
  char *contentType = lm_find(headersIn, "Content-Type");
  char *value;
  int freeValue = 1;
  if(*req_data == NULL)
  {
    lr_sendf(req, WS_HTTP_400, NULL, "400 Bad Request");
    return 1;
  }
  if( ( contentType == NULL ) || ( strcmp(contentType, "application/xml") == 0 ) )
  {
    value = xmlToState(*req_data);
  }
  else if( strcmp( contentType, "application/json" ) == 0 )
  {
    value = jsonToState(*req_data);
    freeValue = 0;
  }
  else
  {
    free(*req_data);
    lr_sendf(req, WS_HTTP_415, NULL, "415 Unsupported Media Type");
    return 0;
  }
  free(*req_data);
  if (!value) {
    lr_sendf(req, WS_HTTP_400, NULL, "400 Bad Request");
    return 1;
  }

  // Call callback
  char *buffer = malloc((MHD_MAX_BUFFER_SIZE+1) * sizeof(char));
  int buf_len = service->putFunction(service,
      buffer, MHD_MAX_BUFFER_SIZE,
      value);
  if(freeValue) free(value);

  // Send response
  if (buf_len == 0) {
    lr_sendf(req, WS_HTTP_500, NULL, "500 Internal Server Error");
    free(buffer);
    return 1;
  } else {
    //    // Send value change event
    //    const char *IP = lr_request_get_ip(req);
    //    buffer[buf_len] = '\0';
    //    send_event_of_value_change(service, buffer, IP);

    // Reply to request
    const char *ret;
    if( strcmp( contentType, "application/xml" ) == 0 )
    {
      ret = stateToXml(buffer);
    }
    else
    {
      ret = stateToJson(buffer);
    }

    lr_sendf(req, WS_HTTP_200, NULL, ret);
    free(ret);
  }
  free(buffer);
  return 0;
}

int
homePortGetConfiguration(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
  HomePort *homeport = (HomePort*) srv_data;
  struct lm *headersIn =  lr_request_get_headers( req );
  char *accept;
  char *res;

  accept = lm_find( headersIn, "Accept" );

  /** Defaults to XML */
  if( strcmp(accept, "application/json") == 0 )
  {
    json_t * configurationJson = configurationToJson( homeport->configuration );
    res = json_dumps( configurationJson, 0 );
    json_decref(configurationJson);
  }
  else 
  {
    mxml_node_t *xml = mxmlNewXML("1.0");

    configurationToXml(homeport->configuration, xml);

    res = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
    mxmlDelete(xml);

  }
//  else
//  {
//    lr_sendf(req, WS_HTTP_406, NULL, NULL);
//    return 0;
//  }
  lr_sendf(req, WS_HTTP_200, NULL, res);

  free(res);
  return 0;
}

static void
sig_cb ( struct ev_loop *loop, struct ev_signal *w, int revents )
{
  HomePort *homeport = (HomePort*)((void**)w->data)[0];
  void (*deinit)(HomePort *, void *) = ((void **)w->data)[1];

  // Call deinit
  // TODO Might be a problem that deinit is not called on ws_stop, but
  // only if the server is stopped by a signal. Note that this is only
  // used in HPD_easy way of starting the server.
  deinit(homeport, ((void **)w->data)[2]);


  // Stop server and loop
  homePortStop(homeport);
  homePortFree(homeport);
  ev_break(loop, EVBREAK_ALL);

  // TODO Isn't it bad that we down stop watcher here?
  free(w->data);
}

char *
generateUri( Device *device, Service *service )
{
  char *uri = malloc((strlen(device->type)+strlen(device->id)+strlen(service->type)+strlen(service->id)+4+1)*sizeof(char));
  if( uri == NULL )
    return NULL;
  uri[0] = '\0';
  strcat(uri, "/");
  strcat(uri, device->type);
  strcat(uri, "/");
  strcat(uri, device->id);
  strcat(uri, "/");
  strcat(uri, service->type);
  strcat(uri, "/");
  strcat(uri, service->id);

  return uri;
}

int
homePortRegisterService(HomePort *homeport, Service *service, char *uri)
{
  int rc;
  Service *s = lr_lookup_service(homeport->rest_interface, uri);
  if (s) {
    printf("A similar service is already registered in the unsecure server\n");
    return HPD_E_SERVICE_ALREADY_REGISTER;
  }

  printf("Registering service\n");
  rc = lr_register_service(homeport->rest_interface,
      uri,
      homePortGetState, NULL, homePortSetState, NULL,
      NULL, service);
  if(rc) {
    printf("Failed to register non secure service\n");
    return HPD_E_MHD_ERROR;
  }

  return HPD_E_SUCCESS;
}

int
homePortUnregisterService( HomePort *homeport, char* uri )
{
  Service *s = lr_lookup_service(homeport->rest_interface, uri);
  if( s == NULL )
    return HPD_E_SERVICE_NOT_REGISTER;

  s = lr_unregister_service ( homeport->rest_interface, uri );
  if( s == NULL )
    return HPD_E_MHD_ERROR;

  return HPD_E_SUCCESS;
}

char*
stateToXml(char *state)
{
  mxml_node_t *xml;
  mxml_node_t *stateXml;

  xml = mxmlNewXML("1.0");
  stateXml = mxmlNewElement(xml, "value");
  mxmlElementSetAttr(stateXml, "timestamp", timestamp());
  mxmlNewText(stateXml, 0, state);

  char* return_value = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
  mxmlDelete(xml);

  return return_value;
}

char*
xmlToState(char *xml_value)
{
  mxml_node_t *xml;
  mxml_node_t *node;

  xml = mxmlLoadString(NULL, xml_value, MXML_TEXT_CALLBACK);
  if(xml == NULL)
  {
    printf("XML value format uncompatible with HomePort\n");
    return NULL;
  }

  node = mxmlFindElement(xml, xml, "value", NULL, NULL, MXML_DESCEND);
  if(node == NULL || node-> child == NULL || node->child->value.text.string == NULL)
  {
    mxmlDelete(xml);
    printf("No \"value\" in the XML file\n");
    return NULL;
  }

  char *state = malloc(sizeof(char)*(strlen(node->child->value.text.string)+1));
  strcpy(state, node->child->value.text.string);

  mxmlDelete(xml);

  return state;
}

char*
stateToJson(char *state)
{
  json_t *json=NULL;
  json_t *value=NULL;

  if( ( json = json_object() ) == NULL )
  {
    goto error;
  }
  if( ( ( value = json_string(state) ) == NULL ) || ( json_object_set_new(json, "value", value) != 0 ) )
  {
    goto error;
  }

  char *ret = json_dumps( json, 0 );
  json_decref(json);
  return ret;

error:
  if(value) json_decref(value);
  if(json) json_decref(json);
  return NULL;
}

char*
jsonToState(char *json_value)
{
  json_t *json = NULL;
  json_error_t *error=NULL;
  json_t *value = NULL;

  if( ( json = json_loads(json_value, 0, error) ) == NULL )
  {
    goto error;
  }

  if( json_is_object(json) == 0 )
  {
    goto error;
  }

  if( ( value = json_object_get(json, "value") ) == NULL )
  {
    goto error;
  }

  char *ret = json_string_value(value);

  json_decref(json);

  return ret;

error:
  return NULL;
}

/**
 *  * Simple timestamp function
 *   *
 *    * @return Returns the timestamp
 *     */
char *
timestamp ( void )
{
  time_t ltime;
  ltime = time(NULL);
  const struct tm *timeptr = localtime(&ltime);

  static char wday_name[7][3] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static char mon_name[12][3] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  static char result[25];

  sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d",
      wday_name[timeptr->tm_wday],
      mon_name[timeptr->tm_mon],
      timeptr->tm_mday, timeptr->tm_hour,
      timeptr->tm_min, timeptr->tm_sec,
      1900 + timeptr->tm_year);
  return result;
}
