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
 * THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
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

/// [includes]
#include "demo_adapter.h"
#include <hpd/hpd_adapter_api.h>
#include <hpd/common/hpd_common.h>
/// [includes]

/// [types]
typedef struct demo_adapter demo_adapter_t;
typedef struct demo_adapter_srv demo_adapter_srv_t;

struct demo_adapter {
    int num_lamps;
    const char *module_id;
    hpd_adapter_t *adapter;
    const hpd_module_t *context;
};

struct demo_adapter_srv {
    int state;
    demo_adapter_t *demo_adapter;
};
/// [types]

/// [definition]
static hpd_error_t demo_adapter_on_create(void **data, const hpd_module_t *context);
static hpd_error_t demo_adapter_on_destroy(void *data);
static hpd_error_t demo_adapter_on_start(void *data);
static hpd_error_t demo_adapter_on_stop(void *data);
static hpd_error_t demo_adapter_on_parse_opt(void *data, const char *name, const char *arg);

struct hpd_module_def hpd_demo_adapter_def = {
        demo_adapter_on_create,
        demo_adapter_on_destroy,
        demo_adapter_on_start,
        demo_adapter_on_stop,
        demo_adapter_on_parse_opt,
};
/// [definition]

/// [send]
static hpd_status_t demo_adapter_send_value(hpd_request_t *req, demo_adapter_srv_t *srv_data)
{
    hpd_error_t rc, rc2;

    // Allocate response
    hpd_response_t *res;
    if ((rc = hpd_response_alloc(&res, req, HPD_S_200))) goto error_return;

    // Create and set value
    hpd_value_t *val;
    if ((rc = hpd_value_allocf(srv_data->demo_adapter->context, &val, "%i", srv_data->state))) goto error_free_res;
    if ((rc = hpd_response_set_value(res, val))) goto error_free_val;

    // Send response
    if ((rc = hpd_respond(res))) goto error_free_res;

    return HPD_S_NONE;

    error_free_val:
    if ((rc2 = hpd_value_free(val)))
        HPD_LOG_ERROR(srv_data->demo_adapter->context, "Free function failed [code: %i].", rc2);

    error_free_res:
    if ((rc2 = hpd_response_free(res)))
        HPD_LOG_ERROR(srv_data->demo_adapter->context, "Free function failed [code: %i].", rc2);

    error_return:
    HPD_LOG_ERROR(srv_data->demo_adapter->context, "%s() failed [code: %i].", __FUNCTION__, rc);
    return HPD_S_500;
}
/// [send]

/// [changed]
static hpd_error_t demo_adapter_send_changed(const hpd_service_id_t *service_id, demo_adapter_srv_t *srv_data)
{
    hpd_error_t rc;

    hpd_value_t *value;
    if ((rc = hpd_value_allocf(srv_data->demo_adapter->context, &value, "%i", srv_data->state))) return rc;

    if ((rc = hpd_id_changed(service_id, value))) hpd_value_free(value);
    return rc;
}
/// [changed]

/// [set]
static hpd_status_t demo_adapter_set_value(hpd_request_t *req, demo_adapter_srv_t *srv_data)
{
    hpd_error_t rc;

    // Get value
    const hpd_value_t *val;
    if ((rc = hpd_request_get_value(req, &val))) goto error_return;

    // Get body from value
    const char *body;
    size_t len;
    if ((rc = hpd_value_get_body(val, &body, &len))) goto error_return;

    // Get service id
    const hpd_service_id_t *service_id;
    if ((rc = hpd_request_get_service(req, &service_id))) goto error_return;

    // Get new state
    char *nul_term = NULL;
    HPD_STR_N_CPY(nul_term, body, len);
    int state = atoi(nul_term);
    free(nul_term);

    if (state != srv_data->state) {
        srv_data->state = state;
        if ((rc = demo_adapter_send_changed(service_id, srv_data)))
            HPD_LOG_ERROR(srv_data->demo_adapter->context, "Failed to send changed value [code: %i].", rc);
    }

    return HPD_S_NONE;

    alloc_error:
    HPD_LOG_ERROR(srv_data->demo_adapter->context, "%s() failed.", __FUNCTION__);
    return HPD_S_500;

    error_return:
    HPD_LOG_ERROR(srv_data->demo_adapter->context, "%s() failed [code: %i].", __FUNCTION__, rc);
    return HPD_S_500;
}
/// [set]

