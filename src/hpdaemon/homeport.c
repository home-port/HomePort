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
#include "web_server_api.h"
#include "utlist.h"

/**
 * Starts the HomePort Daemon
 *
 * @param _hostname Name of the desired host
 *
 * @return 0 if the daemon has been successfully started, 
 *        -1 if failed
 */
int HPD_start( char* _hostname )
{
	return start_server (_hostname, NULL);
}



/**
 * Stops the HomePort Daemon
 *
 * @return 0 if the daemon has been successfully stopped, 
 *        -1 if failed
 */
int HPD_stop()
{
	stop_server();
	return 0;
}

/**
 * Registers a given Service in the HomePort Daemon
 *
 * @param _service The service to register
 *
 * @return 0 if the service has been successfully registered,
 *		  -1 if a similar service already exists in the server,
 *		  -2 if the adding to the XML file failed,
 *        	  -3 if the service was not added to the list successfully, 
 *		  -4 if the server couldn't add it to its list
 */
int HPD_register_service(Service *_service)
{
	return register_service_in_server (_service);
}

/**
 * Unregisters a given Service in the HomePort Daemon
 *
 * @param _service The service to unregister
 *
 * @return 0 if the service has been successfully unregistered, 
 *        	-1 if the removing from the XML file failed,
 *       	-2 if the service couldn't be removed from the server's service list,
 *		-3 if the service couldn't be removed from the AVAHI server
 */
int HPD_unregister_service(Service *_service)
{
	return unregister_service_in_server (_service);
}

/**
 * Registers all the Services contained in a given
 *  Device in the HomePort Daemon
 *
 * @param _device The device that contains the services
 *  to register
 *
 * @return 0 if the services have been successfully registered,
 *		  -1 if a similar service already exists in the server,
 *		  -2 if the adding to the XML file failed,
 *        	  -3 if one of the services was not added to the list successfully, 
 *		  -4 if the server couldn't add one of the services to its list
 */
int HPD_register_device_services(Device *_device){

    ServiceElement *_iterator;
    int return_value;
    LL_FOREACH(_device->service_head, _iterator)
    {
        return_value = register_service_in_server (_iterator->service);
        if(return_value < 0){
	   return return_value;
        }
    }
    return 0;
}


/**
 * Unregisters all the Services contained in a given
 *  Device in the HomePort Daemon
 *
 * @param _device The device that contains the services
 *  to unregister
 *
 * @return 0 if the services have been successfully unregistered, 
 *        	-1 if the removing from the XML file failed,
 *       	-2 if one of the services couldn't be removed from the server's service list,
 *		-3 if one of the services couldn't be removed from the AVAHI server
 */
int HPD_unregister_device_services(Device *_device){

    ServiceElement *_iterator;
    int return_value;
    LL_FOREACH(_device->service_head, _iterator)
    {
        return_value = unregister_service_in_server (_iterator->service);
        if(return_value < 0){
	   return return_value;
        }
    }
    return 0;
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
	return get_service_from_server ( _device_type, _device_ID, _service_type, _service_ID );
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
	return get_device_from_server ( _device_type, _device_ID);
}

