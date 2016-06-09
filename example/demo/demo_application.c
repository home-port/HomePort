/*
 * Copyright 2011 Aalborg University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVidED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 */

#include "demo_application.h"
#include <hpd/hpd_application_api.h>
#include <hpd/common/hpd_common.h>

typedef struct demo_app demo_app_t;

struct demo_app {
    const hpd_module_t *context;
    hpd_listener_t *listener;
};

static hpd_error_t demo_app_on_create(void **data, const hpd_module_t *context);
static hpd_error_t demo_app_on_destroy(void *data);
static hpd_error_t demo_app_on_start(void *data, hpd_t *hpd);
static hpd_error_t demo_app_on_stop(void *data, hpd_t *hpd);
static hpd_error_t demo_app_on_parse_opt(void *data, const char *name, const char *arg);

struct hpd_module_def hpd_demo_app_def = {
        demo_app_on_create,
        demo_app_on_destroy,
        demo_app_on_start,
        demo_app_on_stop,
        demo_app_on_parse_opt,
};

// TODO Error handling (entire file)

void demo_app_on_attach(void *data, const hpd_device_id_t *device)
{
    demo_app_t *demo_app = data;

    hpd_adapter_id_t *adapter;
    hpd_device_get_adapter(device, &adapter);

    const char *aid;
    hpd_adapter_get_id(adapter, &aid);

    const char *did;
    hpd_device_get_id(device, &did);
    
    HPD_LOG_INFO(demo_app->context, "%s/%s attached", aid, did);
    
    hpd_adapter_id_free(adapter);
}

void demo_app_on_detach(void *data, const hpd_device_id_t *device)
{
    demo_app_t *demo_app = data;

    hpd_adapter_id_t *adapter;
    hpd_device_get_adapter(device, &adapter);

    const char *aid;
    hpd_adapter_get_id(adapter, &aid);

    const char *did;
    hpd_device_get_id(device, &did);

    HPD_LOG_INFO(demo_app->context, "%s/%s detached", aid, did);

    hpd_adapter_id_free(adapter);
}

void demo_app_on_change(void *data, const hpd_service_id_t *service, const hpd_value_t *val)
{
    demo_app_t *demo_app = data;

    hpd_adapter_id_t *adapter;
    hpd_service_get_adapter(service, &adapter);

    hpd_device_id_t *device;
    hpd_service_get_device(service, &device);

    const char *aid;
    hpd_adapter_get_id(adapter, &aid);

    const char *did;
    hpd_device_get_id(device, &did);

    const char *sid;
    hpd_service_get_id(service, &sid);

    const char *v;
    size_t len;
    hpd_value_get_body(val, &v, &len);

    HPD_LOG_INFO(demo_app->context, "%s/%s/%s changed to %.*s", aid, did, sid, len, v);

    hpd_adapter_id_free(adapter);
    hpd_device_id_free(device);
}

static hpd_error_t demo_app_on_create(void **data, const hpd_module_t *context)
{
    demo_app_t *demo_app;
    HPD_CALLOC(demo_app, 1, demo_app_t);
    demo_app->context = context;
    (*data) = demo_app;
    return HPD_E_SUCCESS;

    alloc_error:
        HPD_LOG_RETURN_E_ALLOC(context);
}

static hpd_error_t demo_app_on_destroy(void *data)
{
    demo_app_t *demo_app = data;
    free(demo_app);
    return HPD_E_SUCCESS;
}

static hpd_error_t demo_app_on_start(void *data, hpd_t *hpd)
{
    demo_app_t *demo_app = data;

    hpd_listener_alloc(&demo_app->listener, hpd);
    hpd_listener_set_data(demo_app->listener, demo_app, NULL);
    hpd_listener_set_device_callback(demo_app->listener, demo_app_on_attach, demo_app_on_detach);
    hpd_listener_set_value_callback(demo_app->listener, demo_app_on_change);
    hpd_subscribe(demo_app->listener);
    hpd_foreach_attached(demo_app->listener);

    return HPD_E_SUCCESS;
}

static hpd_error_t demo_app_on_stop(void *data, hpd_t *hpd)
{
    demo_app_t *demo_app = data;

    hpd_listener_free(demo_app->listener);

    return HPD_E_SUCCESS;
}

static hpd_error_t demo_app_on_parse_opt(void *data, const char *name, const char *arg)
{
    return HPD_E_ARGUMENT;
}
