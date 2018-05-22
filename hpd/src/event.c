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

#include "daemon.h"
#include "event.h"
#include "discovery.h"
#include "value.h"
#include "log.h"
#include "comm.h"
#include "model.h"

hpd_error_t event_alloc_listener(hpd_listener_t **listener, const hpd_module_t *context)
{
    HPD_CALLOC(*listener, 1, hpd_listener_t);
    (*listener)->context = context;
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC(context->hpd);
}

hpd_error_t event_free_listener(hpd_listener_t *listener)
{
    if (listener->on_free) listener->on_free(listener->data);
    free(listener);
    return HPD_E_SUCCESS;
}

hpd_error_t event_set_listener_data(hpd_listener_t *listener, void *data, hpd_free_f on_free)
{
    if (listener->on_free) listener->on_free(listener->data);
    listener->data = data;
    listener->on_free = on_free;
    return HPD_E_SUCCESS;
}

hpd_error_t event_set_value_callback(hpd_listener_t *listener, hpd_value_f on_change)
{
    listener->on_change = on_change;
    return HPD_E_SUCCESS;
}

hpd_error_t event_set_adapter_callback(hpd_listener_t *listener, hpd_adapter_f on_attach, hpd_adapter_f on_detach, hpd_adapter_f on_change)
{
    listener->on_adp_attach = on_attach;
    listener->on_adp_detach = on_detach;
    listener->on_adp_change = on_change;
    return HPD_E_SUCCESS;
}

hpd_error_t event_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach, hpd_device_f on_change)
{
    listener->on_dev_attach = on_attach;
    listener->on_dev_detach = on_detach;
    listener->on_dev_change = on_change;
    return HPD_E_SUCCESS;
}

hpd_error_t event_set_service_callback(hpd_listener_t *listener, hpd_service_f on_attach, hpd_service_f on_detach, hpd_service_f on_change)
{
    listener->on_srv_attach = on_attach;
    listener->on_srv_detach = on_detach;
    listener->on_srv_change = on_change;
    return HPD_E_SUCCESS;
}

hpd_error_t event_set_log_callback(hpd_listener_t *listener, hpd_log_f on_log)
{
    listener->on_log = on_log;
    return HPD_E_SUCCESS;
}

hpd_error_t event_subscribe(hpd_listener_t *listener)
{
    TAILQ_INSERT_TAIL(&listener->context->hpd->configuration->listeners, listener, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t event_unsubscribe(hpd_listener_t *listener)
{
    if (listener) {
        if (listener->on_free) listener->on_free(listener->data);
        TAILQ_REMOVE(&listener->context->hpd->configuration->listeners, listener, HPD_TAILQ_FIELD);
        free(listener);
    }
    return HPD_E_SUCCESS;
}

hpd_error_t event_get_listener_data(const hpd_listener_t *listener, void **data)
{
    (*data) = listener->data;
    return HPD_E_SUCCESS;
}

hpd_error_t event_foreach_attached(const hpd_listener_t *listener)
{
    hpd_error_t rc;
    const hpd_module_t *context = listener->context;
    hpd_configuration_t *configuration = context->hpd->configuration;
    hpd_adapter_t *adapter;
    TAILQ_FOREACH(adapter, &configuration->adapters, HPD_TAILQ_FIELD) {
        hpd_adapter_id_t *aid;
        if ((rc = discovery_alloc_aid(&aid, context, adapter->id))) return rc;
        listener->on_adp_attach(listener->data, aid);
        if ((rc = discovery_free_aid(aid))) return rc;
    }
    return HPD_E_SUCCESS;
}

static void event_on_changed(hpd_ev_loop_t *loop, ev_async *w, int revents)
{
    hpd_error_t rc;
    hpd_ev_async_t *async = w->data;
    hpd_t *hpd = async->hpd;
    hpd_service_id_t *id = async->service;
    hpd_value_t *value = async->value;

    TAILQ_REMOVE(&hpd->changed_watchers, async, HPD_TAILQ_FIELD);
    ev_async_stop(loop, w);
    free(async);

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_change) listener->on_change(listener->data, id, value);
    }

    if ((rc = discovery_free_sid(id))) {
        LOG_ERROR(hpd, "free function failed [code: %i].", rc);
    }

    if ((rc = value_free(value))) {
        LOG_ERROR(hpd, "free function failed [code: %i].", rc);
    }
}

hpd_error_t event_changed(const hpd_service_id_t *id, hpd_value_t *val)
{
    hpd_error_t rc = HPD_E_ALLOC;
    hpd_ev_async_t *async;
    hpd_t *hpd = id->device.adapter.context->hpd;
    HPD_CALLOC(async, 1, hpd_ev_async_t);
    HPD_CPY_ALLOC(async->value, val, hpd_value_t);
    if ((rc = discovery_copy_sid(&async->service, id))) goto copy_sid_error;
    async->hpd = hpd;
    ev_async_init(&async->watcher, event_on_changed);
    async->watcher.data = async;
    ev_async_start(hpd->loop, &async->watcher);
    ev_async_send(hpd->loop, &async->watcher);
    TAILQ_INSERT_TAIL(&hpd->changed_watchers, async, HPD_TAILQ_FIELD);
    free(val);
    return HPD_E_SUCCESS;

    copy_sid_error:
    free(async->value);
    alloc_error:
    free(async);
    switch (rc) {
        case HPD_E_ALLOC:
            LOG_RETURN_E_ALLOC(hpd);
        default:
            return rc;
    }
}

