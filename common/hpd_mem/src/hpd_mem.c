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

// TODO Error check, entire file

#include <hpd/modules/hpd_mem.h>
#include <hpd/hpd_adapter_api.h>
#include <bsd/sys/queue.h>
#include <hpd/common/hpd_common.h>

static hpd_error_t mem_on_create(void **data, const hpd_module_t *context);
static hpd_error_t mem_on_destroy(void *data);
static hpd_error_t mem_on_start(void *data);
static hpd_error_t mem_on_stop(void *data);
static hpd_error_t mem_on_parse_opt(void *data, const char *name, const char *arg);

typedef struct mem mem_t;
typedef struct mem_srv mem_srv_t;
typedef struct mem_srvs mem_srvs_t;

TAILQ_HEAD(mem_srvs, mem_srv);

struct mem {
    mem_srvs_t msrvs;
    const hpd_module_t *context;
    hpd_adapter_t *adapter;
};

struct mem_srv {
    TAILQ_ENTRY(mem_srv) HPD_TAILQ_FIELD;
    mem_t *mem;
    char *dev;
    char *srv;
    char *set;
    hpd_value_t *value;
    hpd_service_t *service;
};

hpd_error_t hpd_mem_alloc(hpd_module_def_t *mdef)
{
    mem_t *mem;
    HPD_CALLOC(mem, 1, mem_t);
    TAILQ_INIT(&mem->msrvs);

    mdef->on_create = mem_on_create;
    mdef->on_destroy = mem_on_destroy;
    mdef->on_start = mem_on_start;
    mdef->on_stop = mem_on_stop;
    mdef->on_parse_opt = mem_on_parse_opt;
    mdef->data = mem;

    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t hpd_mem_add(hpd_module_def_t *mdef, const char *dev, const char *srv)
{
    mem_t *mem = mdef->data;

    mem_srv_t *msrv;
    HPD_CALLOC(msrv, 1, mem_srv_t);
    HPD_STR_CPY(msrv->dev, dev);
    HPD_STR_CPY(msrv->srv, srv);
    msrv->mem = mem;
    TAILQ_INSERT_TAIL(&mem->msrvs, msrv, HPD_TAILQ_FIELD);

    return HPD_E_SUCCESS;

    alloc_error:
    if (msrv) {
        free(msrv->dev);
        free(msrv->srv);
    }
    free(msrv);
    return HPD_E_ALLOC;
}

hpd_error_t hpd_mem_add_set(hpd_module_def_t *mdef, const char *dev, const char *srv, const char *val)
{
    mem_t *mem = mdef->data;

    mem_srv_t *msrv;
    HPD_CALLOC(msrv, 1, mem_srv_t);
    HPD_STR_CPY(msrv->dev, dev);
    HPD_STR_CPY(msrv->srv, srv);
    HPD_STR_CPY(msrv->set, val);
    msrv->mem = mem;

    TAILQ_INSERT_TAIL(&mem->msrvs, msrv, HPD_TAILQ_FIELD);

    return HPD_E_SUCCESS;

    alloc_error:
    if (msrv) {
        free(msrv->dev);
        free(msrv->srv);
        free(msrv->set);
    }
    free(msrv);
    return HPD_E_ALLOC;
}

hpd_error_t hpd_mem_free(hpd_module_def_t *mdef)
{
    mem_t *mem = mdef->data;

    mem_srv_t *msrv, *tmp;
    TAILQ_FOREACH_SAFE(msrv, &mem->msrvs, HPD_TAILQ_FIELD, tmp) {
        free(msrv->dev);
        free(msrv->srv);
        free(msrv);
    }

    free(mem);
    return HPD_E_SUCCESS;
}

static hpd_status_t mem_on_get(void *data, hpd_request_t *req)
{
    mem_srv_t *msrv = data;
    
    if (!msrv->value) return HPD_S_200;

    hpd_value_t *value;
    hpd_value_copy(msrv->mem->context, &value, msrv->value);

    hpd_response_t *res;
    hpd_response_alloc(&res, req, HPD_S_200);
    hpd_response_set_value(res, value);
    hpd_respond(res);

    return HPD_S_NONE;
}

static hpd_status_t mem_on_put(void *data, hpd_request_t *req)
{
    mem_srv_t *msrv = data;
    const hpd_value_t *value;
    hpd_request_get_value(req, &value);
    hpd_value_copy(msrv->mem->context, &msrv->value, value);

    hpd_value_t *val;
    hpd_value_copy(msrv->mem->context, &val, value);
    hpd_changed(msrv->service, val);
    
    return HPD_S_200;
}

static hpd_error_t mem_on_create(void **data, const hpd_module_t *context)
{
    const hpd_module_def_t *mdef;
    hpd_module_get_def(context, &mdef);
    mem_t *mem = mdef->data;

    mem->context = context;

    (*data) = mem;
    return HPD_E_SUCCESS;
}

static hpd_error_t mem_on_destroy(void *data)
{
    return HPD_E_SUCCESS;
}

static hpd_error_t mem_on_start(void *data)
{
    mem_t *mem = data;

    const char *mid;
    hpd_module_get_id(mem->context, &mid);

    hpd_adapter_alloc(&mem->adapter, mem->context, mid);

    mem_srv_t *msrv;
    TAILQ_FOREACH(msrv, &mem->msrvs, HPD_TAILQ_FIELD) {
        hpd_device_t *dev;
        hpd_adapter_get_device(mem->adapter, msrv->dev, &dev);

        if (!dev) {
            hpd_device_alloc(&dev, mem->context, msrv->dev);
            hpd_device_attach(mem->adapter, dev);
        }

        hpd_service_alloc(&msrv->service, mem->context, msrv->srv);

        hpd_service_set_data(msrv->service, msrv, NULL);
        hpd_service_set_actions(msrv->service,
                                HPD_M_GET, mem_on_get,
                                HPD_M_PUT, mem_on_put
        );

        if (msrv->set)
            hpd_value_alloc(&msrv->value, mem->context, msrv->set, HPD_NULL_TERMINATED);

        hpd_service_attach(dev, msrv->service);
    }

    hpd_adapter_attach(mem->adapter);

    return HPD_E_SUCCESS;
}

static hpd_error_t mem_on_stop(void *data)
{
    mem_t *mem = data;
    hpd_adapter_detach(mem->adapter);
    return HPD_E_SUCCESS;
}

static hpd_error_t mem_on_parse_opt(void *data, const char *name, const char *arg)
{
    return HPD_E_ARGUMENT;
}
