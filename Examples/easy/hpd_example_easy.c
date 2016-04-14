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
#include <getopt.h>

// With installed homeport change these to <hpdaemon/*>
#include "hpd_rest.h"

struct data {
    struct hpd_rest *rest;
    Adapter *adapter;
    Device *device;
    Service *service_lamp0;
    Service *service_lamp1;
    Service *service_switch0;
    Service *service_switch1;
};

enum option_code {
    OPT_DONE = -1,
    OPT_FLAGGED = 0,
    OPT_HELP = 'h',
    OPT_REST = 0xff + 1,
};

static struct option options[] = {
        { "rest", optional_argument, 0, OPT_REST },
        { "help", no_argument, 0, OPT_HELP },
        { 0 }
};

static const char *short_opt = "d:h";

static const char *help_msg = ""
        "      --rest [PORT]   Start rest server on PORT (defaults to 80). if argument is\n"
        "                      not given, Rest server is not started.\n"
        "  -h, --help          This help message.\n";

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

static void deinit(HomePort *homeport, void *in)
{
    struct data *data = in;

    /** Unregistering services is not necessary, calling HPD_stop unregisters and
      deallocates the services and devices that are still register in HPD		   */
    /** However when unregistering a service note that the memory is not deallocated   */
    /** Also note that attempting to free a service that is still registered will fail */

    homePortDetachDevice(homeport, data->device);

    /** Deallocate the memory of the services. When deallocating the last service of a device,
      the device is deallocated too, so there is no need to call destroy_device_struct       */
    homePortFreeService(data->service_lamp0);
    homePortFreeService(data->service_lamp1);
    homePortFreeService(data->service_switch0);
    homePortFreeService(data->service_switch1);

    homePortFreeDevice(data->device);

    homePortFreeAdapter(data->adapter);

    if (data->rest != NULL)
        hpd_rest_deinit(data->rest, homeport);
}

static int init(HomePort *homeport, void *in)
{
    struct data *data = in;

    if (data->rest != NULL)
        hpd_rest_init(data->rest, homeport);

    if (homePortNewAdapter(&data->adapter, homeport, "0", "test", NULL, NULL))
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
    if(homePortNewDevice(&data->device, data->adapter, "0", "Example",
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
     if (homePortNewService (&data->service_lamp0, data->device, "1", "Lamp0", "Lamp", "ON/OFF",
                                        get_lamp, put_lamp,
                                        homePortNewParameter (NULL, NULL,
                                                              NULL, NULL, NULL,
                                                              NULL, NULL)
            ,NULL, NULL))
         return 1;

     if (homePortNewService (&data->service_lamp1, data->device, "2", "Lamp1", "Lamp", "ON/OFF",
                                        get_lamp, put_lamp,
                                        homePortNewParameter (NULL, NULL,
                                                              NULL, NULL, NULL,
                                                              NULL, NULL),
                                        NULL, NULL))
         return 1;

     if (homePortNewService (&data->service_switch0, data->device, "3", "Switch0", "Switch", "ON/OFF",
                                          get_switch, NULL,
                                          homePortNewParameter (NULL, NULL,
                                                                NULL, NULL, NULL,
                                                                NULL, NULL),
                                          NULL, NULL))
         return 1;

     if (homePortNewService (&data->service_switch1, data->device, "4", "Switch1", "Switch", "ON/OFF",
                                          get_switch, NULL,
                                          homePortNewParameter (NULL, NULL,
                                                                NULL, NULL, NULL,
                                                                NULL, NULL),
                                          NULL, NULL))
         return 1;

    homePortAttachDevice(homeport, data->device);

    return 0;
}

int main(int argc, char **argv)
{
    enum option_code c;
    int i;
    int stat;
    struct data data;

    data.rest = NULL;

    do {
        c = (enum option_code) getopt_long(argc, argv, short_opt, options, &i);
        switch (c) {
            case OPT_DONE:
            case OPT_FLAGGED:
                break;
            case OPT_REST:
                hpd_rest_destroy(data.rest);
                data.rest = hpd_rest_create();
                if (optarg)
                    hpd_rest_set_port(data.rest, atoi(optarg));
                break;
            case OPT_HELP:
                printf("%s", help_msg);
                return 1;
        }
    } while (c != OPT_DONE);

    stat = homePortEasy(init, deinit, &data);

    hpd_rest_destroy(data.rest);

    return stat;
}

