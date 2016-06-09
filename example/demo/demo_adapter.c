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

#include "demo_adapter.h"
#include "hpd/hpd_adapter_api.h"
#include "hpd/common/hpd_common.h"

struct hpd_demo_adapter {
    int num_lamps;
    const char *module_id;
    hpd_adapter_id_t *adapter_id;
};

static hpd_error_t demo_adapter_on_create(void **data, const hpd_module_t *context);
static hpd_error_t demo_adapter_on_destroy(void *data);
static hpd_error_t demo_adapter_on_start(void *data, hpd_t *hpd);
static hpd_error_t demo_adapter_on_stop(void *data, hpd_t *hpd);
static hpd_error_t demo_adapter_on_parse_opt(void *data, const char *name, const char *arg);

struct hpd_module_def hpd_demo_adapter_def = {
        demo_adapter_on_create,
        demo_adapter_on_destroy,
        demo_adapter_on_start,
        demo_adapter_on_stop,
        demo_adapter_on_parse_opt,
};

static hpd_status_t demo_adapter_on_get(void *data, hpd_request_t *req)
{
    hpd_error_t rc;
    int *state = data;

    // TODO Clean up on errors !

    const hpd_service_id_t *sid;
    if ((rc = hpd_request_get_service(req, &sid))) return HPD_S_500;

    const char *id;
    if ((rc = hpd_service_get_id(sid, &id))) return HPD_S_500;

    hpd_value_t *val;
    if ((rc = hpd_value_alloc(&val, id, HPD_NULL_TERMINATED))) return HPD_S_500;

    hpd_response_t *res;
    if ((rc = hpd_response_alloc(&res, req, HPD_S_200))) return HPD_S_500;
    if ((rc = hpd_response_set_value(res, val))) return HPD_S_500;
    if ((rc = hpd_respond(res))) return HPD_S_500;

    return HPD_S_NONE;
}

// TODO Has changed signature !
// TODO Just a quick hack for testing - lots of fixes needed
static hpd_status_t demo_adapter_on_put(void *data, hpd_request_t *req)
{
    const hpd_value_t *val_in;
    hpd_value_t *val_out;
    hpd_request_get_value(req, &val_in);
    hpd_value_copy(&val_out, val_in);

    hpd_error_t rc;
    hpd_response_t *res;
    if ((rc = hpd_response_alloc(&res, req, HPD_S_200))) return rc;
    if ((rc = hpd_response_set_value(res, val_out))) return rc;
    if ((rc = hpd_respond(res))) return rc;

    return HPD_E_SUCCESS;
}

static hpd_error_t demo_adapter_on_create(void **data, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create struct with custom data
    struct hpd_demo_adapter *demo_adapter = calloc(1, sizeof(struct hpd_demo_adapter));
    demo_adapter->num_lamps = 1;
    if ((rc = hpd_module_get_id(context, &demo_adapter->module_id))) return rc;

    // Add supported options
    if ((rc = hpd_module_add_option(context, "num-lamps", "count", 0, "Number of lamps to create. Default 1."))) return rc;

    // Return
    *data = demo_adapter;
    return HPD_E_SUCCESS;
}

static hpd_error_t demo_adapter_on_destroy(void *data)
{
    struct hpd_demo_adapter *demo_adapter = data;
    free(demo_adapter);
    return HPD_E_SUCCESS;
}

static hpd_error_t demo_adapter_on_start(void *data, hpd_t *hpd)
{
    struct hpd_demo_adapter *demo_adapter = data;
    hpd_error_t rc;

    printf("[adapter] Starting with %i lamps...\n", demo_adapter->num_lamps); // TODO Wrong!

    // TODO Clean up on errors !

    // Create adapter structure
    // Using module_id as id, because we are only creating one adapter per module in this scenario,
    // and we don't have a more informal id from the underlying device.
    hpd_adapter_t *adapter;
    if ((rc = hpd_adapter_alloc(&adapter, demo_adapter->module_id))) return rc;
    if ((rc = hpd_adapter_set_attr(adapter, HPD_ATTR_TYPE, "demo_adapter"))) return rc;
    // Hand it over to hpd
    // Hereafter the pointer should not be used, only references by id
    if ((rc = hpd_adapter_attach(hpd, adapter))) return rc;

    // Create id structure to reference our adapter
    hpd_adapter_id_alloc(&demo_adapter->adapter_id, hpd, demo_adapter->module_id);

    // Create device structures
    for (size_t i = 0; i < demo_adapter->num_lamps; i++) {
        char *id = NULL;
        HPD_SPRINTF_ALLOC(id, "dev%zu", i);

        hpd_device_t *device;
        if ((rc = hpd_device_alloc(&device, id))) return rc;
        if ((rc = hpd_device_set_attr(device, HPD_ATTR_TYPE, "demo_lamp"))) return rc;
        hpd_service_t *service;
        if ((rc = hpd_service_alloc(&service, "srv0"))) return rc;
        if ((rc = hpd_service_set_actions(service,
                                          HPD_M_GET, demo_adapter_on_get,
                                          HPD_M_PUT, demo_adapter_on_put,
                                          HPD_M_NONE))) return rc;
        int *state;
        HPD_CALLOC(state, 1, int);
        if ((rc = hpd_service_set_data(service, state, free))) return rc;
        hpd_parameter_t *parameter;
        if ((rc = hpd_parameter_alloc(&parameter, "param0"))) return rc;
        if ((rc = hpd_parameter_attach(service, parameter))) return rc;
        if ((rc = hpd_service_attach(device, service))) return rc;
        if ((rc = hpd_device_attach(demo_adapter->adapter_id, device))) return rc;

        free(id);
    }

    return HPD_E_SUCCESS;

    alloc_error:
        return HPD_E_ALLOC;
    nsprintf_error:
        return HPD_E_UNKNOWN;
}

static hpd_error_t demo_adapter_on_stop(void *data, hpd_t *hpd)
{
    struct hpd_demo_adapter *demo_adapter = data;

    printf("[adapter] Stopping...\n"); // TODO Wrong!

    // Detach our adapter from hpd
    // It is now in our control again, hpd does no longer know about it
    hpd_adapter_t *adapter;
    hpd_adapter_detach(demo_adapter->adapter_id, &adapter);

    // Clean up nicely
    hpd_adapter_free(adapter);
    hpd_adapter_id_free(demo_adapter->adapter_id);

    return HPD_E_SUCCESS;
}

static hpd_error_t demo_adapter_on_parse_opt(void *data, const char *name, const char *arg)
{
    struct hpd_demo_adapter *demo_adapter = data;

    // Handle the options we defined in demo_adapter_on_create
    if (strcmp(name, "num-lamps") == 0)
        demo_adapter->num_lamps = atoi(arg);
    else
        return HPD_E_ARGUMENT; // We should return this when we do not recognise the option

    return HPD_E_SUCCESS;
}
