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

#include "discovery.h"
#include "daemon.h"
#include "hpd-0.6/hpd_api.h"
#include "event.h"
#include "log.h"
#include "comm.h"
#include "model.h"

hpd_error_t hpd_id_changed(const hpd_service_id_t *id, hpd_value_t *val)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!val) LOG_RETURN_E_NULL(hpd);
    if (!hpd->loop) LOG_RETURN_HPD_STOPPED(hpd);
    return event_changed(id, val);
}

hpd_error_t hpd_changed(const hpd_service_t *service, hpd_value_t *val)
{
    hpd_error_t rc, rc2;

    if (!service) return HPD_E_NULL;
    const hpd_module_t *context = service->context;
    hpd_t *hpd = context->hpd;
    if (!val) LOG_RETURN_E_NULL(hpd);
    if (!service->device || !service->device->adapter || !service->device->adapter->configuration)
        LOG_RETURN_DETACHED(hpd);
    if (!service->device->adapter->configuration->hpd->loop) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_service_id_t *sid;
    if ((rc = discovery_alloc_sid(&sid, context,
                                  service->device->adapter->id, service->device->id, service->id)))
        return rc;

    rc = event_changed(sid, val);

    if ((rc2 = discovery_free_sid(sid))) {
        if (rc != HPD_E_SUCCESS) rc = rc2;
        else LOG_ERROR(hpd, "free function failed [code: %i].", rc2);
    }

    return rc;
}

hpd_error_t hpd_listener_alloc(hpd_listener_t **listener, const hpd_module_t *context)
{
    if (!context) return HPD_E_NULL;
    if (!listener) LOG_RETURN_E_NULL(context->hpd);
    return event_alloc_listener(listener, context);
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

hpd_error_t hpd_listener_set_adapter_callback(hpd_listener_t *listener, hpd_adapter_f on_attach, hpd_adapter_f on_detach, hpd_adapter_f on_change)
{
    if (!listener) return HPD_E_NULL;
    return event_set_adapter_callback(listener, on_attach, on_detach, on_change);
}

hpd_error_t hpd_listener_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach, hpd_device_f on_change)
{
    if (!listener) return HPD_E_NULL;
    return event_set_device_callback(listener, on_attach, on_detach, on_change);
}

hpd_error_t hpd_listener_set_service_callback(hpd_listener_t *listener, hpd_service_f on_attach, hpd_service_f on_detach, hpd_service_f on_change)
{
    if (!listener) return HPD_E_NULL;
    return event_set_service_callback(listener, on_attach, on_detach, on_change);
}

hpd_error_t hpd_listener_set_log_callback(hpd_listener_t *listener, hpd_log_f on_log)
{
    if (!listener) return HPD_E_NULL;
    return event_set_log_callback(listener, on_log);
}

hpd_error_t hpd_subscribe(hpd_listener_t *listener)
{
    if (!listener) return HPD_E_NULL;
    hpd_t *hpd = listener->context->hpd;
    if (!listener->on_change && !listener->on_dev_attach && !listener->on_dev_detach && !listener->on_log)
        LOG_RETURN(hpd, HPD_E_ARGUMENT, "Listener do not contain any callbacks.");
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    return event_subscribe(listener);
}

hpd_error_t hpd_listener_free(hpd_listener_t *listener)
{
    if (!listener) return HPD_E_NULL;
    return event_unsubscribe(listener);
}

hpd_error_t hpd_listener_get_data(const hpd_listener_t *listener, void **data)
{
    if (!listener) return HPD_E_NULL;
    if (!data) LOG_RETURN_E_NULL(listener->context->hpd);
    return event_get_listener_data(listener, data);
}

hpd_error_t hpd_foreach_attached(const hpd_listener_t *listener)
{
    if (!listener) return HPD_E_NULL;
    if (!listener->on_dev_attach)
        LOG_RETURN(listener->context->hpd, HPD_E_ARGUMENT, "Listener do not contain an on_dev_attach callback.");
    return event_foreach_attached(listener);
}
