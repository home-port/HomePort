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
#include "hpd/hpd_api.h"
#include "request.h"
#include "log.h"
#include "comm.h"

hpd_error_t hpd_request_alloc(hpd_request_t **request, const hpd_service_id_t *id, hpd_method_t method,
                              hpd_response_f on_response)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!request) LOG_RETURN_E_NULL(hpd);
    if (method <= HPD_M_NONE || method >= HPD_M_COUNT)
        LOG_RETURN(hpd, HPD_E_ARGUMENT, "Unknown method given to %s().", __func__);
    return request_alloc_request(request, id, method, on_response);
}

hpd_error_t hpd_request_free(hpd_request_t *request)
{
    if (!request) return HPD_E_NULL;
    return request_free_request(request);
}

hpd_error_t hpd_request_set_value(hpd_request_t *request, hpd_value_t *value)
{
    if (!request) return HPD_E_NULL;
    return request_set_request_value(request, value);
}

hpd_error_t hpd_request_set_data(hpd_request_t *request, void *data, hpd_free_f on_free)
{
    if (!request) return HPD_E_NULL;
    return request_set_request_data(request, data, on_free);
}

hpd_error_t hpd_request(hpd_request_t *request)
{
    if (!request) return HPD_E_NULL;
    hpd_t *hpd = request->service->device.adapter.context->hpd;
    if (!hpd->loop) LOG_RETURN_HPD_STOPPED(hpd);
    return request_request(request);
}

hpd_error_t hpd_request_get_service(const hpd_request_t *req, const hpd_service_id_t **id)
{
    if (!req) return HPD_E_NULL;
    hpd_t *hpd = req->service->device.adapter.context->hpd;
    if (!id) LOG_RETURN_E_NULL(hpd);
    return request_get_request_service(req, id);
}

hpd_error_t hpd_request_get_method(const hpd_request_t *req, hpd_method_t *method)
{
    if (!req) return HPD_E_NULL;
    hpd_t *hpd = req->service->device.adapter.context->hpd;
    if (!method) LOG_RETURN_E_NULL(hpd);
    return request_get_request_method(req, method);
}

hpd_error_t hpd_request_get_value(const hpd_request_t *req, const hpd_value_t **value)
{
    if (!req) return HPD_E_NULL;
    hpd_t *hpd = req->service->device.adapter.context->hpd;
    if (!value) LOG_RETURN_E_NULL(hpd);
    return request_get_request_value(req, value);
}

hpd_error_t hpd_response_alloc(hpd_response_t **response, hpd_request_t *request, hpd_status_t status)
{
    if (!request) return HPD_E_NULL;
    hpd_t *hpd = request->service->device.adapter.context->hpd;
    if (!response) LOG_RETURN_E_NULL(hpd);
    switch (status) {
        case HPD_S_NONE:break;
        case HPD_S_100:
        case HPD_S_101:
        case HPD_S_200:
        case HPD_S_201:
        case HPD_S_202:
        case HPD_S_203:
        case HPD_S_204:
        case HPD_S_205:
        case HPD_S_206:
        case HPD_S_207:
        case HPD_S_300:
        case HPD_S_301:
        case HPD_S_302:
        case HPD_S_303:
        case HPD_S_304:
        case HPD_S_305:
        case HPD_S_306:
        case HPD_S_307:
        case HPD_S_400:
        case HPD_S_401:
        case HPD_S_402:
        case HPD_S_403:
        case HPD_S_404:
        case HPD_S_405:
        case HPD_S_406:
        case HPD_S_407:
        case HPD_S_408:
        case HPD_S_409:
        case HPD_S_410:
        case HPD_S_411:
        case HPD_S_412:
        case HPD_S_413:
        case HPD_S_414:
        case HPD_S_415:
        case HPD_S_416:
        case HPD_S_417:
        case HPD_S_422:
        case HPD_S_423:
        case HPD_S_424:
        case HPD_S_428:
        case HPD_S_429:
        case HPD_S_431:
        case HPD_S_500:
        case HPD_S_501:
        case HPD_S_502:
        case HPD_S_503:
        case HPD_S_504:
        case HPD_S_505:
        case HPD_S_507:
        case HPD_S_511:
            return request_alloc_response(response, request, status);
    }

    LOG_RETURN(hpd, HPD_E_ARGUMENT, "Unknown status code given to %s().", __func__);
}

hpd_error_t hpd_response_free(hpd_response_t *response)
{
    if (!response) return HPD_E_NULL;
    return request_free_response(response);
}

hpd_error_t hpd_response_set_value(hpd_response_t *response, hpd_value_t *value)
{
    if (!response) return HPD_E_NULL;
    return request_set_response_value(response, value);
}

hpd_error_t hpd_respond(hpd_response_t *response)
{
    if (!response) return HPD_E_NULL;
    hpd_t *hpd = response->request->service->device.adapter.context->hpd;
    if (!hpd->loop) LOG_RETURN_HPD_STOPPED(hpd);
    return request_respond(response);
}

hpd_error_t hpd_response_get_status(const hpd_response_t *response, hpd_status_t *status)
{
    if (!response) return HPD_E_NULL;
    hpd_t *hpd = response->request->service->device.adapter.context->hpd;
    if (!status) LOG_RETURN_E_NULL(hpd);
    return request_get_response_status(response, status);
}

hpd_error_t hpd_response_get_value(const hpd_response_t *response, const hpd_value_t **value)
{
    if (!response) return HPD_E_NULL;
    hpd_t *hpd = response->request->service->device.adapter.context->hpd;
    if (!value) LOG_RETURN_E_NULL(hpd);
    return request_get_response_value(response, value);
}

hpd_error_t hpd_response_get_request_data(const hpd_response_t *response, void **data)
{
    if (!response) return HPD_E_NULL;
    hpd_t *hpd = response->request->service->device.adapter.context->hpd;
    if (!data) LOG_RETURN_E_NULL(hpd);
    return request_get_response_request_data(response, data);
}

hpd_error_t hpd_response_get_request_service(const hpd_response_t *response, const hpd_service_id_t **service)
{
    if (!response) return HPD_E_NULL;
    hpd_t *hpd = response->request->service->device.adapter.context->hpd;
    if (!service) LOG_RETURN_E_NULL(hpd);
    return request_get_response_request_service(response, service);
}

hpd_error_t hpd_response_get_request_method(const hpd_response_t *response, hpd_method_t *method)
{
    if (!response) return HPD_E_NULL;
    hpd_t *hpd = response->request->service->device.adapter.context->hpd;
    if (!method) LOG_RETURN_E_NULL(hpd);
    return request_get_response_request_method(response, method);
}

hpd_error_t hpd_response_get_request_value(const hpd_response_t *response, const hpd_value_t **value)
{
    if (!response) return HPD_E_NULL;
    hpd_t *hpd = response->request->service->device.adapter.context->hpd;
    if (!value) LOG_RETURN_E_NULL(hpd);
    return request_get_response_request_value(response, value);
}

