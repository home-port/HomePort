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


#include "homeport.h"
#include "hpd_error.h"
#include "hpd_web_server_interface.h"


/**
 * Parse the user's specified options
 *
 * @param ap va_list of options
 *
 * @return A HPD error code
 */
static int
parse_option_va( va_list ap )
{
	enum HPD_OPTION opt;
	int log_level, max_log_size;

	while( HPD_OPTION_END != ( opt = va_arg( ap, enum HPD_OPTION ) ) )
	{
		switch (opt)
		{

			case HPD_OPTION_HTTP :
#if HPD_HTTP
				hpd_daemon->http_port = va_arg( ap, int );
				break;
#else
				printf("Received HPD_OPTION_HTTP without HTTP feature enabled\n");
				return HPD_E_NO_HTTP;
#endif		
			case HPD_OPTION_HTTPS :
#if HPD_HTTPS
				hpd_daemon->https_port = va_arg( ap, int );
				hpd_daemon->server_cert_path = va_arg( ap, const char*);
				hpd_daemon->server_key_path = va_arg( ap, const char*);
				hpd_daemon->root_ca_path = va_arg( ap, const char*);
				break;
#else
				printf("Received HPD_OPTION_HTTPS without HTTPS feature enabled\n");
				return HPD_E_NO_HTTPS;
#endif		
			case HPD_OPTION_LOG :
				log_level = va_arg( ap, int );
				if( log_level < 0 || log_level > 2 )
			{
				printf("Bad value for log level\n");
				return HPD_E_BAD_PARAMETER;
			}
				max_log_size = va_arg( ap, int );
				if( max_log_size < 0 )
			{
				printf("Bad value for max log size\n");
				return HPD_E_BAD_PARAMETER;
			}
				break;

			default :
				printf("Unrecognized option\n");	
				return HPD_E_BAD_PARAMETER;

		}

	}

	return HPD_E_SUCCESS;

}

static int
HPD_vstart(unsigned int option, struct ev_loop *loop, char *hostname,
           va_list ap)
{
	int rc;

	if( (rc = HPD_init_daemon()) )
	{
		printf("Error initializing HPD_Daemon struct\n");
		return rc;
	}
#if USE_AVAHI
#if !AVAHI_CLIENT
	if( hostname == NULL )
		return HPD_E_NULL_POINTER;

	hpd_daemon->hostname = hostname;
#endif
#endif


	if( option & HPD_USE_CFG_FILE )
	{
		printf("Using config file\n");

		if( va_arg(ap, enum HPD_OPTION) != HPD_OPTION_CFG_PATH )
		{
			printf("Only HPD_OPTION_CFG_PATH msut be specified when using HPD_USE_CFG_FILE\n");

			return HPD_E_BAD_PARAMETER;
		}		

		if( (rc = HPD_config_file_init( va_arg( ap, const char* ) )) )
		{
			printf("Error loading config file %d\n", rc);
			return rc;
		}		
	}

	else
	{	
		if( (rc = parse_option_va(ap)) )
			return rc;


#if HPD_HTTP
		if( hpd_daemon->http_port == 0 )
		{
			printf("Missing HTTP port option\n");
		}
#endif

#if HPD_HTTPS		
		if(  hpd_daemon->https_port == 0 || hpd_daemon->server_cert_path == NULL
		   || hpd_daemon->server_key_path == NULL || hpd_daemon->root_ca_path == NULL ) 
		{
			printf("Missing options to launch HTTPS\n");
			return HPD_E_MISSING_OPTION;
		}
#endif

	}

	return start_server(hostname, NULL, loop);
}

/**
 * Starts the HomePort Daemon
 * 
 * @param option HPD option as specified in homeport.h
 *
 * @param hostname Name of the desired host
 *
 * @param ... va list of option, last option has to be HPD_OPTION_END
 *
 * @return A HPD error code
 */
int 
HPD_start( unsigned int option, struct ev_loop *loop, char *hostname, ... )
{
   int rc;
	va_list ap;
	va_start( ap, hostname);
   rc = HPD_vstart(option, loop, hostname, ap);
	va_end(ap);	
   return rc;
}

/**
 * Stops the HomePort Daemon
 *
 * @return A HPD error code
 */
int 
HPD_stop()
{
	HPD_config_deinit();
	return stop_server();
}

int 
HPD_attach_device( Device *device )
{
	if( device == NULL )
	{
		return HPD_E_NULL_POINTER;
	}
	return register_device_services( device_to_register );

}


/**
 * Unregisters all the Services contained in a given
 *  Device in the HomePort Daemon
 *
 * @param device The device to detach 
 *
 * @return A HPD error code
 */
