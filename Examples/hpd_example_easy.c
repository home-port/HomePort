/*Copyright 2011 Aalborg University. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of
  conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list
  of conditions and the following disclaimer in the documentation and/or other materials
  provided with the distribution.

  THIS SOFTWARE IS PROVidED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCidENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed*/

#include <stdio.h>
#include <stdlib.h>

// With installed homeport change these to <hpdaemon/*>
#include "homeport.h"
#include "hpd_rest.h"

static Adapter *adapter = NULL;
static Device *device = NULL;
static Service *service_lamp0 = NULL;
static Service *service_lamp1 = NULL;
static Service *service_switch0 = NULL;
static Service *service_switch1 = NULL;

/** A GET function for a service
 *	Takes a Service structure in parameter, and return a service value as a char*
 */
void
get_lamp ( Service* service, Request request )
{
    printf("Received GET lamp on %s %s\n", service->description, service->id);
    homePortRespond(service, request, ERR_200, "0", 1);
}
/** A PUT function for a service
 *	Takes a Service structure in parameter, and return an updated service value as a char*
 */
void
put_lamp ( Service* service, Request req, const char *put_value, size_t len )
{
    printf("Received PUT lamp on %s %s\n", service->description, service->id);
    homePortRespond(service, req, ERR_200, "0", 1);
}

void
get_switch ( Service* service, Request request )
{
    printf("Received GET switch on %s %s\n", service->description, service->id);
    homePortRespond(service, request, ERR_200, "0", 1);
}

static void deinit(HomePort *homeport, void *data)
{
    /** Unregistering services is not necessary, calling HPD_stop unregisters and
      deallocates the services and devices that are still register in HPD		   */
    /** However when unregistering a service note that the memory is not deallocated   */
    /** Also note that attempting to free a service that is still registered will fail */

    homePortDetachDevice(homeport, device);

    /** Deallocate the memory of the services. When deallocating the last service of a device,
      the device is deallocated too, so there is no need to call destroy_device_struct       */
    homePortFreeService(service_lamp0);
    homePortFreeService(service_lamp1);
    homePortFreeService(service_switch0);
    homePortFreeService(service_switch1);

    homePortFreeDevice(device);

    homePortFreeAdapter(adapter);

    hpd_rest_deinit(data, homeport);
}

static int init(HomePort *homeport, void *data)
{
    hpd_rest_init(data, homeport, 8890);

    if (homePortNewAdapter(&adapter, homeport, "0", "test", NULL, NULL))
        return 1;

    /** Creation and registration of non secure services */
    /** Create a device that will contain the services
     * 1st  parameter : A description of the device (optional)
     * 2nd  parameter : The id of the device
     * 3rd  parameter : The device's vendor id (optional)
     * 4th  parameter : The device's product id (optional)
     * 5th  parameter : The device's version (optional)
     * 6th  parameter : The device's IP address (optional)
     * 7th  parameter : The device's port (optional)
     * 8th  parameter : The device's location (optional)
     * 9th  parameter : The device's type
     * 10th parameter : A flag indicating if the communication with the device should be secure
     * 				   HPD_SECURE_DEVICE or HPD_NON_SECURE_DEVICE
     */
    if(homePortNewDevice(&device, adapter, "0", "Example",
                               "0x01",
                               "0x01",
                               "V1",
                               "LivingRoom",
                               "Example", NULL, NULL))
        return 1;
    /** Create a service
     * 1st parameter : The service's description (optional)
     * 2nd parameter : The service's id
     * 4th parameter : The service's type
     * 5th parameter : The service's unit (optional)
     * 6th parameter : The device owning the service
     * 7th parameter : The service's GET function
     * 8th parameter : The service's PUT function (optional)
     * 9th parameter : The service's parameter structure
     */
     if (homePortNewService (&service_lamp0, device, "1", "Lamp0", "Lamp", "ON/OFF",
                                        get_lamp, put_lamp,
                                        homePortNewParameter (NULL, NULL,
                                                              NULL, NULL, NULL,
                                                              NULL, NULL)
            ,NULL, NULL))
         return 1;

     if (homePortNewService (&service_lamp1, device, "2", "Lamp1", "Lamp", "ON/OFF",
                                        get_lamp, put_lamp,
                                        homePortNewParameter (NULL, NULL,
                                                              NULL, NULL, NULL,
                                                              NULL, NULL),
                                        NULL, NULL))
         return 1;

     if (homePortNewService (&service_switch0, device, "3", "Switch0", "Switch", "ON/OFF",
                                          get_switch, NULL,
                                          homePortNewParameter (NULL, NULL,
                                                                NULL, NULL, NULL,
                                                                NULL, NULL),
                                          NULL, NULL))
         return 1;

     if (homePortNewService (&service_switch1, device, "4", "Switch1", "Switch", "ON/OFF",
                                          get_switch, NULL,
                                          homePortNewParameter (NULL, NULL,
                                                                NULL, NULL, NULL,
                                                                NULL, NULL),
                                          NULL, NULL))
         return 1;

    homePortAttachDevice(homeport, device);

    return 0;
}

int
main()
{
    struct hpd_rest data;

    /** Starts the hpdaemon. If using avahi-core pass a host name for the server, otherwise pass NULL */
    return homePortEasy(init, deinit, &data);
}

