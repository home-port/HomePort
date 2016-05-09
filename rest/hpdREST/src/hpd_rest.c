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

#include <stdlib.h>
#include "hpd_rest.h"
#include "libREST.h"
#include "lr_interface.h"
#include "hpd_application_api.h"
#include "hpd_common.h"

static hpd_error_t on_create(void **data, hpd_module_t *context);
static hpd_error_t on_destroy(void *data);
static hpd_error_t on_start(void *data, hpd_t *hpd);
static hpd_error_t on_stop(void *data, hpd_t *hpd);
static hpd_error_t on_parse_opt(void *data, const char *name, const char *arg);

hpd_module_def_t hpd_rest = { on_create, on_destroy, on_start, on_stop, on_parse_opt };

typedef struct hpd_rest {
    struct lr_settings settings;
    struct lr *lr;
    hpd_listener_t *listener;
} hpd_rest_t;

static hpd_error_t on_create(void **data, hpd_module_t *context)
{
    hpd_error_t rc;
    struct lr_settings settings = LR_SETTINGS_DEFAULT;

    if ((rc = hpd_module_add_option(context, "port", "port", 0, "Listener port for rest server."))) return rc;

    hpd_rest_t *rest;
    HPD_CALLOC(rest, 1, hpd_rest_t);
    rest->settings = settings;
    (*data) = rest;
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

static hpd_error_t on_destroy(void *data)
{
    free(data);
    return HPD_E_SUCCESS;
}

static hpd_error_t on_dev_attach(void *data, const hpd_device_id_t *device)
{
    hpd_error_t rc;
    hpd_service_id_t *service;

    hpd_device_foreach_service(rc, service, device)
        if (lri_registerService(data, service)) return HPD_E_UNKNOWN;

    return rc;
}

static hpd_error_t on_dev_detach(void *data, const hpd_device_id_t *device)
{
    hpd_error_t rc;
    hpd_service_id_t *service;

    hpd_device_foreach_service(rc, service, device) {
        char *uri = lri_alloc_uri(service);
        if (!uri) return HPD_E_UNKNOWN;
        // TODO Ignoring error for now...
        lri_unregisterService(data, uri);
        free(uri);
    }

    return rc;
}

static hpd_error_t on_start(void *data, hpd_t *hpd)
{
    hpd_error_t rc;
    hpd_rest_t *rest = data;

    hpd_ev_loop_t *loop;
    if ((rc = hpd_get_loop(hpd, &loop))) return rc;

    // Create and start libREST
    // TODO Not the best error handling (probably goes for the entire file)...
    if (!(rest->lr = lr_create(&rest->settings, loop))) return HPD_E_UNKNOWN;
    if (lr_start(rest->lr)) return HPD_E_UNKNOWN;

    // Listen on new devices
    if ((rc = hpd_listener_alloc(&rest->listener, hpd))) return rc;
    if ((rc = hpd_listener_set_device_callback(rest->listener, on_dev_attach, on_dev_detach))) return rc;
    if ((rc = hpd_listener_set_data(rest->listener, rest->lr, NULL))) return rc;
    if ((rc = hpd_subscribe(rest->listener))) return rc;
    if ((rc = hpd_foreach_attached(rest->listener))) return rc;

    // Register devices uri
    if (lr_register_service(rest->lr,
                            "/devices",
                            lri_getConfiguration, NULL, NULL, NULL,
                            NULL, hpd))
        return HPD_E_UNKNOWN;

    return HPD_E_SUCCESS;
}

static hpd_error_t on_stop(void *data, hpd_t *hpd)
{
    hpd_error_t rc;
    hpd_rest_t *rest = data;
    hpd_device_id_t *device;

    hpd_foreach_device(rc, device, hpd) {
        // TODO Ignoring error for now...
        on_dev_detach(data, device);
    }

    if (rc) {
        hpd_listener_free(rest->listener);
    } else {
        rc = hpd_listener_free(rest->listener);
    }
    lr_stop(rest->lr);
    lr_destroy(rest->lr);
    return rc;
}

static hpd_error_t on_parse_opt(void *data, const char *name, const char *arg)
{
    hpd_rest_t *rest = data;

    if (strcmp(name, "port")) {
        // TODO Lazy error handling:
        rest->settings.port = atoi(arg);
    } else {
        return HPD_E_ARGUMENT;
    }

    return HPD_E_SUCCESS;
}
