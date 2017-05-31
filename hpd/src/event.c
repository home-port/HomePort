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

hpd_error_t event_alloc_listener(hpd_listener_t **listener, hpd_t *hpd)
{
    HPD_CALLOC(*listener, 1, hpd_listener_t);
    (*listener)->hpd = hpd;
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC();
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

hpd_error_t event_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach,
                                      hpd_device_f on_change)
{
    listener->on_dev_attach = on_attach;
    listener->on_dev_detach = on_detach;
    listener->on_dev_change = on_change;
    return HPD_E_SUCCESS;
}

hpd_error_t event_subscribe(hpd_listener_t *listener)
{
    TAILQ_INSERT_TAIL(&listener->hpd->configuration->listeners, listener, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t event_unsubscribe(hpd_listener_t *listener)
{
    if (listener) {
        if (listener->on_free) listener->on_free(listener->data);
        TAILQ_REMOVE(&listener->hpd->configuration->listeners, listener, HPD_TAILQ_FIELD);
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
    hpd_configuration_t *configuration = listener->hpd->configuration;
    hpd_adapter_t *adapter;
    hpd_device_t *device;
    TAILQ_FOREACH(adapter, &configuration->adapters, HPD_TAILQ_FIELD) {
        TAILQ_FOREACH(device, adapter->devices, HPD_TAILQ_FIELD) {
            hpd_device_id_t *did;
            if ((rc = discovery_alloc_did(&did, configuration->hpd, adapter->id, device->id))) return rc;
            listener->on_dev_attach(listener->data, did);
            if ((rc = discovery_free_did(did))) return rc;
        }
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
        LOG_ERROR("free function failed [code: %i].", rc);
    }

    if ((rc = value_free(value))) {
        LOG_ERROR("free function failed [code: %i].", rc);
    }
}

static void event_on_device_attached(hpd_ev_loop_t *loop, ev_async *w, int revents)
{
    hpd_error_t rc;
    hpd_ev_async_t *async = w->data;
    hpd_t *hpd = async->hpd;
    hpd_device_id_t *id = async->device;

    TAILQ_REMOVE(&hpd->device_watchers, async, HPD_TAILQ_FIELD);
    ev_async_stop(loop, w);
    free(async);

    hpd_listener_t *listener;
    // TODO With this loop delayed, hpd_foreach_attached() is likely to provide duplicated calls for the same devices
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_dev_attach) listener->on_dev_attach(listener->data, id);
    }

    if ((rc = discovery_free_did(id))) {
        LOG_ERROR("free function failed [code: %i].", rc);
    }
}

static void event_on_device_detached(hpd_ev_loop_t *loop, ev_async *w, int revents)
{
    hpd_error_t rc;
    hpd_ev_async_t *async = w->data;
    hpd_t *hpd = async->hpd;
    hpd_device_id_t *id = async->device;

    TAILQ_REMOVE(&hpd->device_watchers, async, HPD_TAILQ_FIELD);
    ev_async_stop(loop, w);
    free(async);

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_dev_detach) listener->on_dev_detach(listener->data, id);
    }

    if ((rc = discovery_free_did(id))) {
        LOG_ERROR("free function failed [code: %i].", rc);
    }
}

static void event_on_device_changed(hpd_ev_loop_t *loop, ev_async *w, int revents)
{
    hpd_error_t rc;
    hpd_ev_async_t *async = w->data;
    hpd_t *hpd = async->hpd;
    hpd_device_id_t *id = async->device;

    TAILQ_REMOVE(&hpd->device_watchers, async, HPD_TAILQ_FIELD);
    ev_async_stop(loop, w);
    free(async);

    hpd_listener_t *listener;
    TAILQ_FOREACH(listener, &hpd->configuration->listeners, HPD_TAILQ_FIELD) {
        if (listener->on_dev_change) listener->on_dev_change(listener->data, id);
    }

    if ((rc = discovery_free_did(id))) {
        LOG_ERROR("free function failed [code: %i].", rc);
    }
}

