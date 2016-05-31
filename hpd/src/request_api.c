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
#include "request.h"
#include "log.h"
#include "comm.h"

hpd_error_t hpd_request_alloc(hpd_request_t **request, const hpd_service_id_t *id, hpd_method_t method,
                              hpd_response_f on_response)
{
    if (!request || !id) LOG_RETURN_E_NULL();
    if (method <= HPD_M_NONE || method >= HPD_M_COUNT)
        LOG_RETURN(HPD_E_ARGUMENT, "Unknown method given to %s().", __func__);
    return request_alloc_request(request, id, method, on_response);
}

hpd_error_t hpd_request_free(hpd_request_t *request)
{
    if (!request) LOG_RETURN_E_NULL();
    return request_free_request(request);
}

hpd_error_t hpd_request_set_value(hpd_request_t *request, hpd_value_t *value)
{
    if (!request) LOG_RETURN_E_NULL();
    return request_set_request_value(request, value);
}

hpd_error_t hpd_request_set_data(hpd_request_t *request, void *data, hpd_free_f on_free)
{
    if (!request) LOG_RETURN_E_NULL();
    return request_set_request_data(request, data, on_free);
}

hpd_error_t hpd_request(hpd_request_t *request)
{
    if (!request) LOG_RETURN_E_NULL();
    if (!request->service->device.adapter.hpd->loop) LOG_RETURN_HPD_STOPPED();
    return request_request(request);
}

hpd_error_t hpd_request_get_service(const hpd_request_t *req, const hpd_service_id_t **id)
{
    if (!req || !id) LOG_RETURN_E_NULL();
    return request_get_request_service(req, id);
}

hpd_error_t hpd_request_get_method(const hpd_request_t *req, hpd_method_t *method)
{
    if (!req || !method) LOG_RETURN_E_NULL();
    return request_get_request_method(req, method);
}

hpd_error_t hpd_request_get_value(const hpd_request_t *req, hpd_value_t **value)
{
    if (!req || !value) LOG_RETURN_E_NULL();
    return request_get_request_value(req, value);
}

hpd_error_t hpd_response_alloc(hpd_response_t **response, hpd_request_t *request, hpd_status_t status)
{
    if (!response || !request) LOG_RETURN_E_NULL();
    switch (status) {
        case HPD_S_100:break;
        case HPD_S_101:break;
        case HPD_S_200:break;
        case HPD_S_201:break;
        case HPD_S_202:break;
        case HPD_S_203:break;
        case HPD_S_204:break;
        case HPD_S_205:break;
        case HPD_S_206:break;
        case HPD_S_300:break;
        case HPD_S_301:break;
        case HPD_S_302:break;
        case HPD_S_303:break;
        case HPD_S_304:break;
        case HPD_S_305:break;
        case HPD_S_306:break;
        case HPD_S_307:break;
        case HPD_S_400:break;
        case HPD_S_401:break;
        case HPD_S_402:break;
        case HPD_S_403:break;
        case HPD_S_404:break;
        case HPD_S_405:break;
        case HPD_S_406:break;
        case HPD_S_407:break;
        case HPD_S_408:break;
        case HPD_S_409:break;
        case HPD_S_410:break;
        case HPD_S_411:break;
        case HPD_S_412:break;
        case HPD_S_413:break;
        case HPD_S_414:break;
        case HPD_S_415:break;
        case HPD_S_416:break;
        case HPD_S_417:break;
        case HPD_S_500:break;
        case HPD_S_501:break;
        case HPD_S_502:break;
        case HPD_S_503:break;
        case HPD_S_504:break;
        case HPD_S_505:break;
        default:
            LOG_RETURN(HPD_E_ARGUMENT, "Unknown status code given to %s().", __func__);
    }
    return request_alloc_response(response, request, status);
}

hpd_error_t hpd_response_free(hpd_response_t *response)
{
    if (!response) LOG_RETURN_E_NULL();
    return request_free_response(response);
}

hpd_error_t hpd_response_set_value(hpd_response_t *response, hpd_value_t *value)
{
    if (!response) LOG_RETURN_E_NULL();
    return request_set_response_value(response, value);
}

hpd_error_t hpd_respond(hpd_response_t *response)
{
    if (!response) LOG_RETURN_E_NULL();
    if (!response->request->service->device.adapter.hpd->loop) LOG_RETURN_HPD_STOPPED();
    return request_respond(response);
}

hpd_error_t hpd_response_get_status(hpd_response_t *response, hpd_status_t *status)
{
    if (!response || !status) LOG_RETURN_E_NULL();
    return request_get_response_status(response, status);
}

hpd_error_t hpd_response_get_value(hpd_response_t *response, hpd_value_t **value)
{
    if (!response || !value) LOG_RETURN_E_NULL();
    return request_get_response_value(response, value);
}

hpd_error_t hpd_response_get_request_data(const hpd_response_t *response, void **data)
{
    if (!response || !data) LOG_RETURN_E_NULL();
    return request_get_response_request_data(response, data);
}

hpd_error_t hpd_response_get_request_service(const hpd_response_t *response, const hpd_service_id_t **service)
{
    if (!response || !service) LOG_RETURN_E_NULL();
    return request_get_response_request_service(response, service);
}

hpd_error_t hpd_response_get_request_method(const hpd_response_t *response, hpd_method_t *method)
{
    if (!response || !method) LOG_RETURN_E_NULL();
    return request_get_response_request_method(response, method);
}

hpd_error_t hpd_response_get_request_value(const hpd_response_t *response, hpd_value_t **value)
{
    if (!response || !value) LOG_RETURN_E_NULL();
    return request_get_response_request_value(response, value);
}

