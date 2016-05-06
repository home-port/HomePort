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

hpd_error_t event_alloc_listener(hpd_listener_t **listener, hpd_t *hpd)
{
    HPD_CALLOC(*listener, 1, hpd_listener_t);
    (*listener)->hpd = hpd;
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
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

hpd_error_t event_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach)
{
    listener->on_attach = on_attach;
    listener->on_detach = on_detach;
    return HPD_E_SUCCESS;
}

hpd_error_t event_subscribe(hpd_listener_t *listener)
{
    hpd_error_t rc = HPD_E_ALLOC;
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

hpd_error_t event_get_listener_data(hpd_listener_t *listener, void **data)
{
    (*data) = listener->data;
    return HPD_E_SUCCESS;
}

hpd_error_t event_foreach_attached(hpd_listener_t *listener)
{
    hpd_error_t rc;
    configuration_t *configuration = listener->hpd->configuration;
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
