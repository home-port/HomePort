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

#include <stdio.h>
#include <stdarg.h>
#include <ev.h>
#include "hpd_api.h"
#include "old_model.h"
#include "daemon.h"

#ifdef NOT

/**
 * alloc_error can mean that HPD is only partially allocated
 */

hpd_error_t hpd_action_get_method(hpd_action_t *action, hpd_method_t *method)
{
    if (!action || !method) return HPD_E_NULL;
    (*method) = (action)->method;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_adapter_find_device(hpd_adapter_t *adapter, hpd_device_t **device, ...)
{
    if (!adapter || !device) return HPD_E_NULL;

    va_list vp;
    va_start(vp, device);

    ADAPTER_FIRST_DEVICE(adapter, *device);
    while (*device) {
        MAP_MATCHES_VA(&(*device)->attributes, vp);
        mismatch: ADAPTER_NEXT_DEVICE(*device);
    }

    match:
        va_end(vp);
        return HPD_E_SUCCESS;

    null_error:
        va_end(vp);
        return HPD_E_NULL;
}

hpd_error_t hpd_adapter_find_next_device(hpd_device_t **device, ...)
{
    if (!device || !(*device)) return HPD_E_NULL;

    va_list vp;
    va_start(vp, device);

    ADAPTER_NEXT_DEVICE(*device);
    while (*device) {
        MAP_MATCHES_VA(&(*device)->attributes, vp);
        mismatch: ADAPTER_NEXT_DEVICE(*device);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_adapter_find_next_service(hpd_service_t **service, ...)
{
    if (!service || !(*service)) return HPD_E_NULL;

    va_list vp;
    va_start(vp, service);

    ADAPTER_NEXT_SERVICE(*service);
    while (*service) {
        MAP_MATCHES_VA(&(*service)->attributes, vp);
        mismatch: ADAPTER_NEXT_SERVICE(*service);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_adapter_find_service(hpd_adapter_t *adapter, hpd_service_t **service, ...)
{
    if (!adapter || !service) return HPD_E_NULL;

    va_list vp;
    va_start(vp, service);

    ADAPTER_FIRST_SERVICE(adapter, *service);
    while (*service) {
        MAP_MATCHES_VA(&(*service)->attributes, vp);
        mismatch: ADAPTER_NEXT_SERVICE(*service);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_adapter_first_device(hpd_adapter_t *adapter, hpd_device_t **device)
{
    if (!adapter || !device) return HPD_E_NULL;

    ADAPTER_FIRST_DEVICE(adapter, *device);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_adapter_first_service(hpd_adapter_t *adapter, hpd_service_t **service)
{
    if (!adapter || !service) return HPD_E_NULL;

    ADAPTER_FIRST_SERVICE(adapter, *service);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_adapter_get_attr(hpd_adapter_t *adapter, const char *key, const char **val)
{
    if (!adapter || !key || !val) return HPD_E_NULL;

    MAP_GET(&adapter->attributes, key, *val);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_adapter_get_attrs(hpd_adapter_t *adapter, ...)
{
    if (!adapter) return HPD_E_NULL;

    va_list vp;
    const char *key, **val;
    va_start(vp, adapter);

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char **);
        if (!val) goto null_error;
        MAP_GET((&adapter->attributes), key, *val);
    }

    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
        va_end(vp);
        return HPD_E_NULL;
}

hpd_error_t hpd_adapter_get_hpd(hpd_adapter_t *adapter, hpd_t **hpd) {
    if (!adapter || !hpd) return HPD_E_NULL;

    OBJ_GET_CONF_DATA(adapter, *hpd);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_adapter_next_device(hpd_device_t **device)
{
    if (!device || !(*device)) return HPD_E_NULL;

    ADAPTER_NEXT_DEVICE(*device);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_adapter_next_service(hpd_service_t **service)
{
    if (!service || !(*service)) return HPD_E_NULL;

    ADAPTER_NEXT_SERVICE(*service);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_changed(hpd_service_t *service, hpd_value_t *val)
{
    if (!service || !val) return HPD_E_NULL;

    SERVICE_CHANGED(service, val);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_device_attach(hpd_adapter_t *adapter, hpd_device_t *device)
{
    if (!adapter || !device) return HPD_E_NULL;
    if (DEVICE_ATTACHED(device)) return HPD_E_ATTACHED;
    // TODO Check ID Uniqueness
    DEVICE_ATTACH(adapter, device);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_device_detach(hpd_device_t *device) {
    if (!device) return HPD_E_NULL;
    if (!DEVICE_ATTACHED(device)) return HPD_E_DETACHED;

    DEVICE_DETACH(device);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_device_find_service(hpd_device_t *device, hpd_service_t **service, ...)
{
    if (!device || !service) return HPD_E_NULL;

    va_list vp;
    va_start(vp, service);

    DEVICE_FIRST_SERVICE(device, *service);
    while (*service) {
        MAP_MATCHES_VA(&(*service)->attributes, vp);
        mismatch: DEVICE_NEXT_SERVICE(*service);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_device_find_next_service(hpd_service_t **service, ...)
{
    if (!service || !(*service)) return HPD_E_NULL;

    va_list vp;
    va_start(vp, service);

    DEVICE_NEXT_SERVICE(*service);
    while (*service) {
        MAP_MATCHES_VA(&(*service)->attributes, vp);
        mismatch: DEVICE_NEXT_SERVICE(*service);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_device_first_service(hpd_device_t *device, hpd_service_t **service)
{
    if (!device || !service) return HPD_E_NULL;

    DEVICE_FIRST_SERVICE(device, *service);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_device_get_adapter(hpd_device_t *device, hpd_adapter_t **adapter)
{
    if (!device || !adapter) return HPD_E_NULL;

    OBJ_GET_ADAPTER(device, *adapter);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_device_get_attr(hpd_device_t *device, const char *key, const char **val)
{
    if (!device || !key || !val) return HPD_E_NULL;

    MAP_GET(&device->attributes, key, *val);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_device_get_attrs(hpd_device_t *device, ...)
{
    if (!device) return HPD_E_NULL;

    va_list vp;
    const char *key, **val;
    va_start(vp, device);

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char **);
        if (!val) goto null_error;
        MAP_GET((&device->attributes), key, *val);
    }

    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_device_get_data(const hpd_device_t *device, void **data)
{
    if (!device || !data) return HPD_E_NULL;

    OBJ_GET_DATA(device, *data);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_device_next_service(hpd_service_t **service)
{
    if (!service || !(*service)) return HPD_E_NULL;

    DEVICE_NEXT_SERVICE(*service);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_device_set_attr(hpd_device_t *device, const char *key, const char *val)
{
    if (!device || !key) return HPD_E_NULL;

    MAP_SET(&device->attributes, key, val);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t hpd_device_set_attrs(hpd_device_t *device, ...)
{
    if (!device) return HPD_E_NULL;

    va_list vp;
    const char *key, *val;
    va_start(vp, device);

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char *);
        if (!val) goto null_error;
        MAP_SET((&device->attributes), key, val);
    }

    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;

    alloc_error:
    va_end(vp);
    return HPD_E_ALLOC;
}

hpd_error_t hpd_device_set_data(hpd_device_t *device, void *data, hpd_free_f on_free)
{
    if (!device) return HPD_E_NULL;

    OBJ_SET_DATA(device, data, on_free);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_find_adapter(hpd_t *hpd, hpd_adapter_t **adapter, ...)
{
    if (!hpd || !adapter) return HPD_E_NULL;

    va_list vp;
    va_start(vp, adapter);

    CONF_FIRST_ADAPTER(hpd->configuration, *adapter);
    while (*adapter) {
        MAP_MATCHES_VA(&(*adapter)->attributes, vp);
        mismatch: CONF_NEXT_ADAPTER(*adapter);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_find_device(hpd_t *hpd, hpd_device_t **device, ...)
{
    if (!hpd || !device) return HPD_E_NULL;

    va_list vp;
    va_start(vp, device);

    CONF_FIRST_DEVICE(hpd->configuration, *device);
    while (*device) {
        MAP_MATCHES_VA(&(*device)->attributes, vp);
        mismatch: CONF_NEXT_DEVICE(*device);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_find_next_adapter(hpd_adapter_t **adapter, ...)
{
    if (!adapter || !(*adapter)) return HPD_E_NULL;

    va_list vp;
    va_start(vp, adapter);

    CONF_NEXT_ADAPTER(*adapter);
    while (*adapter) {
        MAP_MATCHES_VA(&(*adapter)->attributes, vp);
        mismatch: CONF_NEXT_ADAPTER(*adapter);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_find_next_device(hpd_device_t **device, ...)
{
    if (!device || !(*device)) return HPD_E_NULL;

    va_list vp;
    va_start(vp, device);

    CONF_NEXT_DEVICE(*device);
    while (*device) {
        MAP_MATCHES_VA(&(*device)->attributes, vp);
        mismatch: CONF_NEXT_DEVICE(*device);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_find_next_service(hpd_service_t **service, ...)
{
    if (!service || !(*service)) return HPD_E_NULL;

    va_list vp;
    va_start(vp, service);

    CONF_NEXT_SERVICE(*service);
    while (*service) {
        MAP_MATCHES_VA(&(*service)->attributes, vp);
        mismatch: CONF_NEXT_SERVICE(*service);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_find_service(hpd_t *hpd, hpd_service_t **service, ...)
{
    if (!hpd || !service) return HPD_E_NULL;

    va_list vp;
    va_start(vp, service);

    CONF_FIRST_SERVICE(hpd->configuration, *service);
    while (*service) {
        MAP_MATCHES_VA(&(*service)->attributes, vp);
        mismatch: CONF_NEXT_SERVICE(*service);
    }

    match:
    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_first_adapter(hpd_t *hpd, hpd_adapter_t **adapter)
{
    if (!hpd || !adapter) return HPD_E_NULL;

    CONF_FIRST_ADAPTER(hpd->configuration, *adapter);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_first_device(hpd_t *hpd, hpd_device_t **device)
{
    if (!hpd || !device) return HPD_E_NULL;

    CONF_FIRST_DEVICE(hpd->configuration, *device);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_first_service(hpd_t *hpd, hpd_service_t **service)
{
    if (!hpd || !service) return HPD_E_NULL;

    CONF_FIRST_SERVICE(hpd->configuration, *service);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_foreach_attached(hpd_listener_t *listener)
{
    if (!listener) return HPD_E_NULL;
    switch (listener->type) {
        case CONFIGURATION_LISTENER:
            if (!listener->configuration) return HPD_E_UNSUBSCRIBED;
            break;
        case ADAPTER_LISTENER:
            if (!listener->adapter) return HPD_E_UNSUBSCRIBED;
            if (!ADAPTER_ATTACHED(listener->adapter)) return HPD_E_DETACHED;
            break;
        case DEVICE_LISTENER:
            if (!listener->device) return HPD_E_UNSUBSCRIBED;
            if (!DEVICE_ATTACHED(listener->device)) return HPD_E_DETACHED;
            break;
        case SERVICE_LISTENER:
            return HPD_E_ARGUMENT;
    }

    CONF_FOREACH_ATTACHED(listener);
    return HPD_E_SUCCESS;

    invalid:
    return HPD_E_ARGUMENT;
}

hpd_error_t hpd_listener_alloc_adapter(hpd_listener_t **listener, hpd_adapter_t *adapter)
{
    if (!listener || !adapter) return HPD_E_NULL;

    LISTENER_ALLOC_ADAPTER(*listener, adapter);
    return HPD_E_SUCCESS;

    alloc_error:
        return HPD_E_ALLOC;
}

hpd_error_t hpd_listener_alloc_device(hpd_listener_t **listener, hpd_device_t *device)
{
    if (!listener || !device) return HPD_E_NULL;

    LISTENER_ALLOC_DEVICE(*listener, device);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t hpd_listener_alloc_hpd(hpd_listener_t **listener, hpd_t *hpd)
{
    if (!listener || !hpd) return HPD_E_NULL;

    LISTENER_ALLOC_CONF(*listener, hpd->configuration);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t hpd_listener_alloc_service(hpd_listener_t **listener, hpd_service_t *service)
{
    if (!listener || !service) return HPD_E_NULL;

    LISTENER_ALLOC_SERVICE(*listener, service);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t hpd_listener_free(hpd_listener_t *listener)
{
    if (!listener) return HPD_E_NULL;

    LISTENER_FREE(listener);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_listener_get_data(hpd_listener_t *listener, void **data)
{
    if (!listener || !data) return HPD_E_NULL;

    OBJ_GET_DATA(listener, *data);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_listener_set_data(hpd_listener_t *listener, void *data, hpd_free_f on_free) {
    if (!listener) return HPD_E_NULL;

    OBJ_SET_DATA(listener, data, on_free);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_listener_set_device_callback(hpd_listener_t *listener, hpd_device_f on_attach, hpd_device_f on_detach)
{
    if (!listener) return HPD_E_NULL;

    LISTENER_SET_DEVICE_CALLBACK(listener, on_attach, on_detach);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_listener_set_value_callback(hpd_listener_t *listener, hpd_value_f on_change)
{
    if (!listener) return HPD_E_NULL;

    LISTENER_SET_VALUE_CALLBACK(listener, on_change);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_next_adapter(hpd_adapter_t **adapter) {
    if (!adapter || !(*adapter)) return HPD_E_NULL;

    CONF_NEXT_ADAPTER(*adapter);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_next_device(hpd_device_t **device)
{
    if (!device || !(*device)) return HPD_E_NULL;

    CONF_NEXT_DEVICE(*device);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_next_service(hpd_service_t **service)
{
    if (!service || !(*service)) return HPD_E_NULL;

    CONF_NEXT_SERVICE(*service);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_pair_get(hpd_pair_t *pair, const char **key, const char **value)
{
    if (!pair) return HPD_E_NULL;

    PAIR_GET(pair, *key, *value);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_parameter_attach(hpd_service_t *service, hpd_parameter_t *parameter)
{
    if (!service || !parameter) return HPD_E_NULL;
    if (PARAMETER_ATTACHED(parameter)) return HPD_E_ATTACHED;
    // TODO Check ID Uniqueness
    PARAMETER_ATTACH(service, parameter);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_parameter_detach(hpd_parameter_t *parameter)
{
    if (!parameter) return HPD_E_NULL;
    if (!PARAMETER_ATTACHED(parameter)) return HPD_E_DETACHED;

    PARAMETER_DETACH(parameter);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_parameter_free(hpd_parameter_t *parameter)
{
    if (!parameter) return HPD_E_NULL;

    PARAMETER_FREE(parameter);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_parameter_get_attr(hpd_parameter_t *parameter, const char *key, const char **val) {
    if (!parameter || !key || !val) return HPD_E_NULL;

    MAP_GET(&parameter->attributes, key, *val);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_parameter_get_attrs(hpd_parameter_t *parameter, ...)
{
    if (!parameter) return HPD_E_NULL;

    va_list vp;
    const char *key, **val;
    va_start(vp, parameter);

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char **);
        if (!val) goto null_error;
        MAP_GET((&parameter->attributes), key, *val);
    }

    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;
}

hpd_error_t hpd_parameter_set_attr(hpd_parameter_t *parameter, const char *key, const char *val)
{
    if (!parameter || !key) return HPD_E_NULL;

    MAP_SET(&parameter->attributes, key, val);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t hpd_parameter_set_attrs(hpd_parameter_t *parameter, ...)
{
    if (!parameter) return HPD_E_NULL;

    va_list vp;
    const char *key, *val;
    va_start(vp, parameter);

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char *);
        if (!val) goto null_error;
        MAP_SET((&parameter->attributes), key, val);
    }

    va_end(vp);
    return HPD_E_SUCCESS;

    null_error:
    va_end(vp);
    return HPD_E_NULL;

    alloc_error:
    va_end(vp);
    return HPD_E_ALLOC;
}

hpd_error_t hpd_request_alloc(hpd_request_t **request, hpd_service_t *service, hpd_method_t method, hpd_response_f on_response)
{
    if (!request || !service) return HPD_E_NULL;
    switch (method) {
        case HPD_M_NONE:
        case HPD_M_COUNT:
            return HPD_E_ARGUMENT;
        case HPD_M_GET:
        case HPD_M_PUT:
            break;
    }

    REQUEST_ALLOC(*request, service, method, on_response);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t hpd_request_free(hpd_request_t *request)
{
    if (!request) return HPD_E_NULL;

    REQUEST_FREE(request);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_request_get_method(const hpd_request_t *req, hpd_method_t *method)
{
    if (!req || !method) return HPD_E_NULL;

    OBJ_GET_METHOD(req, *method);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_request_get_service(const hpd_request_t *req, hpd_service_t **service)
{
    if (!req || !service) return HPD_E_NULL;

    OBJ_GET_SERVICE(req, *service);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_request_get_value(const hpd_request_t *req, hpd_value_t **value)
{
    if (!req || !value) return HPD_E_NULL;

    OBJ_GET_VALUE(req, *value);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_request(hpd_request_t *request)
{
    if (!request) return HPD_E_NULL;

    hpd_request_t *REQUEST = request;

    hpd_action_f _act = (REQUEST)->service->actions[(REQUEST)->method].action;
    if (_act) {
        _act((REQUEST));
    } else {
        // TODO THIS !!!
        hpd_response_t *RESPONSE;
        HPD_CALLOC((RESPONSE), 1, hpd_response_t); \
        (RESPONSE)->request = (REQUEST); \
        (RESPONSE)->status = HPD_S_405; \
        (RESPONSE)->value = NULL; \

        REQUEST_FREE(request);
    }

    return HPD_E_SUCCESS;

    alloc_error:
    REQUEST_FREE(request);
    return HPD_E_ALLOC;
}

hpd_error_t hpd_request_set_data(hpd_request_t *request, void *data, hpd_free_f on_free) { return HPD_E_SUCCESS; }

hpd_error_t hpd_request_set_value(hpd_request_t *request, hpd_value_t *value) { return HPD_E_SUCCESS; }

hpd_error_t hpd_respond(hpd_response_t *response) { return HPD_E_SUCCESS; }

hpd_error_t hpd_response_alloc(hpd_response_t **response, hpd_request_t *request, hpd_status_t status) { return HPD_E_SUCCESS; }

hpd_error_t hpd_response_free(hpd_response_t *response) { return HPD_E_SUCCESS; }

hpd_error_t hpd_response_get_request_data(const hpd_response_t *response, void **data) { return HPD_E_SUCCESS; }

hpd_error_t hpd_response_get_request_method(const hpd_response_t *response, hpd_method_t *method) { return HPD_E_SUCCESS; }

hpd_error_t hpd_response_get_request_service(const hpd_response_t *response, hpd_service_t **service) { return HPD_E_SUCCESS; }

hpd_error_t hpd_response_get_request_value(const hpd_response_t *response, hpd_value_t **value) { return HPD_E_SUCCESS; }

hpd_error_t hpd_response_get_status(hpd_response_t *response, hpd_value_t *value) { return HPD_E_SUCCESS; }

hpd_error_t hpd_response_get_value(hpd_response_t *response, hpd_value_t *value) { return HPD_E_SUCCESS; }

hpd_error_t hpd_response_set_value(hpd_response_t *response, hpd_value_t *value) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_attach(hpd_device_t *device, hpd_service_t *service) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_detach(hpd_service_t *service) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_find_parameter(hpd_service_t *service, hpd_parameter_t **parameter, ...) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_find_next_parameter(hpd_parameter_t **parameter, ...) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_first_action(hpd_service_t *service, hpd_action_t **action) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_first_parameter(hpd_service_t *service, hpd_parameter_t **parameter) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_free(hpd_service_t *service) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_get_attr(hpd_service_t *service, const char *key, const char **val) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_get_attrs(hpd_service_t *service, ...) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_get_data(const hpd_service_t *service, void **data) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_get_device(hpd_service_t *service, hpd_device_t **device) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_has_action(hpd_service_t *service, const hpd_method_t method, char *boolean) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_next_action(hpd_action_t **action) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_next_parameter(hpd_parameter_t **parameter) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_set_action(hpd_service_t *service, const hpd_method_t method, hpd_action_f action) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_set_actions(hpd_service_t *service, ...) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_set_attr(hpd_service_t *service, const char *key, const char *val) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_set_attrs(hpd_service_t *service, ...) { return HPD_E_SUCCESS; }

hpd_error_t hpd_service_set_data(hpd_service_t *service, void *data, hpd_free_f on_free) { return HPD_E_SUCCESS; }

hpd_error_t hpd_subscribe(hpd_listener_t *listener) { return HPD_E_SUCCESS; }

hpd_error_t hpd_unsubscribe(hpd_listener_t *listener) { return HPD_E_SUCCESS; }

hpd_error_t hpd_value_alloc(hpd_value_t **value, const char *body, size_t len) { return HPD_E_SUCCESS; }

hpd_error_t hpd_value_first_header(hpd_value_t *value, hpd_pair_t **pair) { return HPD_E_SUCCESS; }

hpd_error_t hpd_value_free(hpd_value_t *value) { return HPD_E_SUCCESS; }

hpd_error_t hpd_value_get_body(hpd_value_t **value, const char **body, size_t *len) { return HPD_E_SUCCESS; }

hpd_error_t hpd_value_get_headers(hpd_value_t *value, ...) { return HPD_E_SUCCESS; }

hpd_error_t hpd_value_get_header(hpd_value_t *value, const char *key, const char **val) { return HPD_E_SUCCESS; }

hpd_error_t hpd_value_next_header(hpd_pair_t **pair) { return HPD_E_SUCCESS; }

hpd_error_t hpd_value_set_headers(hpd_value_t *value, ...) { return HPD_E_SUCCESS; }

hpd_error_t hpd_value_set_header(hpd_value_t *value, const char *key, const char *val) { return HPD_E_SUCCESS; }

#endif
