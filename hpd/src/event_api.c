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
#include "event.h"
#include "log.h"
#include "comm.h"

hpd_error_t hpd_changed(hpd_service_id_t *id, hpd_value_t *val)
{
    if (!id || !val) LOG_RETURN_E_NULL();
    if (!id->device.adapter.hpd->loop) LOG_RETURN_HPD_STOPPED();
    return event_changed(id, val);
}

hpd_error_t hpd_listener_alloc(hpd_listener_t **listener, hpd_t *hpd)
{
    if (!listener || !hpd) LOG_RETURN_E_NULL();
    return event_alloc_listener(listener, hpd);
}

hpd_error_t hpd_listener_set_data(hpd_listener_t *listener, void *data, hpd_free_f on_free)
{
    if (!listener) LOG_RETURN_E_NULL();
    return event_set_listener_data(listener, data, on_free);
}

hpd_error_t hpd_listener_set_value_callback(hpd_listener_t *listener, hpd_value_f on_change)
{
    if (!listener) LOG_RETURN_E_NULL();
    return event_set_value_callback(listener, on_change);
}
hpd_error_t hpd_listener_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach)
{
    if (!listener) LOG_RETURN_E_NULL();
    return event_set_device_callback(listener, on_attach, on_detach);
}

hpd_error_t hpd_subscribe(hpd_listener_t *listener)
{
    if (!listener) LOG_RETURN_E_NULL();
    if (!listener->on_change && !listener->on_attach && !listener->on_detach)
        LOG_RETURN(HPD_E_ARGUMENT, "Listener do not contain any callbacks.");
    if (!listener->hpd->configuration) LOG_RETURN_HPD_STOPPED();
    return event_subscribe(listener);
}

hpd_error_t hpd_listener_free(hpd_listener_t *listener)
{
    if (!listener) LOG_RETURN_E_NULL();
    return event_unsubscribe(listener);
}

hpd_error_t hpd_listener_get_data(hpd_listener_t *listener, void **data)
{
    if (!listener || !data) LOG_RETURN_E_NULL();
    return event_get_listener_data(NULL, data);
}

hpd_error_t hpd_foreach_attached(hpd_listener_t *listener)
{
    if (!listener) LOG_RETURN_E_NULL();
    if (!listener->on_attach)
        LOG_RETURN(HPD_E_ARGUMENT, "Listener do not contain an on_attach callback.");
    return event_foreach_attached(listener);
}
