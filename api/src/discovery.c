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

#include "event.h"
#include "model.h"
#include "discovery.h"
#include "daemon.h"
#include "hpd_common.h"

hpd_error_t discovery_alloc_adapter(hpd_adapter_t **adapter, const char *id)
{
    HPD_CALLOC((*adapter), 1, hpd_adapter_t);
    HPD_CALLOC((*adapter)->devices, 1, devices_t);
    TAILQ_INIT((*adapter)->devices);
    HPD_CALLOC((*adapter)->attributes, 1, map_t);
    MAP_INIT((*adapter)->attributes);
    HPD_STR_CPY((*adapter)->id, id);
    return HPD_E_SUCCESS;

    alloc_error:
    if (*adapter) discovery_free_adapter(*adapter);
    (*adapter) = NULL;
    return HPD_E_ALLOC;
}

hpd_error_t discovery_alloc_device(hpd_device_t **device, const char *id)
{
    HPD_CALLOC((*device), 1, hpd_device_t);
    HPD_CALLOC((*device)->services, 1, services_t);
    TAILQ_INIT((*device)->services);
    HPD_CALLOC((*device)->attributes, 1, map_t);
    MAP_INIT((*device)->attributes);
    HPD_STR_CPY((*device)->id, id);
    return HPD_E_SUCCESS;

    alloc_error:
    if (*device) discovery_free_device(*device);
    (*device) = NULL;
    return HPD_E_ALLOC;
}

hpd_error_t discovery_alloc_service(hpd_service_t **service, const char *id)
{
    HPD_CALLOC((*service), 1, hpd_service_t);
    HPD_CALLOC((*service)->parameters, 1, parameters_t);
    TAILQ_INIT((*service)->parameters);
    HPD_CALLOC((*service)->attributes, 1, map_t);
    MAP_INIT((*service)->attributes);
    HPD_STR_CPY((*service)->id, id);
    return HPD_E_SUCCESS;

    alloc_error:
    if (*service) discovery_free_service(*service);
    (*service) = NULL;
    return HPD_E_ALLOC;
}

hpd_error_t discovery_alloc_parameter(hpd_parameter_t **parameter, const char *id)
{
    HPD_CALLOC((*parameter), 1, hpd_parameter_t);
    HPD_CALLOC((*parameter)->attributes, 1, map_t);
    MAP_INIT((*parameter)->attributes);
    HPD_STR_CPY((*parameter)->id, id);
    return HPD_E_SUCCESS;

    alloc_error:
    if (*parameter) discovery_free_parameter(*parameter);
    (*parameter) = NULL;
    return HPD_E_ALLOC;
}

hpd_error_t discovery_free_adapter(hpd_adapter_t *adapter)
{
    hpd_error_t rc;
    if (adapter->on_free) adapter->on_free(adapter->data);
    if (adapter->devices) {
        HPD_TAILQ_MAP_REMOVE(adapter->devices, discovery_free_device, hpd_device_t, rc);
        free(adapter->devices);
    }
    if (adapter->attributes) {
        MAP_FREE(adapter->attributes);
        free(adapter->attributes);
    }
    free(adapter->id);
    free(adapter);
    return HPD_E_SUCCESS;

    map_error:
        // TODO This is bad ...
        return rc;
}

hpd_error_t discovery_free_device(hpd_device_t *device)
{
    hpd_error_t rc;
    if (device->on_free) device->on_free(device->data);
    if (device->services) {
        HPD_TAILQ_MAP_REMOVE(device->services, discovery_free_service, hpd_service_t, rc);
        free(device->services);
    }
    if (device->attributes) {
        MAP_FREE(device->attributes);
        free(device->attributes);
    }
    free(device->id);
    free(device);
    return HPD_E_SUCCESS;

    map_error:
        // TODO This is bad ...
        return rc;
}