int 
HPD_detach_device( Device *device )
{
	if( device == NULL )
		return HPD_E_NULL_POINTER;

	return unregister_device_services( device_to_unregister );
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

char*
stateToXml(char *state)
{
  mxml_node_t *xml;
  mxml_node_t *stateXml;

  xml = mxmlNewXML("1.0");
  xml_value = mxmlNewElement(xml, "value");
  mxmlElementSetAttr(xml_value, "timestamp", timestamp());
  mxmlNewText(xml_value, 0, state);

  char* return_value = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
  mxmlDelete(xml);
  
  return return_value;
}

int
HPD_get_state(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
  Service *service = (Service*) srv_data;
  char *buffer, state;

  if(service->getfunction == NULL)
  {
    lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
    return 1;
  }
  // Call callback and send response
  buffer = malloc((MHD_MAX_BUFFER_SIZE+1) * sizeof(char));
  buf_len = service->get_function(service, buffer, MHD_MAX_BUFFER_SIZE);
  if (buf_len) {
    buffer[buf_len] = '\0';
    /*TODO Check header for XML or jSON*/
    state = stateToXml(buffer);
    lm_insert(headers, "Content-Type", "application/xml");
    lr_sendf(req, WS_HTTP_200, headers, xmlbuff);
    free(xmlbuff);
  } else {
    lr_sendf(req, WS_HTTP_500, NULL, "Internal Server Error");
  }
  free(buffer);

  return 0;
}

int 
HPD_set_state(void *srv_data, void **req_data,
                      struct lr_request *req,
                      const char *body, size_t len)
{
   Service *service = srv_data;
   char *new_put;
   size_t new_len;

   // Check if allowed
   if (service->put_function == NULL) {
      lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
      return 1;
   }

/*TODO What is this crap with put value???*/
   if (body) {
      new_len = len+1;
      if (service->put_value) new_len += strlen(service->put_value);
      new_put = realloc(service->put_value, new_len*sizeof(char));
      if (!new_put) {
         free(service->put_value);
         service->put_value = NULL;
         lr_sendf(req, WS_HTTP_500, NULL, "500 Internal Server Error");
         return 1;
      }

      strncpy(new_put, body, len);
      new_put[new_len-1] = '\0';
      service->put_value = new_put;
   } else {
      if (service->put_value == NULL) {
         lr_sendf(req, WS_HTTP_400, NULL, "400 Bad Request");
         return 1;
      }
      char *value = get_value_from_xml_value(service->put_value);
      free(service->put_value);
      service->put_value = NULL;
      if (!value) {
         lr_sendf(req, WS_HTTP_400, NULL, "400 Bad Request");
         return 1;
      }

      // Call callback
      char *buffer = malloc((MHD_MAX_BUFFER_SIZE+1) * sizeof(char));
      int buf_len = service->put_function(service,
                                          buffer, MHD_MAX_BUFFER_SIZE,
                                          value);
      free(value);

      // Send response
      if (buf_len == 0) {
         lr_sendf(req, WS_HTTP_500, NULL, "500 Internal Server Error");
         free(buffer);
         return 1;
      } else {
         // Send value change event
         const char *IP = lr_request_get_ip(req);
         buffer[buf_len] = '\0';
         send_event_of_value_change(service, buffer, IP);
         
         // Reply to request
         const char *xmlbuff = get_xml_value(buffer);
         lr_sendf(req, WS_HTTP_200, NULL, xmlbuff);
      }
      free(buffer);
   }

   return 0;
}


/**
 * Gets a device given its uniqueness identifiers
 *
 * @param device_type The type of the device 
 *
 * @param device_ID The ID of the device 
 *
 * @return The desired Device or NULL if failed
 */
Device* 
HPD_get_device( char *device_type, char *device_ID )
{
	if( device_type == NULL || device_ID == NULL )
		return NULL;

	return get_device( device_type, device_ID );
}

/**
 * Sends an event of a changing of value from a given Service
 *
 * @param service_changed the Service that has its value changed
 *
 * @param updated_value The new value.
 *
 * @return HPD_E_SUCCESS if successful
 */
int 
HPD_send_event_of_value_change ( Service *service_changed, char *updated_value )
{
	return send_event_of_value_change (service_changed, updated_value, NULL);
}

static void
sig_cb ( struct ev_loop *loop, struct ev_signal *w, int revents )
{
   void (*deinit)(struct ev_loop *, void *) = ((void **)w->data)[0];

   // Call deinit
   // TODO Might be a problem that deinit is not called on ws_stop, but
   // only if the server is stopped by a signal. Note that this is only
   // used in HPD_easy way of starting the server.
   deinit(loop, ((void **)w->data)[1]);

   // Stop server and loop
   HPD_stop();
   ev_break(loop, EVBREAK_ALL);

   // TODO Isn't it bad that we down stop watcher here?
   free(w->data);
}

int
HPD_easy ( int (*init)(struct ev_loop *loop, void *data), void (*deinit)(struct ev_loop *loop, void *data), void *data,
           unsigned int option, char *hostname, ... )
{
   int rc;

   // Create loop
   struct ev_loop *loop = EV_DEFAULT;

   // Create signal watchers
   struct ev_signal sigint_watcher;
   struct ev_signal sigterm_watcher;
   ev_signal_init(&sigint_watcher, sig_cb, SIGINT);
   ev_signal_init(&sigterm_watcher, sig_cb, SIGTERM);
   void **w_data = malloc(2*sizeof(void *));
   w_data[0] = deinit;
   w_data[1] = data;
   sigint_watcher.data = w_data;
   sigterm_watcher.data = w_data;
   ev_signal_start(loop, &sigint_watcher);
   ev_signal_start(loop, &sigterm_watcher);

   // Start homeport server
	va_list ap;
	va_start( ap, hostname);
   rc = HPD_vstart(option, loop, hostname, ap);
	va_end(ap);
   if (rc) return rc;

   // Call init
   if ((rc = init(loop, data))) return rc;

   // Start loop
   ev_run(loop, 0);

   return 0;
}

