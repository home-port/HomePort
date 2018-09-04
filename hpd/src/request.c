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

#include "value.h"
#include "discovery.h"
#include "request.h"
#include "hpd-0.6/common/hpd_common.h"
#include <ev.h>
#include "daemon.h"
#include "log.h"
#include "comm.h"
#include "model.h"

hpd_error_t request_alloc_request(hpd_request_t **request, const hpd_service_id_t *id, hpd_method_t method,
                                  hpd_response_f on_response)
{
    hpd_error_t rc = HPD_E_ALLOC;
    HPD_CALLOC(*request, 1, hpd_request_t);
    if ((rc = discovery_copy_sid(&(*request)->service, id))) goto alloc_error;
    (*request)->method = method;
    (*request)->on_response = on_response;
    return HPD_E_SUCCESS;

    alloc_error:
        request_free_request(*request);
    switch (rc) {
        case HPD_E_ALLOC:
            LOG_RETURN_E_ALLOC(id->device.adapter.context->hpd);
        default:
            return rc;
    }
}

hpd_error_t request_free_request(hpd_request_t *request)
{
    if (request) {
        if (request->on_free) request->on_free(request->data);
        if (request->service) discovery_free_sid(request->service);
        if (request->value) value_free(request->value);
        free(request);
    }
    return HPD_E_SUCCESS;
}

hpd_error_t request_set_request_value(hpd_request_t *request, hpd_value_t *value)
{
    if (value) {
        HPD_CPY_ALLOC(request->value, value, hpd_value_t);
        free(value);
    } else {
        value_free(request->value);
        request->value = NULL;
    }
    return HPD_E_SUCCESS;

    alloc_error:
        LOG_RETURN_E_ALLOC(request->service->device.adapter.context->hpd);
}

hpd_error_t request_set_request_data(hpd_request_t *request, void *data, hpd_free_f on_free)
{
    request->data = data;
    request->on_free = on_free;
    return HPD_E_SUCCESS;
}

hpd_error_t request_get_request_service(const hpd_request_t *req, const hpd_service_id_t **id)
{
    (*id) = req->service;
    return HPD_E_SUCCESS;
}

hpd_error_t request_get_request_method(const hpd_request_t *req, hpd_method_t *method)
{
    (*method) = req->method;
    return HPD_E_SUCCESS;
}

hpd_error_t request_get_request_value(const hpd_request_t *req, const hpd_value_t **value)
{
    (*value) = req->value;
    return HPD_E_SUCCESS;
}

hpd_error_t request_alloc_response(hpd_response_t **response, hpd_request_t *request, hpd_status_t status)
{
    HPD_CALLOC(*response, 1, hpd_response_t);
    HPD_CPY_ALLOC((*response)->request, request, hpd_request_t);
    free(request);
    (*response)->status = status;
    return HPD_E_SUCCESS;

    alloc_error:
    request_free_response(*response);
        LOG_RETURN_E_ALLOC(request->service->device.adapter.context->hpd);
}

hpd_error_t request_free_response(hpd_response_t *response)
{
    if (response) {
        if (response->request) request_free_request(response->request);
        if (response->value) value_free(response->value);
        free(response);
    }
    return HPD_E_SUCCESS;
}

hpd_error_t request_set_response_value(hpd_response_t *response, hpd_value_t *value)
{
    if (value) {
        HPD_CPY_ALLOC(response->value, value, hpd_value_t);
        free(value);
    } else {
        value_free(response->value);
        response->value = NULL;
    }
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC(response->request->service->device.adapter.context->hpd);
}

hpd_error_t request_get_response_status(const hpd_response_t *response, hpd_status_t *status)
{
    (*status) = response->status;
    return HPD_E_SUCCESS;
}

hpd_error_t request_get_response_value(const hpd_response_t *response, const hpd_value_t **value)
{
    (*value) = response->value;
    return HPD_E_SUCCESS;
}

hpd_error_t request_get_response_request_data(const hpd_response_t *response, void **data)
{
    (*data) = response->request->data;
    return HPD_E_SUCCESS;
}

hpd_error_t request_get_response_request_service(const hpd_response_t *response, const hpd_service_id_t **service)
{
    (*service) = response->request->service;
    return HPD_E_SUCCESS;
}

hpd_error_t request_get_response_request_method(const hpd_response_t *response, hpd_method_t *method)
{
    (*method) = response->request->method;
    return HPD_E_SUCCESS;
}

