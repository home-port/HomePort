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

#include "hpd_rest.h"
#include "homeport.h"
#include "libREST.h"
#include "hpd_error.h"
#include "lr_interface.h"
#include "utlist.h"

static void on_dev_attach(void *data, Device *device) {
    Service *service;
    DL_FOREACH(device->service_head, service)
    {
        lri_registerService(data, service);
    }
}

static void on_dev_detach(void *data, Device *device) {
    Service *service;
    DL_FOREACH(device->service_head, service)
    {
        lri_unregisterService(data, service->uri);
    }
}

int hpd_rest_init(struct hpd_lr *data, HomePort *hp, int port)
{
    // Create settings
    struct lr_settings settings = LR_SETTINGS_DEFAULT;
    settings.port = port;

    // Create and start libREST
    data->lr = lr_create(&settings, hp->loop);
    if ( data->lr == NULL )
    {
        // TODO Fix error code
        return 1;
    }
    if(lr_start(data->lr))
        return HPD_E_MHD_ERROR;

    // Listen on new devices
    data->dev_listener = homePortNewDeviceListener(hp, on_dev_attach, on_dev_detach, data->lr, NULL);
    homePortSubscribe(data->dev_listener);
    homePortForAllAttachedDevices(data->dev_listener);

    // Register devices uri
    return lr_register_service(data->lr,
                               "/devices",
                               lri_getConfiguration, NULL, NULL, NULL,
                               NULL, hp);

    return 0;
}

int hpd_rest_deinit(struct hpd_lr *data, HomePort *hp)
{
    lr_stop(data->lr);

    lr_destroy(data->lr);

    return 0;
}