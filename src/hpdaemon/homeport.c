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


#include "homeport.h"
#include "hpd_error.h"
#include "web_server.h"


/**
 * Parse the user's specified options
 *
 * @param ap va_list of options
 *
 * @return A HPD error code
 */
static 
int parse_option_va(va_list ap)
{
	enum HPD_OPTION opt;
	int log_level, max_log_size;
	char *log_file_name;

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

/**
 * Starts the HomePort Daemon
 *
 * @param _hostname Name of the desired host
 *
 * @return A HPD error code
 */
int HPD_start( unsigned int option, char *_hostname, ... )
{

	int rc;

	if( rc = HPD_init_daemon() )
	{
		printf("Error initializing HPD_Daemon struct\n");
		return rc;
	}
#if !AVAHI_CLIENT
	if( _hostname == NULL )
		return HPD_E_NULL_POINTER;

	hpd_daemon->hostname = _hostname;
#endif

	va_list ap;

	va_start( ap, _hostname);

	if( option & HPD_USE_CFG_FILE )
	{
		printf("Using config file\n");

		if( va_arg(ap, enum HPD_OPTION) != HPD_OPTION_CFG_PATH )
		{
			printf("Only HPD_OPTION_CFG_PATH msut be specified when using HPD_USE_CFG_FILE\n");

			return HPD_E_BAD_PARAMETER;
		}		
		
		if( rc = HPD_config_file_init( va_arg( ap, const char* ) ) )
		{
			printf("Error loading config file %d\n", rc);
			return rc;
		}		
	}

	else
	{	
		if( rc = parse_option_va(ap) )
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

	va_end(ap);	

	return start_server(_hostname, NULL);
}



/**
 * Stops the HomePort Daemon
 *
 * @return A HPD error code
 */
int HPD_stop()
{
	HPD_config_deinit();
	return stop_server();
}

/**
 * Registers a given Service in the HomePort Daemon
 *
 * @param _service The service to register
 *
 * @return A HPD error code
 */
int HPD_register_service(Service *_service)
{
	if( _service == NULL )
		return HPD_E_NULL_POINTER;

	return register_service_in_server(_service);
}

/**
 * Unregisters a given Service in the HomePort Daemon
 *
 * @param _service The service to unregister
 *
 * @return A HPD error code
 */
int HPD_unregister_service(Service *_service)
{
	if( _service == NULL )
		return HPD_E_NULL_POINTER;

	return unregister_service_in_server(_service);
}

/**
 * Registers all the Services contained in a given
 *  Device in the HomePort Daemon
 *
 * @param _device The device that contains the services
 *  to register
 *
 * @return A HPD error code
 */
int HPD_register_device_services(Device *_device)
{
	if( _device == NULL )
	{
		return HPD_E_NULL_POINTER;
	}
	return register_device_services( _device );
	
}


/**
 * Unregisters all the Services contained in a given
 *  Device in the HomePort Daemon
 *
 * @param _device The device that contains the services
 *  to unregister
 *
 * @return A HPD error code
 */
int HPD_unregister_device_services(Device *_device)
{
    if( _device == NULL )
	return HPD_E_NULL_POINTER;
	
    return unregister_device_services(_device);
}


/**
 * Gets a service given its uniqueness identifiers
 *
 * @param _device_type The type of the device that contains the service
 *
 * @param _device_ID The ID of the device that contains the service
 *
 * @param _service_type The type of the wanted service
 *
 * @param _service_ID The ID of the wanted service
 *
 * @return The desired Service or NULL if failed
 */
Service* HPD_get_service( char *_device_type, char *_device_ID, char *_service_type, char *_service_ID )
{

	if( _device_type == NULL || _device_ID == NULL || _service_type == NULL || _service_ID == NULL )
		return NULL;

	return get_service_from_server( _device_type, _device_ID, _service_type, _service_ID );
}

/**
 * Gets a device given its uniqueness identifiers
 *
 * @param _device_type The type of the device 
 *
 * @param _device_ID The ID of the device 
 *
 * @return The desired Device or NULL if failed
 */
Device* HPD_get_device(char *_device_type, char *_device_ID)
{
	if( _device_type == NULL || _device_ID == NULL )
		return NULL;

	return get_device_from_server( _device_type, _device_ID );
}

/**
 * Sends an event of a changing of value from a given Service
 *
 * @param _service_url The URL of the service
 *
 * @param _updated_value The new value.
 *
 * @return HPD_E_SUCCESS if successful
 */
int HPD_send_event_of_value_change (Service *service, char *_updated_value)
{
	return send_event_of_value_change (service, _updated_value);
}