/// [on_get]
static hpd_status_t demo_adapter_on_get(void *data, hpd_request_t *req)
{
    return demo_adapter_send_value(req, data);
}
/// [on_get]

/// [on_put]
static hpd_status_t demo_adapter_on_put(void *data, hpd_request_t *req)
{
    hpd_status_t status;
    if ((status = demo_adapter_set_value(req, data)) != HPD_S_NONE) return status;
    return demo_adapter_send_value(req, data);
}
/// [on_put]

/// [on_create]
static hpd_error_t demo_adapter_on_create(void **data, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create struct with custom data
    demo_adapter_t *demo_adapter;
    HPD_CALLOC(demo_adapter, 1, demo_adapter_t);
    demo_adapter->num_lamps = 1;
    demo_adapter->context = context;
    if ((rc = hpd_module_get_id(context, &demo_adapter->module_id)))
        goto error_free;

    // Add supported options
    if ((rc = hpd_module_add_option(context, "num-lamps", "count", 0, "Number of lamps to create. Default 1.")))
        goto error_free;

    *data = demo_adapter;
    return HPD_E_SUCCESS;

    alloc_error:
    HPD_LOG_RETURN_E_ALLOC(context);

    error_free:
    free(demo_adapter);
    return rc;
}
/// [on_create]

/// [on_destroy]
static hpd_error_t demo_adapter_on_destroy(void *data)
{
    demo_adapter_t *demo_adapter = data;
    free(demo_adapter);
    return HPD_E_SUCCESS;
}
/// [on_destroy]

/// [create_adapter]
static hpd_error_t demo_adapter_create_adapter(demo_adapter_t *demo_adapter)
{
    hpd_error_t rc, rc2;

    // Create adapter structure (using module_id as id)
    hpd_adapter_t *adapter;
    if ((rc = hpd_adapter_alloc(&adapter, demo_adapter->context, demo_adapter->module_id))) goto error_return;
    if ((rc = hpd_adapter_set_attr(adapter, HPD_ATTR_TYPE, "demo_adapter"))) goto error_free_adapter;
    if ((rc = hpd_adapter_attach(adapter))) goto error_free_adapter;

    return HPD_E_SUCCESS;

    error_free_adapter:
    if ((rc2 = hpd_adapter_free(adapter)))
        HPD_LOG_ERROR(demo_adapter->context, "Free function failed [code: %i].", rc2);

    error_return:
    return rc;
}
/// [create_adapter]

/// [create_parameter]
static hpd_error_t demo_adapter_create_parameter(demo_adapter_t *demo_adapter, hpd_service_t *service)
{
    hpd_error_t rc, rc2;

    hpd_parameter_t *parameter;
    if ((rc = hpd_parameter_alloc(&parameter, demo_adapter->context, "param0"))) goto error_return;
    if ((rc = hpd_parameter_attach(service, parameter))) goto error_free;

    return HPD_E_SUCCESS;

    error_free:
    if ((rc2 = hpd_parameter_free(parameter)))
        HPD_LOG_ERROR(demo_adapter->context, "Free function failed [code: %i].", rc2);

    error_return:
    return rc;
}
/// [create_parameter]

/// [create_service]
static hpd_error_t demo_adapter_create_service(demo_adapter_t *demo_adapter, hpd_device_t *device)
{
    hpd_error_t rc, rc2;

    hpd_service_t *service;
    if ((rc = hpd_service_alloc(&service, demo_adapter->context, "srv0"))) goto error_return;
    if ((rc = hpd_service_set_actions(service,
                                      HPD_M_GET, demo_adapter_on_get,
                                      HPD_M_PUT, demo_adapter_on_put,
                                      HPD_M_NONE))) goto error_free_service;

    demo_adapter_srv_t *srv_data;
    HPD_CALLOC(srv_data, 1, demo_adapter_srv_t);
    srv_data->state = 0;
    srv_data->demo_adapter = demo_adapter;
    if ((rc = hpd_service_set_data(service, srv_data, free))) goto error_free_data;

    if ((rc = demo_adapter_create_parameter(demo_adapter, service))) goto error_free_service;
    if ((rc = hpd_service_attach(device, service))) goto error_free_service;

    return HPD_E_SUCCESS;

    alloc_error:
    if ((rc2 = hpd_service_free(service)))
        HPD_LOG_ERROR(demo_adapter->context, "Free function failed [code: %i].", rc2);
    HPD_LOG_RETURN_E_ALLOC(demo_adapter->context);

    error_free_data:
    free(srv_data);

    error_free_service:
    if ((rc2 = hpd_service_free(service)))
        HPD_LOG_ERROR(demo_adapter->context, "Free function failed [code: %i].", rc2);

    error_return:
    return rc;
}
/// [create_service]

