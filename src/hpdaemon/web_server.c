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

#include "web_server.h"
#include "hpd_error.h"

#define AVAHI 1



/**
 * Start the MHD web server(s) and the AVAHI client or server
 *
 * @param _hostname Hostname for the local address of the server
 *
 * @param _domain_name Domain name for the local address of the server (if NULL = .local)
 *
 * @return A HPD error code
 */

int start_server(char* _hostname, char *_domain_name)
{
	int rc;

	init_xml_file (XML_FILE_NAME,DEVICE_LIST_ID);

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
    	rc = avahi_start (_hostname, _domain_name);
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
int stop_server()
{
	int rc;

	free_condition_variable();

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

	free_server_sent_events_ressources();

	return rc;

}


/**
 * Add a service to the XML file, the server(s), and the AVAHI client or server
 *
 * @param _service The service to register
 *
 * @return A HPD error code
 */
int register_service_in_server(Service *_service)
{

	int rc;

	if( _service->device->secure_device == HPD_NON_SECURE_DEVICE )
	{
#if HPD_HTTP
		rc = is_unsecure_service_registered( _service );	

		if( rc == 1 )
    		{
        		printf("A similar service is already registered in the unsecure server\n");
        		return HPD_E_SERVICE_ALREADY_REGISTER;
    		}

		printf("Registering non secure service\n");
		rc = register_service_in_unsecure_server (_service);
		if( rc < HPD_E_SUCCESS )
			return rc;
#else
		printf("Trying to register non secure service without HTTP support\n");
		return HPD_E_NO_HTTP;
#endif
	}
	
	else if( _service->device->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
		rc = is_secure_service_registered( _service );

		if( rc == 1 )
    		{
        		printf("A similar service is already registered in the unsecure server\n");
        		return HPD_E_SERVICE_ALREADY_REGISTER;
    		}

		printf("Registering secure service\n");
		rc = register_service_in_secure_server (_service);
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
    	rc = add_service_to_xml (_service);
    	if (rc == -1){
        	printf("Impossible to add the Service to the XML file.\n");
        	return HPD_E_XML_ERROR;
    	}
    	else if(rc == -2){
        	printf("The Service already exists\n");
    	}

	rc = avahi_create_service ( _service );
    	if(  rc < HPD_E_SUCCESS )
    	{
        	printf("avahi_create_service failed : %d\n", rc);
        	return rc;
    	}

	return HPD_E_SUCCESS;
}

/**
 * Remove a service from the XML file, the server(s), and the AVAHI client or server
 *
 * @param _service The service to remove
 *
 * @return A HPD error code
 */
int unregister_service_in_server( Service *_service )
{

	int rc;

	if( !is_service_registered( _service ) )
		return HPD_E_SERVICE_NOT_REGISTER;

	if( _service->device->secure_device == HPD_NON_SECURE_DEVICE )
	{
#if HPD_HTTP
		rc = unregister_service_in_unsecure_server (_service);
		if( rc < HPD_E_SUCCESS )
			return rc;
#else
		printf("Unregistering non secure service without HTTP feature (Service is not register anyway)\n");
		return HPD_E_NO_HTTP;
#endif
	}

	else if( _service->device->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
		rc = unregister_service_in_secure_server (_service);
		if( rc < HPD_E_SUCCESS )
			return rc;
#else
		printf("Unregistering secure service without HTTPS feature (Service is not register anyway)\n");
		return HPD_E_NO_HTTPS;
#endif
	}
	else 
		return HPD_E_BAD_PARAMETER;


	rc = remove_service_from_XML(_service);
    	if( rc < HPD_E_SUCCESS )
    	{
        	printf("remove_service_from_xml failed : %d\n", rc);
        	return HPD_E_XML_ERROR;
    	}

	rc = avahi_remove_service ( _service );
    	if(  rc < HPD_E_SUCCESS )
    	{
        	printf("avahi_remove_service failed : %d\n", rc);
        	return rc;
    	}

	rc = unsubscribe_and_notify_service_leaving( _service );
	if(  rc < HPD_E_SUCCESS )
    	{
        	printf("unsubscribe_and_notify_service_leaving failed : %d\n", rc);
        	return rc;
    	}

	return HPD_E_SUCCESS;
}

/**
 * Register all of a device's services
 *
 * @param _service The device to register
 *
 * @return A HPD error code
 */
int register_device_services( Device *_device )
{
    ServiceElement *_iterator;
    int return_value;

	if( _device->secure_device == HPD_NON_SECURE_DEVICE )
	{
#if HPD_HTTP
		LL_FOREACH(_device->service_head, _iterator)
		{
			return_value = register_service_in_server (_iterator->service);
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

	if( _device->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
		LL_FOREACH(_device->service_head, _iterator)
		{
			return_value = register_service_in_secure_server (_iterator->service);
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
 * @param _service The device to unregister
 *
 * @return A HPD error code
 */
int unregister_device_services( Device *_device )
{
	ServiceElement *_iterator;
    	int return_value;
	if( _device->secure_device == HPD_NON_SECURE_DEVICE )
	{
#if HPD_HTTP
		LL_FOREACH(_device->service_head, _iterator)
		{
			return_value = unregister_service_in_server (_iterator->service);
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

	if( _device->secure_device == HPD_SECURE_DEVICE )
	{
#if HPD_HTTPS
		LL_FOREACH(_device->service_head, _iterator)
		{
			return_value = unregister_service_in_secure_server (_iterator->service);
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
 * @param _service The service to check
 *
 * @return A HPD error code
 */
int is_service_registered( Service *_service )
{

	if( _service == NULL )
		return HPD_E_NULL_POINTER;

#if HPD_HTTP
	if( _service->device->secure_device == HPD_NON_SECURE_DEVICE )
	{
		return is_unsecure_service_registered(_service);
	}	
#endif

#if HPD_HTTPS
	if( _service->device->secure_device == HPD_SECURE_DEVICE )
	{
		return is_secure_service_registered(_service);
	}	
#endif

	return 0;
}

/**
 * Retrieve a Service object from a server
 *
 * @param _device_type The type of the device that owns the service
 *
 * @param _device_ID The ID of the device that owns the service
 *
 * @param _service_type The type of the service
 *
 * @param _service_ID The ID of the service 
 *
 * @return A pointer on the Service struct if found, NULL otherwise
 */
Service* get_service_from_server( char *_device_type, char *_device_ID, char *_service_type, char *_service_ID )
{
	Service *_service = NULL;

#if HPD_HTTP
	_service = get_service_from_unsecure_server ( _device_type, _device_ID, _service_type, _service_ID );
#endif
#if HPD_HTTPS
	if( _service == NULL )
	{
		_service = get_service_from_secure_server ( _device_type, _device_ID, _service_type, _service_ID );
	}
#endif
	return _service;
}


/**
 * Retrieve a Device object from a server
 *
 * @param _device_type The type of the device
 *
 * @param _device_ID The ID of the device
 *
 * @return A pointer on the Device struct if found, NULL otherwise
 */
Device* get_device_from_server( char *_device_type, char *_device_ID)
{
	Device *_device = NULL;
	
#if HPD_HTTP
	_device = get_device_from_unsecure_server ( _device_type, _device_ID );
#endif
#if HPD_HTTPS
	if( _device == NULL )
	{
		_device = get_device_from_secure_server( _device_type, _device_ID );
	}
#endif
	return _device;
}
