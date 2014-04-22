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
#include <string.h>
#include <ev.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** Once the library is installed, should be remplaced by <hpdaemon/homeport.h> */
#include "homeport.h"

#if HPD_HTTP 
static Service *service_lamp0 = NULL;
static Service *service_lamp1 = NULL;
static Service *service_switch0 = NULL;
static Service *service_switch1 = NULL;
#endif
#if HPD_HTTPS
static Device *secure_device = NULL;Object {device: Object}
device: Object
#endif

char switch0_state[] = "0";
char switch1_state[] = "0";
ev_timer timer;

void
switch_event(struct ev_loop *loop, ev_timer *timer2, int revents)
{
  printf("value change\n");
  if(strcmp(switch0_state, "0") == 0)
  {
    strcpy(switch0_state, "1");
    strcpy(switch1_state, "1");
  }
  else
  {
    strcpy(switch0_state, "0");
    strcpy(switch1_state, "0");
  }
  HPD_send_event_of_value_change( service_switch0, switch0_state );
  HPD_send_event_of_value_change( service_switch1, switch1_state );

  ev_timer_again(loop, &timer);
}

/** A GET function for a service
 *	Takes a Service structure in parameter, and return a service value as a char*
 */
  size_t 
get_lamp ( Service* service, char *buffer, size_t max_buffer_size )
{
  printf("Received GET lamp on %s %s\n", service->description, service->ID);
  sprintf(buffer,"0");
  return strlen(buffer);
}
/** A PUT function for a service
 *	Takes a Service structure in parameter, and return an updated service value as a char*
 */
  size_t 
put_lamp ( Service* service, char *buffer, size_t max_buffer_size, char *put_value )
{
  printf("Received PUT lamp on %s %s\n", service->description, service->ID);
  sprintf(buffer, "0");
  return strlen(buffer);
}

  size_t 
get_switch ( Service* service, char *buffer, size_t max_buffer_size )
{
  printf("Received GET switch on %s %s\n", service->description, service->ID);
  sprintf(buffer,"0");
  return strlen(buffer);
}

  static void
exit_handler ( int sig )
{
  /** Unregistering services is not necessary, calling HPD_stop unregisters and 
    deallocates the services and devices that are still register in HPD		   */
  /** However when unregistering a service note that the memory is not deallocated   */
  /** Also note that attempting to free a service that is still registered will fail */

#if HPD_HTTP	
  /** Unregister a service from the HPD web server */
  if (service_lamp0)
    HPD_unregister_service (service_lamp0);
  if (service_lamp1)
    HPD_unregister_service (service_lamp1);
  if (service_switch0)
    HPD_unregister_service (service_switch0);
  if (service_switch1)
    HPD_unregister_service (service_switch1);
  /** Deallocate the memory of the services. When deallocating the last service of a device, 
    the device is deallocated too, so there is no need to call destroy_device_struct       */
  destroy_service_struct(service_lamp0);
  destroy_service_struct(service_lamp1);
  destroy_service_struct(service_switch0);
  destroy_service_struct(service_switch1);
#endif

#if HPD_HTTPS
  /** Unregister all the services of a device */
  HPD_unregister_device_services(secure_device);
  /** Deallocate the memory of the device, and all the services linked to this device */
  destroy_device_struct(secure_device);
#endif

  /** Stops the HPD daemon */
  HPD_stop ();

  exit(sig);
}

  int 
main()
{	
  int rc;

  /** Create the event loop */
  struct ev_loop *loop = EV_DEFAULT;

  // Register signals for correctly exiting
  signal(SIGINT, exit_handler);
  signal(SIGTERM, exit_handler);

  /** Starts the hpdaemon. If using avahi-core pass a host name for the server, otherwise pass NULL */
  if( (rc = HPD_start( HPD_USE_CFG_FILE, loop, "Homeport",
	  HPD_OPTION_CFG_PATH, "./hpd.cfg" )) )
  {
    printf("Failed to start HPD %d\n", rc);
    return 1;
  }

#if HPD_HTTP 
  Device *device = create_device_struct("Example", 
      "1", 
      "0x01", 
      "0x01", 
      "V1", 
      NULL, 
      NULL, 
      "LivingRoom", 
      "Example",																				 
      HPD_NON_SECURE_DEVICE);
  Device *device2 = create_device_struct("Example2", 
      "2", 
      "0x01", 
      "0x01", 
      "V1", 
      NULL, 
      NULL, 
      "LivingRoom", 
      "Example2",																				 
      HPD_NON_SECURE_DEVICE);
  service_lamp0 = create_service_struct ("Lamp0", "0", 1, "Lamp", "ON/OFF", device, 
      get_lamp, put_lamp, 
      create_parameter_struct ("0", "1", "0",
	"1", "1", "1",
	"ON/OFF", NULL)
      ,NULL);
  service_lamp1 = create_service_struct ("Lamp1", "1", 1, "Lamp", "ON/OFF", device,
      get_lamp, put_lamp, 
      create_parameter_struct ("0", "1", "0",
	"1", "1", "1",
	"ON/OFF", NULL)
      ,NULL);
  service_switch0 = create_service_struct ("Switch0", "0", 0, "Switch", "ON/OFF", device2,
      get_switch, NULL, 
      create_parameter_struct ("0", "1", "0",
	"1", "1", "1",
	"ON/OFF", NULL)
      ,NULL);

  service_switch1 = create_service_struct ("Switch1", "1", 0, "Switch", "ON/OFF", device2,
      get_switch, NULL, 
      create_parameter_struct ("0", "1", "0",
	"1", "1", "1",
	"ON/OFF", NULL)
      ,NULL);

  printf("Registering service lamp0. Can be accessed at URL : Example/1/Lamp/0\n");

  HPD_register_service (service_lamp0);
  printf("Registering service lamp1. Can be accessed at URL : Example/1/Lamp/1\n");	
  HPD_register_service (service_lamp1);
  printf("Registering service switch0. Can be accessed at URL : Example/1/Switch/0\n");
  HPD_register_service (service_switch0); 
  printf("Registering service switch1. Can be accessed at URL : Example/1/Switch/1\n");
  HPD_register_service (service_switch1);

  ev_init(&timer, switch_event);  
  timer.repeat = 10.;
  ev_timer_again(loop, &timer);


#endif
  ev_run(loop, 0);

  exit_handler(0);
  return (0);
}

