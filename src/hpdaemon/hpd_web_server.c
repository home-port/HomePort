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
 * @file hpd_web_server.c
 * @brief  Methods for managing the Web Server
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include "hpd_web_server.h"
#include "hpd_error.h"

#define AVAHI 1



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
start_server(char* hostname, char *domain_name)
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

#if HPD_HTTP
	rc = start_unsecure_server();
	if( rc < 0 )
	{	
		printf("Failed to start HTTP server\n");
		return rc;
	}
#endif

#if HPD_HTTPS
	rc = start_secure_server();
	if( rc < 0 )
	{	
		printf("Failed to start HTTPS server\n");
		return rc;
	}
#endif

#if AVAHI == 1
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
	rc = stop_unsecure_server();
	if( rc < HPD_E_SUCCESS )
		return rc;
#endif

#if HPD_HTTPS
	rc = stop_secure_server();
	if( rc < HPD_E_SUCCESS )
		return rc;
#endif

	delete_xml(XML_FILE_NAME);

#if AVAHI == 1
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
register_service_in_server( Service *service_to_register )
{

	int rc;

	if( service_to_register->device->secure_device == HPD_NON_SECURE_DEVICE )
	{
#if HPD_HTTP
		rc = is_unsecure_service_registered( service_to_register );	

		if( rc == 1 )
		{
			printf("A similar service is already registered in the unsecure server\n");
			return HPD_E_SERVICE_ALREADY_REGISTER;
		}

		printf("Registering non secure service\n");
		rc = register_service_in_unsecure_server ( service_to_register );
		if( rc < HPD_E_SUCCESS )
			return rc;
#else
		printf("Trying to register non secure service without HTTP support\n");
		return HPD_E_NO_HTTP;
#endif
	}

	else if( service_to_register->device->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
		rc = is_secure_service_registered( service_to_register );

		if( rc == 1 )
		{
			printf("A similar service is already registered in the unsecure server\n");
			return HPD_E_SERVICE_ALREADY_REGISTER;
		}

		printf("Registering secure service\n");
		rc = register_service_in_secure_server ( service_to_register );
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

	rc = avahi_create_service ( service_to_register );
	if(  rc < HPD_E_SUCCESS )
	{
		printf("avahi_create_service failed : %d\n", rc);
		return rc;
	}

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
unregister_service_in_server( Service *service_to_unregister )
{

	int rc;

	if( !is_service_registered( service_to_unregister ) )
		return HPD_E_SERVICE_NOT_REGISTER;

	if( service_to_unregister->device->secure_device == HPD_NON_SECURE_DEVICE )
	{
#if HPD_HTTP
		rc = unregister_service_in_unsecure_server ( service_to_unregister );
		if( rc < HPD_E_SUCCESS )
			return rc;
#else
		printf("Unregistering non secure service without HTTP feature (Service is not register anyway)\n");
		return HPD_E_NO_HTTP;
#endif
	}

	else if( service_to_unregister->device->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
		rc = unregister_service_in_secure_server ( service_to_unregister );
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

	rc = avahi_remove_service ( service_to_unregister );
	if(  rc < HPD_E_SUCCESS )
	{
		printf("avahi_remove_service failed : %d\n", rc);
		return rc;
	}

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

	if( device_to_register->secure_device == HPD_NON_SECURE_DEVICE )
	{
#if HPD_HTTP
		DL_FOREACH(device_to_register->service_head, iterator)
		{
			return_value = register_service_in_server (iterator->service);
			if(return_value < HPD_E_SUCCESS)
			{
				return return_value;
			}
		}

		return HPD_E_SUCCESS;
#else
		printf("Trying to register non secure device without HTTP feature\n");
		return HPD_E_NO_HTTP;
#endif
	}

	if( device_to_register->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
		DL_FOREACH(device_to_register->service_head, iterator)
		{
			return_value = register_service_in_secure_server (iterator);
			if(return_value < HPD_E_SUCCESS)
			{
				return return_value;
			}
		}
		return HPD_E_SUCCESS;
#else
		printf("Trying to register secure device without HTTPS feature\n");
		return HPD_E_NO_HTTPS;
#endif
	}

	return HPD_E_BAD_PARAMETER;
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
	if( device_to_unregister->secure_device == HPD_NON_SECURE_DEVICE )
	{
#if HPD_HTTP
		DL_FOREACH( device_to_unregister->service_head, iterator )
		{
			return_value = unregister_service_in_server ( iterator->service );
			if(return_value < HPD_E_SUCCESS)
			{
				return return_value;
			}
		}
#else
		printf("Trying to unregister non secure device without HTTP feature\n");
		return HPD_E_NO_HTTP;
#endif
	}

	if( device_to_unregister->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
		DL_FOREACH(device_to_unregister->service_head, iterator)
		{
			return_value = unregister_service_in_secure_server (iterator);
			if(return_value < HPD_E_SUCCESS)
			{
				return return_value;
			}
		}
#else
		printf("Trying to unregister secure device without HTTPS feature\n");
		return HPD_E_NO_HTTPS;
#endif
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
		return is_unsecure_service_registered(service);
	}	
#endif

#if HPD_HTTPS
	if( service->device->secure_device == HPD_SECURE_DEVICE )
	{
		return is_secure_service_registered(service);
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
get_service_from_server( char *device_type, char *device_ID, char *service_type, char *service_ID )
{
	Service *service = NULL;

#if HPD_HTTP
	service = get_service_from_unsecure_server ( device_type, device_ID, service_type, service_ID );
#endif
#if HPD_HTTPS
	if( service == NULL )
	{
		service = get_service_from_secure_server ( device_type, device_ID, service_type, service_ID );
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
get_device_from_server( char *device_type, char *device_ID)
{
	Device *device = NULL;

#if HPD_HTTP
	device = get_device_from_unsecure_server ( device_type, device_ID );
#endif
#if HPD_HTTPS
	if( device == NULL )
	{
		device = get_device_from_secure_server( device_type, device_ID );
	}
#endif
	return device;
}