hpd_error_t event_inform_adp_attached(hpd_adapter_t *adapter)
{
    hpd_error_t rc;

    const hpd_module_t *context = adapter->context;
    hpd_t *hpd = context->hpd;

    hpd_adapter_id_t *aid;
    if ((rc = discovery_alloc_aid(&aid, context, adapter->id))) return rc;
    
    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_adp_attach) listener->on_adp_attach(listener->data, aid);
    }
    
    return discovery_free_aid(aid);
}

hpd_error_t event_inform_adp_detached(hpd_adapter_t *adapter)
{
    hpd_error_t rc;

    const hpd_module_t *context = adapter->context;
    hpd_t *hpd = context->hpd;

    hpd_adapter_id_t *aid;
    if ((rc = discovery_alloc_aid(&aid, context, adapter->id))) return rc;

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_adp_detach) listener->on_adp_detach(listener->data, aid);
    }

    return discovery_free_aid(aid);
}

hpd_error_t event_inform_adp_changed(hpd_adapter_t *adapter)
{
    hpd_error_t rc;

    const hpd_module_t *context = adapter->context;
    hpd_t *hpd = context->hpd;

    hpd_adapter_id_t *aid;
    if ((rc = discovery_alloc_aid(&aid, context, adapter->id))) return rc;

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_adp_change) listener->on_adp_change(listener->data, aid);
    }

    return discovery_free_aid(aid);
}

hpd_error_t event_inform_dev_attached(hpd_device_t *device)
{
    hpd_error_t rc;

    const hpd_module_t *context = device->context;
    hpd_t *hpd = context->hpd;
    hpd_adapter_t *adapter = device->adapter;

    hpd_device_id_t *did;
    if ((rc = discovery_alloc_did(&did, context, adapter->id, device->id))) return rc;

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_dev_attach) listener->on_dev_attach(listener->data, did);
    }

    return discovery_free_did(did);
}

hpd_error_t event_inform_dev_detached(hpd_device_t *device)
{
    hpd_error_t rc;

    const hpd_module_t *context = device->context;
    hpd_t *hpd = context->hpd;
    hpd_adapter_t *adapter = device->adapter;

    hpd_device_id_t *did;
    if ((rc = discovery_alloc_did(&did, context, adapter->id, device->id))) return rc;

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_dev_detach) listener->on_dev_detach(listener->data, did);
    }

    return discovery_free_did(did);
}

hpd_error_t event_inform_dev_changed(hpd_device_t *device)
{
    hpd_error_t rc;

    const hpd_module_t *context = device->context;
    hpd_t *hpd = context->hpd;
    hpd_adapter_t *adapter = device->adapter;

    hpd_device_id_t *did;
    if ((rc = discovery_alloc_did(&did, context, adapter->id, device->id))) return rc;

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_dev_change) listener->on_dev_change(listener->data, did);
    }

    return discovery_free_did(did);
}

hpd_error_t event_inform_srv_attached(hpd_service_t *service)
{
    hpd_error_t rc;

    const hpd_module_t *context = service->context;
    hpd_t *hpd = context->hpd;
    hpd_device_t *device = service->device;
    hpd_adapter_t *adapter = device->adapter;

    hpd_service_id_t *sid;
    if ((rc = discovery_alloc_sid(&sid, context, adapter->id, device->id, service->id))) return rc;

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_srv_attach) listener->on_srv_attach(listener->data, sid);
    }

    return discovery_free_sid(sid);
}

hpd_error_t event_inform_srv_detached(hpd_service_t *service)
{
    hpd_error_t rc;

    const hpd_module_t *context = service->context;
    hpd_t *hpd = context->hpd;
    hpd_device_t *device = service->device;
    hpd_adapter_t *adapter = device->adapter;

    hpd_service_id_t *sid;
    if ((rc = discovery_alloc_sid(&sid, context, adapter->id, device->id, service->id))) return rc;

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_srv_detach) listener->on_srv_detach(listener->data, sid);
    }

    return discovery_free_sid(sid);
}

hpd_error_t event_inform_srv_changed(hpd_service_t *service)
{
    hpd_error_t rc;

    const hpd_module_t *context = service->context;
    hpd_t *hpd = context->hpd;
    hpd_device_t *device = service->device;
    hpd_adapter_t *adapter = device->adapter;

    hpd_service_id_t *sid;
    if ((rc = discovery_alloc_sid(&sid, context, adapter->id, device->id, service->id))) return rc;

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_srv_change) listener->on_srv_change(listener->data, sid);
    }

    return discovery_free_sid(sid);
}

hpd_error_t event_log(hpd_t *hpd, const char *msg)
{
    hpd_listener_t *listener;
    if (hpd->configuration) {
        TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
            if (listener->on_log) listener->on_log(listener->data, msg);
        }
    }
    return HPD_E_SUCCESS;
}
