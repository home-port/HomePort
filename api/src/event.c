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

#include "daemon.h"
#include "event.h"
#include "old_model.h"
#include "hpd_common.h"
#include "discovery.h"

hpd_error_t event_alloc_listener_hpd(hpd_listener_t **listener, hpd_t *hpd)
{
    HPD_CALLOC(*listener, 1, hpd_listener_t);
    (*listener)->type = CONFIGURATION_LISTENER;
    (*listener)->hpd = hpd;
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t event_alloc_listener_adapter(hpd_listener_t **listener, hpd_adapter_id_t *id)
{
    HPD_CALLOC(*listener, 1, hpd_listener_t);
    (*listener)->type = ADAPTER_LISTENER;
    (*listener)->aid = id;
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t event_alloc_listener_device(hpd_listener_t **listener, hpd_device_id_t *id)
{
    HPD_CALLOC(*listener, 1, hpd_listener_t);
    (*listener)->type = DEVICE_LISTENER;
    (*listener)->did = id;
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t event_alloc_listener_service(hpd_listener_t **listener, hpd_service_id_t *id)
{
    HPD_CALLOC(*listener, 1, hpd_listener_t);
    (*listener)->type = SERVICE_LISTENER;
    (*listener)->sid = id;
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t event_free_listener(hpd_listener_t *listener)
{
    if (listener->on_free) listener->on_free(listener->data);
    if (listener->ref) listener->ref->listener = NULL;
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

hpd_error_t event_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach)
{
    listener->on_attach = on_attach;
    listener->on_detach = on_detach;
    return HPD_E_SUCCESS;
}

hpd_error_t event_subscribe(hpd_listener_t *listener, hpd_listener_ref_t **ref)
{
    hpd_error_t rc = HPD_E_ALLOC;
    if (ref) HPD_CALLOC(*ref, 1, hpd_listener_ref_t);
    switch (listener->type) {
        case CONFIGURATION_LISTENER: {
            listener->configuration = listener->hpd->configuration;
            TAILQ_INSERT_TAIL(&listener->configuration->listeners, listener, HPD_TAILQ_FIELD);
            break;
        }
        case ADAPTER_LISTENER: {
            if ((rc = discovery_find_adapter(listener->aid, &listener->adapter))) goto find_error;
            TAILQ_INSERT_TAIL(listener->adapter->listeners, listener, HPD_TAILQ_FIELD);
            break;
        }
        case DEVICE_LISTENER: {
            if ((rc = discovery_find_device(listener->did, &listener->device))) goto find_error;
            TAILQ_INSERT_TAIL(listener->device->listeners, listener, HPD_TAILQ_FIELD);
            break;
        }
        case SERVICE_LISTENER: {
            if ((rc = discovery_find_service(listener->sid, &listener->service))) goto find_error;
            TAILQ_INSERT_TAIL(listener->service->listeners, listener, HPD_TAILQ_FIELD);
            break;
        }
    }
    if (ref) {
        (*ref)->listener = listener;
        listener->ref = *ref;
    }
    return HPD_E_SUCCESS;

    find_error:
        free(*ref);
    alloc_error:
        return rc;
}

hpd_error_t event_unsubscribe(hpd_listener_ref_t *ref)
{
    hpd_listener_t *listener = ref->listener;
    if (listener) {
        if (listener->on_free) listener->on_free(listener->data);
        switch (listener->type) {
            case CONFIGURATION_LISTENER:
                TAILQ_REMOVE(&listener->configuration->listeners, listener, HPD_TAILQ_FIELD);
                break;
            case ADAPTER_LISTENER:
                TAILQ_REMOVE(listener->adapter->listeners, listener, HPD_TAILQ_FIELD);
                break;
            case DEVICE_LISTENER:
                TAILQ_REMOVE(listener->device->listeners, listener, HPD_TAILQ_FIELD);
                break;
            case SERVICE_LISTENER:
                TAILQ_REMOVE(listener->service->listeners, listener, HPD_TAILQ_FIELD);
                break;
        }
        free(listener);
    }
    free(ref);
    return HPD_E_SUCCESS;
}

hpd_error_t event_get_listener_ref_data(hpd_listener_ref_t *ref, void **data)
{
    (*data) = ref->listener->data;
    return HPD_E_SUCCESS;
}

hpd_error_t event_foreach_attached(hpd_listener_ref_t *ref)
{
    hpd_error_t rc;
    hpd_listener_t *listener = ref->listener;
    switch (listener->type) {
        case CONFIGURATION_LISTENER: {
            configuration_t *configuration = listener->configuration;
            hpd_adapter_t *adapter;
            hpd_device_t *device;
            HPD_TAILQ_FOREACH(adapter, &configuration->adapters) {
                HPD_TAILQ_FOREACH(device, adapter->devices) {
                    hpd_device_id_t *did;
                    if ((rc = discovery_alloc_did(&did, configuration->data, adapter->id, device->id))) return rc;
                    listener->on_attach(listener->data, did);
                    if ((rc = discovery_free_did(did))) return rc;
                }
            }
            break;
        }
        case ADAPTER_LISTENER: {
            hpd_adapter_t *adapter = listener->adapter;
            configuration_t *configuration = adapter->configuration;
            hpd_device_t *device;
            HPD_TAILQ_FOREACH(device, adapter->devices) {
                hpd_device_id_t *did;
                if ((rc = discovery_alloc_did(&did, configuration->data, adapter->id, device->id))) return rc;
                listener->on_attach(listener->data, did);
                if ((rc = discovery_free_did(did))) return rc;
            }
            break;
        }
        case DEVICE_LISTENER:{
            hpd_device_t *device = listener->device;
            hpd_adapter_t *adapter = device->adapter;
            configuration_t *configuration = adapter->configuration;
            hpd_device_id_t *did;
            if ((rc = discovery_alloc_did(&did, configuration->data, adapter->id, device->id))) return rc;
            listener->on_attach(listener->data, did);
            if ((rc = discovery_free_did(did))) return rc;
            break;
        }
        case SERVICE_LISTENER:
            break;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t event_changed(hpd_service_id_t *id, hpd_value_t *val)
{
    // TODO Send it over the event loop
    // TODO Remember service may die after this function !!!
}

hpd_error_t event_inform_adapter_attached(hpd_adapter_t *adapter)
{
    // TODO Send it over the event loop
    // TODO Remember adapter may die after this function !!!
//    ADAPTER_INFORM(adapter, on_attach);
}

hpd_error_t event_inform_adapter_detached(hpd_adapter_t *adapter)
{
    // TODO Send it over the event loop
    // TODO Remember adapter may die after this function !!!
//    ADAPTER_INFORM(adapter, on_detach);
}

hpd_error_t event_inform_device_attached(hpd_device_t *device)
{
    // TODO Send it over the event loop
    // TODO Remember device may die after this function !!!
//    DEVICE_INFORM(device, on_attach);
}

hpd_error_t event_inform_device_detached(hpd_device_t *device)
{
    // TODO Send it over the event loop
    // TODO Remember device may die after this function !!!
//    DEVICE_INFORM(device, on_detach);
}
