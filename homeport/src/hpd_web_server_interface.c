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
 * @file hpd_web_server_interface.c
 * @brief  Methods for managing the Web Server
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include "hpd_web_server_interface.h"
#include "hpd_error.h"
#include "hpd_configure.h"

#include <stdarg.h>

struct lr *unsecure_web_server;

static int req_destroy_str(void *srv_data, void **req_data,
                           struct lr_request *req)
{
   free(*req_data);
   return 0;
}

void unregister_socket(struct event_socket *s)
{
   lr_unregister_service(unsecure_web_server, s->url);
}

static int req_destroy_socket(void *srv_data, void **req_data,
                              struct lr_request *req)
{
   lr_send_stop(req);
   // TODO For now we just close socket on connection lost
   const char *url = lr_request_get_url(req); 
   void *socket = lr_unregister_service(unsecure_web_server, url);
   destroy_socket(socket);
   return 0;
}

static int answer_get_devices(void *srv_data, void **req_data,
                              struct lr_request *req,
                              const char *body, size_t len)
{
   char *xmlbuff = get_xml_device_list();
   struct lm *headers = lm_create();

   lm_insert(headers, "Content-Type", "text/xml");
   lr_sendf(req, WS_HTTP_200, headers, xmlbuff);

   lm_destroy(headers);
   free(xmlbuff);
   return 0;
}

void send_event(struct event_socket *s, const char *fmt, ...)
{
   struct lr_request *req = s->req;
   const char *ip = lr_request_get_ip(req);
   printf("Send value change: %s\n", ip);
   va_list arg;
   va_start(arg, fmt);
   lr_send_vchunkf(req, fmt, arg);
   va_end(arg);
}

static int answer_get_event_socket(void *srv_data, void **req_data,
                                   struct lr_request *req,
                                   const char *body, size_t len)
{
   lr_request_keep_open(req);
   lr_send_start(req, WS_HTTP_200, NULL);
   open_event_socket(srv_data, req);
   return 0;
}

static int answer_post_events(void *srv_data, void **req_data,
                              struct lr_request *req,
                              const char *body, size_t len)
{
   int rc;
   struct event_socket *socket;
   char *str;
   char *req_str = *req_data;
   struct ev_loop *loop = srv_data;

   // Recieve data
   if (body) {
      if (*req_data) len += strlen(req_str);
      str = realloc(*req_data, (len+1)*sizeof(char));
      if (!str) {
         printf("Failed to allocate memory\n");
         lr_sendf(req, WS_HTTP_500, NULL, "Internal server error");
         return 0;
      }
      strncat(str, body, len);
      str[len] = '\0';
      *req_data = str;
      return 0;
   }
   
   // Subscribe to events
   socket = subscribe_to_events(*req_data, loop);

   // Register new url in libREST
   rc = lr_register_service(unsecure_web_server,
                            socket->url,
                            answer_get_event_socket, NULL, NULL, NULL,
                            req_destroy_socket, socket);
   if (rc) {
      printf("Failed to register new event url\n");
      lr_sendf(req, WS_HTTP_500, NULL, "Internal server error");
      return 0;
   }
   
   // Send respond
   // TODO Fix body to corrispond with RFC 2616 (holds for all other
   // bodies too.
   struct lm *headers = lm_create();
   lm_insert(headers, "Location", socket->url);
   // TODO CORS HEADER - SHOULD BE PACKED INSIDE SETTINGS OR COMPILE FLAG
   lm_insert(headers, "Access-Control-Expose-Headers", "Location");
   lr_sendf(req, WS_HTTP_201, headers, "Created");
   lm_destroy(headers);
   
   return 0;
}

// TODO Do I need to add more to this (like logging, etc.)
static int answer_get(void *srv_data, void **req_data,
                      struct lr_request *req,
                      const char *body, size_t len)
{
   Service *service = srv_data;
   char *buffer, *xmlbuff;
   const char *arg, *url, *ip;
   enum http_method method;
   int buf_len;

   // Check if allowed
   if (!service->get_function) {
      lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
      return 1;
   }

   // Check arguments
   arg = lr_request_get_argument(req, "x");
   if (arg) arg = "x=1";
   else {
      arg = lr_request_get_argument(req, "p");
      if (arg) arg = "p=1";
   }

   // Log request
   method = lr_request_get_method(req);
   url = lr_request_get_url(req);
   ip = lr_request_get_ip(req);
   Log (HPD_LOG_ONLY_REQUESTS, NULL, ip, http_method_str(method), url, arg);

   // Argument "x=1"
   if (arg && strcmp(arg, "x=1") == 0) {
      xmlbuff = extract_service_xml(service);
      lr_sendf(req, WS_HTTP_200, NULL, xmlbuff);
      free(xmlbuff);
      return 0;
   }

   // Call callback and send response
   buffer = malloc((MHD_MAX_BUFFER_SIZE+1) * sizeof(char));
   buf_len = service->get_function(service, buffer, MHD_MAX_BUFFER_SIZE);
   if (buf_len) {
      buffer[buf_len] = '\0';
      xmlbuff = get_xml_value(buffer);
      lr_sendf(req, WS_HTTP_200, NULL, xmlbuff);
      free(xmlbuff);
   } else {
      lr_sendf(req, WS_HTTP_500, NULL, "Internal Server Error");
   }
   free(buffer);

   return 0;
}

