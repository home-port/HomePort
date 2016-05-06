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

#ifndef HOMEPORT_HPD_APPLICATION_API_H
#define HOMEPORT_HPD_APPLICATION_API_H

#include "hpd_shared_api.h"
#include "old_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/// [hpd_request_t functions]
hpd_error_t hpd_request_alloc(hpd_request_t **request, hpd_service_id_t *id, hpd_method_t method, hpd_response_f on_response);
hpd_error_t hpd_request_free(hpd_request_t *request);
hpd_error_t hpd_request_set_value(hpd_request_t *request, hpd_value_t *value);
hpd_error_t hpd_request_set_data(hpd_request_t *request, void *data, hpd_free_f on_free);
hpd_error_t hpd_request(hpd_request_t *request);
/// [hpd_request_t functions]

/// [hpd_response_t functions]
hpd_error_t hpd_response_get_status(hpd_response_t *response, hpd_status_t *status);
hpd_error_t hpd_response_get_value(hpd_response_t *response, hpd_value_t **value);
hpd_error_t hpd_response_get_request_data(const hpd_response_t *response, void **data);
hpd_error_t hpd_response_get_request_service(const hpd_response_t *response, hpd_service_id_t **service);
hpd_error_t hpd_response_get_request_method(const hpd_response_t *response, hpd_method_t *method);
hpd_error_t hpd_response_get_request_value(const hpd_response_t *response, hpd_value_t **value);
/// [hpd_response_t functions]

/// [hpd_listener_t functions]
hpd_error_t hpd_listener_alloc(hpd_listener_t **listener, hpd_t *hpd);
hpd_error_t hpd_listener_set_data(hpd_listener_t *listener, void *data, hpd_free_f on_free);
hpd_error_t hpd_listener_set_value_callback(hpd_listener_t *listener, hpd_value_f on_change);
hpd_error_t hpd_listener_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach);
hpd_error_t hpd_subscribe(hpd_listener_t *listener);
hpd_error_t hpd_listener_free(hpd_listener_t *listener);
hpd_error_t hpd_listener_get_data(hpd_listener_t *listener, void **data);
hpd_error_t hpd_foreach_attached(hpd_listener_t *listener);
/// [hpd_listener_t functions]

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_HPD_APPLICATION_API_H