hpd_error_t event_changed(const hpd_service_id_t *id, hpd_value_t *val)
{
    hpd_error_t rc = HPD_E_ALLOC;
    hpd_ev_async_t *async;
    HPD_CALLOC(async, 1, hpd_ev_async_t);
    HPD_CPY_ALLOC(async->value, val, hpd_value_t);
    if ((rc = discovery_copy_sid(&async->service, id))) goto copy_sid_error;
    hpd_t *hpd = id->device.adapter.hpd;
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
            LOG_RETURN_E_ALLOC();
        default:
            return rc;
    }
}

hpd_error_t event_inform_adapter_attached(hpd_adapter_t *adapter)
{
    hpd_error_t rc;
    hpd_device_t *device;
    TAILQ_FOREACH(device, adapter->devices, HPD_TAILQ_FIELD) {
        // TODO Could alternatively remove the attached watchers and return an error
        if ((rc = event_inform_device_attached(device)))
            LOG_ERROR("event_inform_device_attached() [code: %i].", rc);
    }
    return HPD_E_SUCCESS;
}

hpd_error_t event_inform_adapter_detached(hpd_adapter_t *adapter)
{
    hpd_error_t rc;
    hpd_device_t *device;
    TAILQ_FOREACH(device, adapter->devices, HPD_TAILQ_FIELD) {
        // TODO Could alternatively remove the attached watchers and return an error
        if ((rc = event_inform_device_detached(device)))
            LOG_ERROR("event_inform_device_detached() [code: %i].", rc);
    }
    return HPD_E_SUCCESS;
}

hpd_error_t event_inform_device_attached(hpd_device_t *device)
{
    hpd_error_t rc = HPD_E_ALLOC;
    hpd_ev_async_t *async;
    HPD_CALLOC(async, 1, hpd_ev_async_t);
    hpd_adapter_t *adapter = device->adapter;
    hpd_t *hpd = adapter->configuration->hpd;
    if ((rc = discovery_alloc_did(&async->device, hpd, adapter->id, device->id))) goto did_error;
    async->hpd = hpd;
    ev_async_init(&async->watcher, event_on_device_attached);
    async->watcher.data = async;
    ev_async_start(hpd->loop, &async->watcher);
    ev_async_send(hpd->loop, &async->watcher);
    TAILQ_INSERT_TAIL(&hpd->device_watchers, async, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;

    did_error:
    free(async);
    alloc_error:
    switch (rc) {
        case HPD_E_ALLOC:
            LOG_RETURN_E_ALLOC();
        default:
            return rc;
    }
}

hpd_error_t event_inform_device_detached(hpd_device_t *device)
{
    hpd_error_t rc = HPD_E_ALLOC;
    hpd_ev_async_t *async;
    HPD_CALLOC(async, 1, hpd_ev_async_t);
    hpd_adapter_t *adapter = device->adapter;
    hpd_t *hpd = adapter->configuration->hpd;
    if ((rc = discovery_alloc_did(&async->device, hpd, adapter->id, device->id))) goto did_error;
    async->hpd = hpd;
    ev_async_init(&async->watcher, event_on_device_detached);
    async->watcher.data = async;
    ev_async_start(hpd->loop, &async->watcher);
    ev_async_send(hpd->loop, &async->watcher);
    TAILQ_INSERT_TAIL(&hpd->device_watchers, async, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;

    did_error:
    free(async);
    alloc_error:
    switch (rc) {
        case HPD_E_ALLOC:
            LOG_RETURN_E_ALLOC();
        default:
            return rc;
    }
}

hpd_error_t event_inform_device_changed(hpd_device_t *device)
{
    hpd_error_t rc = HPD_E_ALLOC;
    hpd_ev_async_t *async;
    HPD_CALLOC(async, 1, hpd_ev_async_t);
    hpd_adapter_t *adapter = device->adapter;
    hpd_t *hpd = adapter->configuration->hpd;
    if ((rc = discovery_alloc_did(&async->device, hpd, adapter->id, device->id))) goto did_error;
    async->hpd = hpd;
    ev_async_init(&async->watcher, event_on_device_changed);
    async->watcher.data = async;
    ev_async_start(hpd->loop, &async->watcher);
    ev_async_send(hpd->loop, &async->watcher);
    TAILQ_INSERT_TAIL(&hpd->device_watchers, async, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;

    did_error:
    free(async);
    alloc_error:
    switch (rc) {
        case HPD_E_ALLOC:
            LOG_RETURN_E_ALLOC();
        default:
            return rc;
    }
}

