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

#ifndef HOMEPORT_EVENT_H
#define HOMEPORT_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hpd_types.h"

hpd_error_t event_alloc_listener(hpd_listener_t **listener, hpd_t *hpd);
hpd_error_t event_free_listener(hpd_listener_t *listener);

hpd_error_t event_set_listener_data(hpd_listener_t *listener, void *data, hpd_free_f on_free);
hpd_error_t event_set_value_callback(hpd_listener_t *listener, hpd_value_f on_change);
hpd_error_t event_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach);

hpd_error_t event_subscribe(hpd_listener_t *listener);
hpd_error_t event_unsubscribe(hpd_listener_t *listener);

hpd_error_t event_get_listener_data(hpd_listener_t *listener, void **data);

hpd_error_t event_foreach_attached(hpd_listener_t *listener);

hpd_error_t event_changed(hpd_service_id_t *id, hpd_value_t *val);

hpd_error_t event_inform_adapter_attached(hpd_adapter_t *adapter);
hpd_error_t event_inform_adapter_detached(hpd_adapter_t *adapter);
hpd_error_t event_inform_device_attached(hpd_device_t *device);
hpd_error_t event_inform_device_detached(hpd_device_t *device);

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_EVENT_H
