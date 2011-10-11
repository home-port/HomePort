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

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** Once the library is installed, should be remplaced by <hpdaemon/homeport.h> */
#include "homeport.h"
#include "services.h"

/** A GET function for a service
*	Takes a Service structure in parameter, and return a service value as a char*
*/
size_t get_lamp (Service* _service, char *buffer, size_t max_buffer_size)
{
	printf("Received GET lamp on %s %s\n", _service->description, _service->ID);
	sprintf(buffer,"0");
	return strlen(buffer);
}
/** A PUT function for a service
*	Takes a Service structure in parameter, and return an updated service value as a char*
*/
size_t put_lamp (Service* _service, char *buffer, size_t max_buffer_size, char *put_value)
{
	printf("Received PUT lamp on %s %s\n", _service->description, _service->ID);
	sprintf(buffer, "0");
	return strlen(buffer);
}

size_t get_switch (Service* _service, char *buffer, size_t max_buffer_size)
{
	printf("Received GET switch on %s %s\n", _service->description, _service->ID);
	sprintf(buffer,"0");
	return strlen(buffer);
}


int main()
{	

	int rc;

	/** Starts the hpdaemon. If using avahi-core pass a host name for the server, otherwise pass NULL */
	if( rc = HPD_start(HPD_USE_CFG_FILE, "Homeport", HPD_OPTION_CFG_PATH, "./hpd.cfg"))
	{
		printf("Failed to start HPD %d\n", rc);
		return 1;
	}

#if HPD_HTTP /** Creation and registration of non secure services */
	/** Create a device that will contain the services 
	* 1st  parameter : A description of the device (optional) 
	* 2nd  parameter : The ID of the device
	* 3rd  parameter : The device's vendor ID (optional)
	* 4th  parameter : The device's product ID (optional)
	* 5th  parameter : The device's version (optional)
	* 6th  parameter : The device's IP address (optional)
	* 7th  parameter : The device's port (optional)
	* 8th  parameter : The device's location (optional)
	* 9th  parameter : The device's type
	* 10th parameter : A flag indicating if the communication with the device should be secure
	* 				   HPD_SECURE_DEVICE or HPD_NON_SECURE_DEVICE
	*/ 
	Device *_device = create_device_struct("Example", 
						"1", 
						"0x01", 
						"0x01", 
						"V1", 
						NULL, 
						NULL, 
						"LivingRoom", 
						"Example",																				 
						HPD_NON_SECURE_DEVICE);
	/** Create a service
	* 1st parameter : The service's description (optional)
	* 2nd parameter : The service's ID
	* 4th parameter : The service's type
	* 5th parameter : The service's unit (optional)
	* 6th parameter : The device owning the service
	* 7th parameter : The service's GET function
	* 8th parameter : The service's PUT function (optional)
	* 9th parameter : The service's parameter structure
	*/
	Service *_service_lamp0 = create_service_struct ("Lamp0", "0", "Lamp", "ON/OFF", _device, 
	                                                 get_lamp, put_lamp, 
	                                                 create_parameter_struct ("0", NULL, NULL,
	                                                                          NULL, NULL, NULL,
	                                                                          NULL, NULL)
							,NULL);
	Service *_service_lamp1 = create_service_struct ("Lamp1", "1", "Lamp", "ON/OFF", _device,
	                                                 get_lamp, put_lamp, 
	                                                 create_parameter_struct ("0", NULL, NULL,
	                                                                          NULL, NULL, NULL,
	                                                                          NULL, NULL),NULL);
	Service *_service_switch0 = create_service_struct ("Switch0", "0", "Switch", "ON/OFF", _device,
	                                                   get_switch, NULL, 
	                                                   create_parameter_struct ("0", NULL, NULL,
	                                                                            NULL, NULL, NULL,
	                                                                            NULL, NULL),NULL);

	Service *_service_switch1 = create_service_struct ("Switch1", "1", "Switch", "ON/OFF", _device,
	                                                   get_switch, NULL, 
	                                                   create_parameter_struct ("0", NULL, NULL,
	                                                                            NULL, NULL, NULL,
	                                                                            NULL, NULL),NULL);

	printf("Registering service lamp0. Can be accessed at URL : Example/1/Lamp/0\n");
	/** Register a service into the HPD web server */	
	HPD_register_service (_service_lamp0);
	printf("Registering service lamp1. Can be accessed at URL : Example/1/Lamp/1\n");	
	HPD_register_service (_service_lamp1);
	printf("Registering service switch0. Can be accessed at URL : Example/1/Switch/0\n");
	HPD_register_service (_service_switch0); 
	printf("Registering service switch1. Can be accessed at URL : Example/1/Switch/1\n");
	HPD_register_service (_service_switch1);

#endif

#if HPD_HTTPS /** Creation and registration of secure services */
	/** Create a secured device that will contain the secured services */
	Device *_secure_device = create_device_struct("SecureExample", 
						"1", 
						"0x01", 
						"0x01", 
						"V1", 
						NULL, 
						NULL, 
						"LivingRoom", 
						"SecureExample",																				 
						HPD_SECURE_DEVICE);
	/** Create a service
	* 1st parameter : The service's description (optional)
	* 2nd parameter : The service's ID
	* 4th parameter : The service's type
	* 5th parameter : The service's unit (optional)
	* 6th parameter : The device owning the service
	* 7th parameter : The service's GET function
	* 8th parameter : The service's PUT function (optional)
	* 9th parameter : The service's parameter structure
	*/
	Service *_secure_service_lamp0 = create_service_struct ("Lamp0", "0", "Lamp", "ON/OFF", _secure_device, 
	                                                 get_lamp, put_lamp, 
	                                                 create_parameter_struct ("0", NULL, NULL,
	                                                                          NULL, NULL, NULL,
	                                                                          NULL, NULL), NULL);
	Service *_secure_service_lamp1 = create_service_struct ("Lamp1", "1", "Lamp", "ON/OFF", _secure_device,
	                                                 get_lamp, put_lamp, 
	                                                 create_parameter_struct ("0", NULL, NULL,
	                                                                          NULL, NULL, NULL,
	                                                                          NULL, NULL), NULL);
	Service *_secure_service_switch0 = create_service_struct ("Switch0", "0", "Switch", "ON/OFF", _secure_device,
	                                                   get_switch, NULL, 
	                                                   create_parameter_struct ("0", NULL, NULL,
	                                                                            NULL, NULL, NULL,
	                                                                            NULL, NULL), NULL);

	Service *_secure_service_switch1 = create_service_struct ("Switch1", "1", "Switch", "ON/OFF", _secure_device,
	                                                   get_switch, NULL, 
	                                                   create_parameter_struct ("0", NULL, NULL,
	                                                                            NULL, NULL, NULL,
	                                                                            NULL, NULL), NULL);
	/** Register all the device's services into the HPD web server */	
	HPD_register_device_services(_secure_device);


#endif

	getchar();

#if HPD_HTTP
	/** Unregister a service from the HPD web server */
	HPD_unregister_service (_service_lamp0);
	HPD_unregister_service (_service_lamp1);
	HPD_unregister_service (_service_switch0);
	HPD_unregister_service (_service_switch1);

#endif

#if HPD_HTTPS
	/* Unregister all the services of a device */
	HPD_unregister_device_services(_secure_device);
#endif

	/** Stops the HPD daemon */
	HPD_stop ();

	return (0);
}
