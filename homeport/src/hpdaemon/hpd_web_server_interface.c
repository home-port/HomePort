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

#if HPD_HTTP
struct lr *unsecure_web_server;
#endif

#if HPD_HTTPS
HPD_web_server_struct *secure_web_server;
#endif

/**
 * Creates a HPD_web_server_struct and allocate memory for it, should not be used anywhere but 
 * in this file (static).
 *
 * @param is_secure HPD_IS_SECURE_CONNECTION or HPD_IS_UNSECURE_CONNECTION
 *
 * @return A newly allocated HPD_web_server_struct pointer or NULL if an error occured
 */
#if HPD_HTTPS
static HPD_web_server_struct* 
create_HPD_web_server_struct( int is_secure )
{

	HPD_web_server_struct *new_HPD_web_server_struct = malloc( sizeof( HPD_web_server_struct ) );
	new_HPD_web_server_struct->service_head = NULL;
	new_HPD_web_server_struct->is_configuring = HPD_NO;
	new_HPD_web_server_struct->is_secure = is_secure;
	
	if( pthread_mutex_init( &new_HPD_web_server_struct->configure_mutex, NULL ) )
	{
		printf("Error while initializing configure mutex\n");
		free( new_HPD_web_server_struct );
		return NULL;
	}
	
	return new_HPD_web_server_struct;
}
#endif

/**
 * Destroy a HPD_web_server_struct and deallocate its memory, should not be used anywhere but 
 * in this file (static).
 *
 * @param to_destroy The HPD_web_server_struct to destroy
 *
 * @return HPD_YES if successful or HPD_E_NULL_POINTER if the pointer given is NULL
 */
#if HPD_HTTPS
static int 
destroy_HPD_web_server_struct( HPD_web_server_struct* to_destroy )
{
	if( !to_destroy )
		return HPD_E_NULL_POINTER;

	pthread_mutex_destroy( &to_destroy->configure_mutex );
	
	free( to_destroy );
	
	return HPD_YES;
}
#endif

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

	rc = initiate_global_event_queue();
	if( rc < 0 )
	{	
		printf("Failed to initiate Server Sent Event Queue\n");
		return rc;
	}

   struct lr_settings settings = LR_SETTINGS_DEFAULT;
   settings.port = hpd_daemon->http_port;

#if HPD_HTTP
	
	unsecure_web_server = lr_create(&settings, loop);
   if (!unsecure_web_server)
      return HPD_E_MHD_ERROR;

   if (lr_start(unsecure_web_server))
      return HPD_E_MHD_ERROR;

   rc = HPD_E_SUCCESS;
#endif

#if HPD_HTTPS
	secure_web_server = create_HPD_web_server_struct( HPD_IS_SECURE_CONNECTION );

	rc = start_secure_web_server( secure_web_server );
	if( rc < 0 )
	{	
		printf("Failed to start HTTPS server\n");
		return rc;
	}
#endif

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

	free_server_sent_events_ressources();

#if HPD_HTTP
   lr_stop(unsecure_web_server);
   lr_destroy(unsecure_web_server);
	rc = HPD_E_SUCCESS;
#endif

#if HPD_HTTPS
	rc = stop_web_server( secure_web_server );
	if( rc < HPD_E_SUCCESS )
		return rc;
	destroy_HPD_web_server_struct( secure_web_server );
#endif

	delete_xml(XML_FILE_NAME);

#if USE_AVAHI
	avahi_quit ();
#endif

	return rc;

}

static int answer_get(void *data, struct lr_request *req,
                      const char *body, size_t len)
{
   //Service *service = data;
   // TODO Write this

   return 0;
}

