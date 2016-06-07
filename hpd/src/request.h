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

#ifndef HOMEPORT_REQUEST_H
#define HOMEPORT_REQUEST_H

#include "hpd/hpd_types.h"

#ifdef __cplusplus
extern "C" {
#endif

hpd_error_t request_alloc_request(hpd_request_t **request, const hpd_service_id_t *id, hpd_method_t method, hpd_response_f on_response);
hpd_error_t request_free_request(hpd_request_t *request);
hpd_error_t request_set_request_value(hpd_request_t *request, hpd_value_t *value);
hpd_error_t request_set_request_data(hpd_request_t *request, void *data, hpd_free_f on_free);
hpd_error_t request_request(hpd_request_t *request);
hpd_error_t request_get_request_service(const hpd_request_t *req, const hpd_service_id_t **id);
hpd_error_t request_get_request_method(const hpd_request_t *req, hpd_method_t *method);
hpd_error_t request_get_request_value(const hpd_request_t *req, const hpd_value_t **value);
hpd_error_t request_alloc_response(hpd_response_t **response, hpd_request_t *request, hpd_status_t status);
hpd_error_t request_free_response(hpd_response_t *response);
hpd_error_t request_set_response_value(hpd_response_t *response, hpd_value_t *value);
hpd_error_t request_respond(hpd_response_t *response);
hpd_error_t request_get_response_status(const hpd_response_t *response, hpd_status_t *status);
hpd_error_t request_get_response_value(const hpd_response_t *response, const hpd_value_t **value);
hpd_error_t request_get_response_request_data(const hpd_response_t *response, void **data);
hpd_error_t request_get_response_request_service(const hpd_response_t *response, const hpd_service_id_t **service);
hpd_error_t request_get_response_request_method(const hpd_response_t *response, hpd_method_t *method);
hpd_error_t request_get_response_request_value(const hpd_response_t *response, const hpd_value_t **value);

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_REQUEST_H