hpd_error_t request_get_response_request_value(const hpd_response_t *response, const hpd_value_t **value)
{
    (*value) = response->request->value;
    return HPD_E_SUCCESS;
}

static void request_on_request(hpd_ev_loop_t *loop, ev_async *w, int revents)
{
    hpd_error_t rc;
    hpd_ev_async_t *async = w->data;
    hpd_request_t *request = async->request;
    hpd_service_id_t *service_id = request->service;

    hpd_t *hpd = service_id->device.adapter.context->hpd;
    char *sid = service_id->sid;
    char *did = service_id->device.did;
    char *aid = service_id->device.adapter.aid;
    
    TAILQ_REMOVE(&hpd->request_watchers, async, HPD_TAILQ_FIELD);
    ev_async_stop(loop, w);
    free(async);

    hpd_service_t *service;
    hpd_response_t *response;
    
    switch (discovery_find_service(service_id, &service)) {
        case HPD_E_NOT_FOUND: {
            LOG_DEBUG(hpd, "Did not find service %s/%s/%s.", aid, did, sid);
            if ((rc = request_alloc_response(&response, request, HPD_S_404))) goto error_free_request;
            if ((rc = request_respond(response))) goto error_free_response;
            return;
        }
    }

    hpd_action_f action = service->actions[request->method].action;
    if (!action) {
        LOG_DEBUG(hpd, "Action (%s) not supported by service %s/%s/%s.",
                request->method == HPD_M_GET ? "GET" : request->method == HPD_M_PUT ? "PUT" : "UNKNOWN",
                aid, did, sid);
        if ((rc = request_alloc_response(&response, request, HPD_S_405))) goto error_free_request;
        if ((rc = request_respond(response))) goto error_free_response;
        return;
    }

    hpd_status_t status;
    if ((status = action(service->data, request)) != HPD_S_NONE) {
        if ((rc = request_alloc_response(&response, request, status))) goto error_free_request;
        if ((rc = request_respond(response))) goto error_free_response;
        return;
    }

    return;

    error_free_response:
        request_free_response(response);
    error_free_request:
        request_free_request(request);
        LOG_ERROR(hpd, "on_request() failed [code: %i].", rc);
        return;
}

static void request_on_respond(hpd_ev_loop_t *loop, ev_async *w, int revents)
{
    hpd_error_t rc;
    hpd_ev_async_t *async = w->data;
    hpd_response_t *response = async->response;
    hpd_request_t *request = response->request;
    hpd_service_id_t *service_id = request->service;
    hpd_t *hpd = service_id->device.adapter.context->hpd;

    TAILQ_REMOVE(&hpd->respond_watchers, async, HPD_TAILQ_FIELD);
    ev_async_stop(loop, w);
    free(async);

    if (request->on_response) request->on_response(request->data, response);

    if ((rc = request_free_response(response))) {
        LOG_ERROR(hpd, "Free function failed [code: %i].", rc);
    }
}

hpd_error_t request_request(hpd_request_t *request)
{
    hpd_ev_async_t *async;
    hpd_t *hpd = request->service->device.adapter.context->hpd;
    HPD_CALLOC(async, 1, hpd_ev_async_t);
    HPD_CPY_ALLOC(async->request, request, hpd_request_t);
    ev_async_init(&async->watcher, request_on_request);
    async->watcher.data = async;
    ev_async_start(hpd->loop, &async->watcher);
    ev_async_send(hpd->loop, &async->watcher);
    TAILQ_INSERT_TAIL(&hpd->request_watchers, async, HPD_TAILQ_FIELD);
    free(request);
    return HPD_E_SUCCESS;

    alloc_error:
    free(async);
    LOG_RETURN_E_ALLOC(hpd);
}

hpd_error_t request_respond(hpd_response_t *response)
{
    hpd_ev_async_t *async;
    hpd_t *hpd = response->request->service->device.adapter.context->hpd;
    HPD_CALLOC(async, 1, hpd_ev_async_t);
    HPD_CPY_ALLOC(async->response, response, hpd_response_t);
    ev_async_init(&async->watcher, request_on_respond);
    async->watcher.data = async;
    ev_async_start(hpd->loop, &async->watcher);
    ev_async_send(hpd->loop, &async->watcher);
    TAILQ_INSERT_TAIL(&hpd->respond_watchers, async, HPD_TAILQ_FIELD);
    free(response);
    return HPD_E_SUCCESS;

    alloc_error:
    free(async);
    LOG_RETURN_E_ALLOC(hpd);
}