// TODO Do I need to add more to this (like logging, etc.)
static int answer_put(void *srv_data, void **req_data,
                      struct lr_request *req,
                      const char *body, size_t len)
{
   Service *service = srv_data;
   char *new_put;
   size_t new_len;

   // Check if allowed
   if (!service->get_function) {
      lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
      return 1;
   }

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
      // Get value from XML TODO Returns NULL if no value in message !!!
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
 * Start the MHD web server(s) and the AVAHI client or server
 *
 * @param hostname Hostname for the local address of the server
 *
 * @param domain_name Domain name for the local address of the server (if NULL = .local)
 *
 * @return A HPD error code
 */
int 
start_server(char* hostname, char *domain_name, struct ev_loop *loop)
{
	int rc;

	rc = init_xml_file (XML_FILE_NAME,DEVICE_LIST_ID);
	if( rc < 0 )
	{	
		printf("Failed to initiate XML file\n");
		return rc;
	}

   struct lr_settings settings = LR_SETTINGS_DEFAULT;
   settings.port = hpd_daemon->http_port;

	unsecure_web_server = lr_create(&settings, loop);
   if (!unsecure_web_server)
      return HPD_E_MHD_ERROR;

   if (lr_start(unsecure_web_server))
      return HPD_E_MHD_ERROR;

   rc = lr_register_service(unsecure_web_server,
                            "/devices",
                            answer_get_devices, NULL, NULL, NULL,
                            NULL, NULL);
   rc = lr_register_service(unsecure_web_server,
                            "/events",
                            NULL, answer_post_events, NULL, NULL,
                            req_destroy_str, loop);
   if (rc) {
      printf("Failed to register non secure service\n");
		return HPD_E_MHD_ERROR;
   }

   rc = HPD_E_SUCCESS;

#if USE_AVAHI
	rc = avahi_start (hostname, domain_name);
	if( rc < 0 )
	{
		printf("Failed to start avahi\n");
		return rc;
	}	
#endif

	return rc;
}

/**
 * Stop the MHD web server(s) and the AVAHI client or server
 *
 * @return A HPD error code
 */
int 
stop_server()
{
	int rc;

#if HPD_HTTP
   lr_stop(unsecure_web_server);
   lr_destroy(unsecure_web_server);
	rc = HPD_E_SUCCESS;
#endif

	delete_xml(XML_FILE_NAME);

#if USE_AVAHI
	avahi_quit ();
#endif

	return rc;

}

/**
 * Add a service to the XML file, the server(s), and the AVAHI client or server
	 *
 * @param service_to_register The service to register
 *
 * @return A HPD error code
 */
int 
register_service( Service *service_to_register )
{

	int rc;

	if( service_to_register->device->secure_device == HPD_NON_SECURE_DEVICE )
	{
      Service *s = lr_lookup_service(unsecure_web_server, service_to_register->value_url);
		if (s) {
			printf("A similar service is already registered in the unsecure server\n");
			return HPD_E_SERVICE_ALREADY_REGISTER;
		}

		printf("Registering non secure service\n");
		rc = lr_register_service(unsecure_web_server,
                               service_to_register->value_url,
                               answer_get, NULL, answer_put, NULL,
                               NULL, service_to_register);
		if(rc) {
         printf("Failed to register non secure service\n");
			return HPD_E_MHD_ERROR;
      }
	} else
      return HPD_E_BAD_PARAMETER;

	/* Add to XML */
	rc = add_service_to_xml ( service_to_register );
	if (rc == -1){
		printf("Impossible to add the Service to the XML file.\n");
		return HPD_E_XML_ERROR;
	}
	else if(rc == -2){
		printf("The Service already exists\n");
	}
#if USE_AVAHI
	rc = avahi_create_service ( service_to_register );
	if(  rc < HPD_E_SUCCESS )
	{
		printf("avahi_create_service failed : %d\n", rc);
		return rc;
	}
#endif
	rc = notify_service_availability( service_to_register, HPD_YES);
	if(  rc < HPD_E_SUCCESS )
	{
		printf("notify_service_availability failed : %d\n", rc);
		return rc;
	}

	return HPD_E_SUCCESS;
}

/**
 * Remove a service from the XML file, the server(s), and the AVAHI client or server
	 *
 * @param service_to_unregister The service to remove
 *
 * @return A HPD error code
 */
int 
unregister_service( Service *service_to_unregister )
{

	int rc;

	if( service_to_unregister->device->secure_device == HPD_NON_SECURE_DEVICE )
	{
      Service *s = lr_lookup_service(unsecure_web_server, service_to_unregister->value_url);
	   if( !s )
		   return HPD_E_SERVICE_NOT_REGISTER;

		s = lr_unregister_service ( unsecure_web_server,
            service_to_unregister->value_url );
		if( !s )
			return HPD_E_MHD_ERROR;
	}
	else 
		return HPD_E_BAD_PARAMETER;


	rc = remove_service_from_XML( service_to_unregister );
	if( rc < HPD_E_SUCCESS )
	{
		printf("remove_service_from_xml failed : %d\n", rc);
		return HPD_E_XML_ERROR;
	}
#if USE_AVAHI
	rc = avahi_remove_service ( service_to_unregister );
	if(  rc < HPD_E_SUCCESS )
	{
		printf("avahi_remove_service failed : %d\n", rc);
		return rc;
	}
#endif
	rc = notify_service_availability( service_to_unregister, HPD_NO);
	if(  rc < HPD_E_SUCCESS )
	{
		printf("notify_service_availability failed : %d\n", rc);
		return rc;
	}

	return HPD_E_SUCCESS;
}

/**
 * Register all of a device's services
 *
 * @param device_to_register The device to register
 *
 * @return A HPD error code
 */
int 
register_device_services( Device *device_to_register )
{
	ServiceElement *iterator;
	int return_value;

	DL_FOREACH( device_to_register->service_head, iterator )
	{
		return_value = register_service( iterator->service );
		if( return_value < HPD_E_SUCCESS )
		{
			return return_value;
		}
	}

	return HPD_E_SUCCESS;

}

/**
 * Unregister all of a device's services
 *
 * @param device_to_unregister The device to unregister
 *
 * @return A HPD error code
 */
int 
unregister_device_services( Device *device_to_unregister )
{
	ServiceElement *iterator;
	int return_value;

	DL_FOREACH( device_to_unregister->service_head, iterator )
	{
		return_value = unregister_service( iterator->service );
		if(return_value < HPD_E_SUCCESS)
		{
			return return_value;
		}
	}


	return HPD_E_SUCCESS;
}

/**
 * Check if a service is registered in a server
 *
 * @param service The service to check
 *
 * @return A HPD error code
 */
int 
is_service_registered( Service *service )
{

	if( service == NULL )
		return HPD_E_NULL_POINTER;

	if( service->device->secure_device == HPD_NON_SECURE_DEVICE )
	{
      Service *s = lr_lookup_service(unsecure_web_server, service->value_url);
      if (s) return 1;
      else return 0;
	}	

	return 0;
}

/**
 * Retrieve a Service object from a server
 *
 * @param device_type The type of the device that owns the service
 *
 * @param device_ID The ID of the device that owns the service
 *
 * @param service_type The type of the service
 *
 * @param service_ID The ID of the service 
 *
 * @return A pointer on the Service struct if found, NULL otherwise
 */
Service* 
get_service( char *device_type, char *device_ID, char *service_type, char *service_ID )
{
	Service *service = NULL;

   // TODO This is not the best solution (duplicated code), nor the best
   // place to do this
	char *value_url = malloc(sizeof(char)*( strlen("/") + strlen(device_type) + strlen("/") 
	                                           + strlen(device_ID) +
                                              strlen("/") +
                                              strlen(service_type)
	                                           + strlen("/") +
                                              strlen(service_ID) + 1 ) );
	sprintf( value_url,"/%s/%s/%s/%s", device_type, device_ID, service_type,
	         service_ID );
	service = lr_lookup_service(unsecure_web_server, value_url);
   free(value_url);
	return service;
}


/**
 * Retrieve a Device object from a server
 *
 * @param device_type The type of the device
 *
 * @param device_ID The ID of the device
 *
 * @return A pointer on the Device struct if found, NULL otherwise
 */
Device* 
get_device( char *device_type, char *device_ID)
{
	Device *device = NULL;
	Service *service = NULL;

   // TODO This is not the best solution (duplicated code), nor the best
   // place to do this
   // TODO BUG DOES NOT HAVE A SERVER ID AND TYPE HERE !!!
	char *value_url = malloc(sizeof(char)*( strlen("/") + strlen(service->device->type) + strlen("/") 
	                                           + strlen(service->device->ID) + strlen("/") + strlen(service->type)
	                                           + strlen("/") + strlen(service->ID) + 1 ) );
	sprintf( value_url,"/%s/%s/%s/%s", service->device->type, service->device->ID, service->type,
	         service->ID );
	service = lr_lookup_service(unsecure_web_server, value_url);
   free(value_url);
	device = service->device;
	return device;
}