hpd_error_t discovery_free_service(hpd_service_t *service)
{
    hpd_error_t rc;
    if (service->on_free) service->on_free(service->data);
    if (service->parameters) {
        HPD_TAILQ_MAP_REMOVE(service->parameters, discovery_free_parameter, hpd_parameter_t, rc);
        free(service->parameters);
    }
    if (service->attributes) {
        MAP_FREE(service->attributes);
        free(service->attributes);
    }
    free(service->id);
    free(service);
    return HPD_E_SUCCESS;

    map_error:
        // TODO This is bad ...
        return rc;
}


hpd_error_t discovery_free_parameter(hpd_parameter_t *parameter)
{
    if (parameter->attributes) {
        MAP_FREE(parameter->attributes);
        free(parameter->attributes);
    }
    free(parameter->id);
    free(parameter);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_attach_adapter(hpd_t *hpd, hpd_adapter_t *adapter)
{
    hpd_error_t rc;
    hpd_adapter_t *copy;
    HPD_CPY_ALLOC(copy, adapter, hpd_adapter_t);

    TAILQ_INSERT_TAIL(&hpd->configuration->adapters, copy, HPD_TAILQ_FIELD);
    copy->configuration = hpd->configuration;

    if ((rc = event_inform_adapter_attached(copy))) {
        TAILQ_REMOVE(&hpd->configuration->adapters, copy, HPD_TAILQ_FIELD);
        free(copy);
        return rc;
    }

    free(adapter);
    return HPD_E_SUCCESS;

    alloc_error:
        return HPD_E_ALLOC;
}

hpd_error_t discovery_attach_device(hpd_adapter_t *adapter, hpd_device_t *device)
{
    hpd_error_t rc;
    hpd_device_t *copy;
    HPD_CPY_ALLOC(copy, device, hpd_device_t);

    TAILQ_INSERT_TAIL(adapter->devices, copy, HPD_TAILQ_FIELD);
    copy->adapter = adapter;

    if ((rc = event_inform_device_attached(copy))) {
        TAILQ_REMOVE(adapter->devices, copy, HPD_TAILQ_FIELD);
        free(copy);
        return rc;
    }

    free(device);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t discovery_attach_service(hpd_device_t *device, hpd_service_t *service)
{
    hpd_service_t *copy;
    HPD_CPY_ALLOC(copy, service, hpd_service_t);

    TAILQ_INSERT_TAIL(device->services, copy, HPD_TAILQ_FIELD);
    copy->device = device;

    free(service);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t discovery_attach_parameter(hpd_service_t *service, hpd_parameter_t *parameter)
{
    hpd_parameter_t *copy;
    HPD_CPY_ALLOC(copy, parameter, hpd_parameter_t);

    TAILQ_INSERT_TAIL(service->parameters, copy, HPD_TAILQ_FIELD);
    copy->service = service;

    free(parameter);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t discovery_detach_adapter(hpd_adapter_t *adapter)
{
    hpd_error_t rc;

    // Inform event listeners
    if ((rc = event_inform_adapter_detached(adapter))) return rc;

    // Detach it
    TAILQ_REMOVE(&adapter->configuration->adapters, adapter, HPD_TAILQ_FIELD);
    adapter->configuration = NULL;

    return HPD_E_SUCCESS;
}

hpd_error_t discovery_detach_device(hpd_device_t *device)
{
    hpd_error_t rc;

    // Inform event listeners
    if ((rc = event_inform_device_detached(device))) return rc;

    // Detach it
    TAILQ_REMOVE(device->adapter->devices, device, HPD_TAILQ_FIELD);
    device->adapter = NULL;

    return HPD_E_SUCCESS;
}

hpd_error_t discovery_detach_service(hpd_service_t *service)
{
    TAILQ_REMOVE(service->device->services, service, HPD_TAILQ_FIELD);
    service->device = NULL;

    return HPD_E_SUCCESS;
}

hpd_error_t discovery_detach_parameter(hpd_parameter_t *parameter)
{
    TAILQ_REMOVE(parameter->service->parameters, parameter, HPD_TAILQ_FIELD);
    parameter->service = NULL;

    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_adapter_data(hpd_adapter_t *adapter, void **data)
{
    (*data) = adapter->data;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_device_data(hpd_device_t *device, void **data)
{
    (*data) = device->data;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_service_data(hpd_service_t *service, void **data)
{
    (*data) = service->data;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_adapter_id(hpd_adapter_t *adapter, const char **id)
{
    (*id) = adapter->id;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_device_id(hpd_device_t *device, const char **id)
{
    (*id) = device->id;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_service_id(hpd_service_t *service, const char **id)
{
    (*id) = service->id;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_parameter_id(hpd_parameter_t *parameter, const char **id)
{
    (*id) = parameter->id;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_adapter_attr(hpd_adapter_t *adapter, const char *key, const char **val)
{
    MAP_GET(adapter->attributes, key, *val);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_device_attr(hpd_device_t *device, const char *key, const char **val)
{
    MAP_GET(device->attributes, key, *val);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_service_attr(hpd_service_t *service, const char *key, const char **val)
{
    MAP_GET(service->attributes, key, *val);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_parameter_attr(hpd_parameter_t *parameter, const char *key, const char **val)
{
    MAP_GET(parameter->attributes, key, *val);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_adapter_attrs_v(hpd_adapter_t *adapter, va_list vp)
{
    hpd_error_t rc;
    const char *key, **val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char **);
        if (!val) return HPD_E_NULL;
        if ((rc = discovery_get_adapter_attr(adapter, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_device_attrs_v(hpd_device_t *device, va_list vp)
{
    hpd_error_t rc;
    const char *key, **val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char **);
        if (!val) return HPD_E_NULL;
        if ((rc = discovery_get_device_attr(device, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_service_attrs_v(hpd_service_t *service, va_list vp)
{
    hpd_error_t rc;
    const char *key, **val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char **);
        if (!val) return HPD_E_NULL;
        if ((rc = discovery_get_service_attr(service, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_parameter_attrs_v(hpd_parameter_t *parameter, va_list vp)
{
    hpd_error_t rc;
    const char *key, **val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char **);
        if (!val) return HPD_E_NULL;
        if ((rc = discovery_get_parameter_attr(parameter, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_action_method(hpd_action_t *action, hpd_method_t *method)
{
    (*method) = action->method;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_set_adapter_data(hpd_adapter_t *adapter, void *data, hpd_free_f on_free)
{
    hpd_error_t rc;
    if (adapter->on_free && (rc = adapter->on_free(adapter->data))) return rc;
    adapter->data = data;
    adapter->on_free = on_free;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_set_device_data(hpd_device_t *device, void *data, hpd_free_f on_free)
{
    hpd_error_t rc;
    if (device->on_free && (rc = device->on_free(device->data))) return rc;
    device->data = data;
    device->on_free = on_free;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_set_service_data(hpd_service_t *service, void *data, hpd_free_f on_free)
{
    hpd_error_t rc;
    if (service->on_free && (rc = service->on_free(service->data))) return rc;
    service->data = data;
    service->on_free = on_free;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_set_adapter_attr(hpd_adapter_t *adapter, const char *key, const char *val)
{
    MAP_SET(adapter->attributes, key, val);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t discovery_set_device_attr(hpd_device_t *device, const char *key, const char *val)
{
    MAP_SET(device->attributes, key, val);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t discovery_set_service_attr(hpd_service_t *service, const char *key, const char *val)
{
    MAP_SET(service->attributes, key, val);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t discovery_set_parameter_attr(hpd_parameter_t *parameter, const char *key, const char *val)
{
    MAP_SET(parameter->attributes, key, val);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t discovery_set_service_action(hpd_service_t *service, const hpd_method_t method, hpd_action_f action)
{
    hpd_action_t *action_p = &service->actions[method];
    action_p->service = service;
    action_p->method = method;
    action_p->action = action;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_set_adapter_attrs_v(hpd_adapter_t *adapter, va_list vp)
{
    hpd_error_t rc;
    const char *key, *val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char *);
        if (!val) return HPD_E_NULL;
        if ((rc = discovery_set_adapter_attr(adapter, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_set_device_attrs_v(hpd_device_t *device, va_list vp)
{
    hpd_error_t rc;
    const char *key, *val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char *);
        if (!val) return HPD_E_NULL;
        if ((rc = discovery_set_device_attr(device, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_set_service_attrs_v(hpd_service_t *service, va_list vp)
{
    hpd_error_t rc;
    const char *key, *val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char *);
        if (!val) return HPD_E_NULL;
        if ((rc = discovery_set_service_attr(service, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_set_parameter_attrs_v(hpd_parameter_t *parameter, va_list vp)
{
    hpd_error_t rc;
    const char *key, *val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char *);
        if (!val) return HPD_E_NULL;
        if ((rc = discovery_set_parameter_attr(parameter, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_set_service_actions_v(hpd_service_t *service, va_list vp)
{
    hpd_error_t rc;
    hpd_method_t method;
    hpd_action_f action;

    while ((method = va_arg(vp, hpd_method_t)) != HPD_M_NONE) {
        if (method <= HPD_M_NONE || method >= HPD_M_COUNT) return HPD_E_ARGUMENT;
        action = va_arg(vp, hpd_action_f);
        if (!action) return HPD_E_NULL;
        if ((rc = discovery_set_service_action(service, method, action))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_action_in_service(hpd_service_t *service, hpd_action_t **action)
{
    (*action) = &service->actions[HPD_M_NONE+1];
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_adapter_attr(hpd_adapter_t *adapter, hpd_pair_t **pair)
{
    (*pair) = TAILQ_FIRST(adapter->attributes);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_device_attr(hpd_device_t *device, hpd_pair_t **pair)
{
    (*pair) = TAILQ_FIRST(device->attributes);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_service_attr(hpd_service_t *service, hpd_pair_t **pair)
{
    (*pair) = TAILQ_FIRST(service->attributes);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_parameter_attr(hpd_parameter_t *parameter, hpd_pair_t **pair)
{
    (*pair) = TAILQ_FIRST(parameter->attributes);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_hpd_adapter(hpd_t *hpd, hpd_adapter_t **adapter)
{
    (*adapter) = TAILQ_FIRST(&hpd->configuration->adapters);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_hpd_device(hpd_t *hpd, hpd_device_t **device)
{
    hpd_adapter_t *adapter;
    HPD_TAILQ_FOREACH(adapter, &hpd->configuration->adapters) {
        *device = TAILQ_FIRST(adapter->devices);
        if (*device) return HPD_E_SUCCESS;
    }

    *device = NULL;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_hpd_service(hpd_t *hpd, hpd_service_t **service)
{
    hpd_adapter_t *adapter;
    HPD_TAILQ_FOREACH(adapter, &hpd->configuration->adapters) {
        hpd_device_t *device;
        HPD_TAILQ_FOREACH(device, adapter->devices) {
            *service = TAILQ_FIRST(device->services);
            if (*service) return HPD_E_SUCCESS;
        }
    }

    *service = NULL;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_adapter_device(hpd_adapter_t *adapter, hpd_device_t **device)
{
    (*device) = TAILQ_FIRST(adapter->devices);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_adapter_service(hpd_adapter_t *adapter, hpd_service_t **service)
{
    hpd_device_t *device;
    HPD_TAILQ_FOREACH(device, adapter->devices) {
        *service = TAILQ_FIRST(device->services);
        if (*service) return HPD_E_SUCCESS;
    }

    *service = NULL;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_device_service(const hpd_device_t *device, hpd_service_t **service)
{
    (*service) = TAILQ_FIRST(device->services);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_first_service_parameter(hpd_service_t *service, hpd_parameter_t **parameter)
{
    (*parameter) = TAILQ_FIRST(service->parameters);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_action_in_service(hpd_action_t **action)
{
    hpd_service_t *service = (*action)->service;
    hpd_method_t method = (*action)->method + 1;
    if (method < HPD_M_COUNT) (*action) = &service->actions[method];
    else (*action) = NULL;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_adapter_attr(hpd_pair_t **pair)
{
    *pair = TAILQ_NEXT(*pair, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_device_attr(hpd_pair_t **pair)
{
    *pair = TAILQ_NEXT(*pair, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_service_attr(hpd_pair_t **pair)
{
    *pair = TAILQ_NEXT(*pair, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_parameter_attr(hpd_pair_t **pair)
{
    *pair = TAILQ_NEXT(*pair, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_hpd_adapter(hpd_adapter_t **adapter)
{
    *adapter = TAILQ_NEXT(*adapter, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_hpd_device(hpd_device_t **device)
{
    hpd_device_t *d;
    d = TAILQ_NEXT(*device, HPD_TAILQ_FIELD);
    if (d) {
        *device = d;
    } else {
        hpd_adapter_t *adapter = (*device)->adapter;
        while ((adapter = TAILQ_NEXT(adapter, HPD_TAILQ_FIELD))) {
            *device = TAILQ_FIRST(adapter->devices);
            if (*device) return HPD_E_SUCCESS;
        }
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_hpd_service(hpd_service_t **service)
{
    hpd_error_t rc;
    hpd_service_t *s;
    s = TAILQ_NEXT(*service, HPD_TAILQ_FIELD);

    if (s) {
        *service = s;
    } else {
        hpd_device_t *device = (*service)->device;
        while ((device = TAILQ_NEXT(device, HPD_TAILQ_FIELD))) {
            s = TAILQ_FIRST(device->services);
            if (s) {
                *service = s;
                return HPD_E_SUCCESS;
            }
        }
        hpd_adapter_t *adapter = (*service)->device->adapter;
        while ((adapter = TAILQ_NEXT(adapter, HPD_TAILQ_FIELD))) {
            if ((rc = discovery_first_adapter_service(adapter, &s))) return rc;
            if (s) {
                *service = s;
                return HPD_E_SUCCESS;
            }
        }
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_adapter_device(hpd_device_t **device)
{
    *device = TAILQ_NEXT(*device, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_adapter_service(hpd_service_t **service)
{
    hpd_service_t *s;
    s = TAILQ_NEXT(*service, HPD_TAILQ_FIELD);
    if (s) {
        *service = s;
    } else {
        hpd_device_t *device = (*service)->device;
        while ((device = TAILQ_NEXT(device, HPD_TAILQ_FIELD))) {
            *service = TAILQ_FIRST(device->services);
            if (*service) return HPD_E_SUCCESS;
        }
    }
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_device_service(hpd_service_t **service)
{
    *service = TAILQ_NEXT(*service, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_next_service_parameter(hpd_parameter_t **parameter)
{
    *parameter = TAILQ_NEXT(*parameter, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}


hpd_bool_t discovery_has_service_action(hpd_service_t *service, const hpd_method_t method)
{
    return (service->actions[method].action != NULL);
}

hpd_bool_t discovery_is_adapter_id_unique(hpd_t *hpd, hpd_adapter_t *adapter)
{
    hpd_adapter_t *a;
    HPD_TAILQ_FOREACH(a, &hpd->configuration->adapters)
        if (strcmp(a->id, adapter->id) == 0) return HPD_FALSE;
    return HPD_TRUE;
}

hpd_bool_t discovery_is_device_id_unique(hpd_adapter_t *adapter, hpd_device_t *device)
{
    hpd_device_t *d;
    HPD_TAILQ_FOREACH(d, adapter->devices)
        if (strcmp(d->id, device->id) == 0) return HPD_FALSE;
    return HPD_TRUE;
}

hpd_bool_t discovery_is_service_id_unique(hpd_device_t *device, hpd_service_t *service)
{
    hpd_service_t *s;
    HPD_TAILQ_FOREACH(s, device->services)
        if (strcmp(s->id, service->id) == 0) return HPD_FALSE;
    return HPD_TRUE;
}

hpd_bool_t discovery_is_parameter_id_unique(hpd_service_t *service, hpd_parameter_t *parameter)
{
    hpd_parameter_t *p;
    HPD_TAILQ_FOREACH(p, service->parameters)
        if (strcmp(p->id, parameter->id) == 0) return HPD_FALSE;
    return HPD_TRUE;
}