static int answer_put(void *data, struct lr_request *req,
                      const char *body, size_t len)
{
   //Service *service = data;
   // TODO Write this

   return 0;
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
#if HPD_HTTP
      Service *s = lr_lookup_service(unsecure_web_server, service_to_register->value_url);
		if (s) {
			printf("A similar service is already registered in the unsecure server\n");
			return HPD_E_SERVICE_ALREADY_REGISTER;
		}

		printf("Registering non secure service\n");
		rc = lr_register_service(unsecure_web_server,
                               service_to_register->value_url,
                               answer_get, NULL, answer_put, NULL,
                               service_to_register);
		if(rc)
			return HPD_E_MHD_ERROR;
#else
		printf("Trying to register non secure service without HTTP support\n");
		return HPD_E_NO_HTTP;
#endif
	}

	else if( service_to_register->device->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
		rc = is_service_registered_in_web_server( service_to_register, 
							  secure_web_server );

		if( rc == 1 )
		{
			printf("A similar service is already registered in the unsecure server\n");
			return HPD_E_SERVICE_ALREADY_REGISTER;
		}

		printf("Registering secure service\n");
		rc = register_service_in_web_server ( service_to_register, secure_web_server );
		if( rc < HPD_E_SUCCESS )
			return rc;
#else
		printf("Trying to register secure service without HTTPS support\n");
		return HPD_E_NO_HTTPS;
#endif
	}

	else 
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
#if HPD_HTTP
      Service *s = lr_lookup_service(unsecure_web_server, service_to_unregister->value_url);
	   if( !s )
		   return HPD_E_SERVICE_NOT_REGISTER;

		s = lr_unregister_service ( unsecure_web_server,
            service_to_unregister->value_url );
		if( !s )
			return HPD_E_MHD_ERROR;
#else
		printf("Unregistering non secure service without HTTP feature (Service is not register anyway)\n");
		return HPD_E_NO_HTTP;
#endif
	}

	else if( service_to_unregister->device->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
	   if( !is_service_registered( service_to_unregister ) )
		   return HPD_E_SERVICE_NOT_REGISTER;

		rc = unregister_service_in_web_server ( service_to_unregister, secure_web_server );
		if( rc < HPD_E_SUCCESS )
			return rc;
#else
		printf("Unregistering secure service without HTTPS feature (Service is not register anyway)\n");
		return HPD_E_NO_HTTPS;
#endif
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

#if HPD_HTTP
	if( service->device->secure_device == HPD_NON_SECURE_DEVICE )
	{
      Service *s = lr_lookup_service(unsecure_web_server, service->value_url);
      if (s) return 1;
      else return 0;
	}	
#endif

#if HPD_HTTPS
	if( service->device->secure_device == HPD_SECURE_DEVICE )
	{
		return is_service_registered_in_web_server( service, secure_web_server );
	}	
#endif

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

#if HPD_HTTP
   // TODO This is not the best solution (duplicated code), nor the best
   // place to do this
	char *value_url = malloc(sizeof(char)*( strlen("/") + strlen(service->device->type) + strlen("/") 
	                                           + strlen(service->device->ID) + strlen("/") + strlen(service->type)
	                                           + strlen("/") + strlen(service->ID) + 1 ) );
	sprintf( value_url,"/%s/%s/%s/%s", service->device->type, service->device->ID, service->type,
	         service->ID );
	service = lr_lookup_service(unsecure_web_server, value_url);
   free(value_url);
#endif
#if HPD_HTTPS
	if( service == NULL )
	{
		service = get_service_from_web_server ( device_type, device_ID, 
							service_type, service_ID,
							secure_web_server );
	}
#endif
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

#if HPD_HTTP
   // TODO This is not the best solution (duplicated code), nor the best
   // place to do this
	char *value_url = malloc(sizeof(char)*( strlen("/") + strlen(service->device->type) + strlen("/") 
	                                           + strlen(service->device->ID) + strlen("/") + strlen(service->type)
	                                           + strlen("/") + strlen(service->ID) + 1 ) );
	sprintf( value_url,"/%s/%s/%s/%s", service->device->type, service->device->ID, service->type,
	         service->ID );
	service = lr_lookup_service(unsecure_web_server, value_url);
   free(value_url);
	device = service->device;
#endif
#if HPD_HTTPS
	if( device == NULL )
	{
		device = get_device_from_web_server( device_type, device_ID, secure_web_server );
	}
#endif
	return device;
}
