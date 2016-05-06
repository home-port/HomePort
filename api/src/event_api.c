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

#include "discovery.h"
#include "daemon.h"
#include "hpd_api.h"
#include "hpd_internal_api.h"
#include "old_model.h"
#include "event.h"

hpd_error_t hpd_changed(hpd_service_id_t *id, hpd_value_t *val)
{
    if (!id || !val) return HPD_E_NULL;
    if (!id->hpd->loop) return HPD_E_STOPPED;
    return event_changed(id, val);
}

hpd_error_t hpd_listener_alloc_hpd(hpd_listener_t **listener, hpd_t *hpd)
{
    if (!listener || !hpd) return HPD_E_NULL;
    return event_alloc_listener_hpd(listener, hpd);
}

hpd_error_t hpd_listener_alloc_adapter(hpd_listener_t **listener, hpd_adapter_id_t *id)
{
    if (!listener || !id) return HPD_E_NULL;
    return event_alloc_listener_adapter(listener, id);
}

hpd_error_t hpd_listener_alloc_device(hpd_listener_t **listener, hpd_device_id_t *id)
{
    if (!listener || !id) return HPD_E_NULL;
    return event_alloc_listener_device(listener, id);
}

hpd_error_t hpd_listener_alloc_service(hpd_listener_t **listener, hpd_service_id_t *id)
{
    if (!listener || !id) return HPD_E_NULL;
    return event_alloc_listener_service(listener, id);
}

hpd_error_t hpd_listener_set_data(hpd_listener_t *listener, void *data, hpd_free_f on_free)
{
    if (!listener) return HPD_E_NULL;
    return event_set_listener_data(listener, data, on_free);
}

hpd_error_t hpd_listener_set_value_callback(hpd_listener_t *listener, hpd_value_f on_change)
{
    if (!listener) return HPD_E_NULL;
    return event_set_value_callback(listener, on_change);
}
hpd_error_t hpd_listener_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach)
{
    if (!listener) return HPD_E_NULL;
    return event_set_device_callback(listener, on_attach, on_detach);
}

hpd_error_t hpd_subscribe(hpd_listener_t *listener, hpd_listener_ref_t **ref)
{
    if (!listener) return HPD_E_NULL;
    if (!listener->on_change && !listener->on_attach && !listener->on_detach) return HPD_E_ARGUMENT;
    switch (listener->type) {
        case CONFIGURATION_LISTENER:
            if (!listener->hpd->configuration) return HPD_E_STOPPED;
            break;
        case ADAPTER_LISTENER:
            if (!listener->adapter->configuration) return HPD_E_STOPPED;
            break;
        case DEVICE_LISTENER:
            if (!listener->device->adapter->configuration) return HPD_E_STOPPED;
            break;
        case SERVICE_LISTENER:
            if (!listener->on_change) return HPD_E_ARGUMENT;
            if (!listener->service->device->adapter->configuration) return HPD_E_STOPPED;
            break;
    }
    return event_subscribe(listener, ref);
}

hpd_error_t hpd_listener_free(hpd_listener_ref_t *listener)
{
    if (!listener) return HPD_E_NULL;
    return event_unsubscribe(listener);
}

hpd_error_t hpd_listener_get_data(hpd_listener_ref_t *listener, void **data)
{
    if (!listener || !data) return HPD_E_NULL;
    if (!listener->listener) return HPD_E_NOT_FOUND;
    return event_get_listener_ref_data(listener, data);
}

hpd_error_t hpd_foreach_attached(hpd_listener_ref_t *listener)
{
    if (!listener) return HPD_E_NULL;
    if (!listener->listener) return HPD_E_NOT_FOUND;
    switch (listener->listener->type) {
        case CONFIGURATION_LISTENER:break;
        case ADAPTER_LISTENER:break;
        case DEVICE_LISTENER:break;
        case SERVICE_LISTENER:return HPD_E_ARGUMENT;
    }
    if (!listener->listener->on_attach) return HPD_E_ARGUMENT;
    return event_foreach_attached(listener);
}