/// [create_lamp]
static hpd_error_t demo_adapter_create_lamp(demo_adapter_t *demo_adapter, const char *id)
{
    hpd_error_t rc, rc2;

    hpd_device_t *device;
    if ((rc = hpd_device_alloc(&device, demo_adapter->context, id))) goto error_return;
    if ((rc = hpd_device_set_attr(device, HPD_ATTR_TYPE, "demo_lamp"))) goto error_free;
    if ((rc = demo_adapter_create_service(demo_adapter, device))) goto error_free;
    if ((rc = hpd_device_attach(demo_adapter->adapter, device))) goto error_free;

    return HPD_E_SUCCESS;

    error_free:
    if ((rc2 = hpd_device_free(device)))
        HPD_LOG_ERROR(demo_adapter->context, "Free function failed [code: %i].", rc2);

    error_return:
    return rc;
}
/// [create_lamp]

/// [on_start]
static hpd_error_t demo_adapter_on_start(void *data)
{
    demo_adapter_t *demo_adapter = data;
    hpd_error_t rc, rc2;

    HPD_LOG_INFO(demo_adapter->context, "Starting with %i lamps...", demo_adapter->num_lamps);

    if ((rc = demo_adapter_create_adapter(demo_adapter))) goto error_return;

    // Create device structures
    char *id = NULL;
    for (int i = 0; i < demo_adapter->num_lamps; i++) {
        HPD_SPRINTF_ALLOC(id, "dev%i", i);
        if ((rc = demo_adapter_create_lamp(demo_adapter, id))) goto error_free_id;
        free(id);
    }

    return HPD_E_SUCCESS;

    alloc_error:
    free(id);
    if ((rc2 = demo_adapter_on_stop(demo_adapter)))
        HPD_LOG_ERROR(demo_adapter->context, "on_stop() failed [code: %i].", rc2);
    HPD_LOG_RETURN_E_ALLOC(demo_adapter->context);

    snprintf_error:
    free(id);
    if ((rc2 = demo_adapter_on_stop(demo_adapter)))
        HPD_LOG_ERROR(demo_adapter->context, "on_stop() failed [code: %i].", rc2);
    HPD_LOG_RETURN_E_SNPRINTF(demo_adapter->context);

    error_free_id:
    free(id);
    if ((rc2 = demo_adapter_on_stop(demo_adapter)))
        HPD_LOG_ERROR(demo_adapter->context, "on_stop() failed [code: %i].", rc2);

    error_return:
    return rc;
}
/// [on_start]

/// [on_stop]
static hpd_error_t demo_adapter_on_stop(void *data)
{
    hpd_error_t rc;

    demo_adapter_t *demo_adapter = data;

    HPD_LOG_INFO(demo_adapter->context, "Stopping...");

    // Detach adapter from hpd
    if ((rc = hpd_adapter_detach(demo_adapter->adapter))) goto error_return;

    // Clean up nicely
    if ((rc = hpd_adapter_free(demo_adapter->adapter))) goto error_return;

    return HPD_E_SUCCESS;

    error_return:
    return rc;
}
/// [on_stop]

/// [on_parse_opt]
static hpd_error_t demo_adapter_on_parse_opt(void *data, const char *name, const char *arg)
{
    demo_adapter_t *demo_adapter = data;

    if (strcmp(name, "num-lamps") == 0) {
        demo_adapter->num_lamps = atoi(arg);
        return HPD_E_SUCCESS;
    }

    return HPD_E_ARGUMENT;
}
/// [on_parse_opt]
