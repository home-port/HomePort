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

#ifndef HOMEPORT_MODEL_H
#define HOMEPORT_MODEL_H

#include "hpd_api.h"
#include "map.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "hpd_internal_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum hpd_status hpd_status_t;

typedef struct adapters adapters_t;
typedef struct devices devices_t;
typedef struct services services_t;
typedef struct parameters parameters_t;
typedef struct listeners listeners_t;

TAILQ_HEAD(adapters, hpd_adapter);
TAILQ_HEAD(devices, hpd_device);
TAILQ_HEAD(services, hpd_service);
TAILQ_HEAD(parameters, hpd_parameter);
TAILQ_HEAD(listeners, hpd_listener);

struct hpd_action {
    hpd_method_t method;           //< Method
    hpd_action_f action;           //< Action
};

struct hpd_value {
    map_t       headers;
    char       *body;
    size_t      len;
};

struct hpd_request {
    hpd_service_t  *service;
    hpd_method_t    method;
    hpd_value_t    *value;
    // Callback and data for returning the response to sender
    hpd_response_f  on_response;
    hpd_free_f      on_free;
    void       *data;
};

struct hpd_response {
    hpd_request_t  *request;
    hpd_status_t    status;
    hpd_value_t    *value;
};

struct hpd_parameter
{
    hpd_service_t *service;
    TAILQ_ENTRY(hpd_parameter) HPD_TAILQ_FIELD;
    char *id;
    map_t *attributes;
};

struct hpd_service
{
    // Navigational members
    hpd_device_t *device;
    TAILQ_ENTRY(hpd_service) HPD_TAILQ_FIELD;
    parameters_t *parameters;
    listeners_t *listeners;
    // Data members
    char *id;
    map_t *attributes;
    hpd_action_t actions[HPD_M_COUNT];
    // User data
    hpd_free_f on_free;
    void *data;
};

struct hpd_device
{
    // Navigational members
    hpd_adapter_t *adapter;
    TAILQ_ENTRY(hpd_device) HPD_TAILQ_FIELD;
    services_t *services;
    listeners_t *listeners;
    // Data members
    char *id;
    map_t *attributes;
    // User data
    hpd_free_f on_free;
    void *data;
};

struct hpd_adapter
{
    // Navigational members
    configuration_t *configuration;
    TAILQ_ENTRY(hpd_adapter) HPD_TAILQ_FIELD;
    devices_t *devices;
    listeners_t *listeners;
    // Data members
    char *id;
    map_t *attributes;
    // User data
    hpd_free_f on_free;
    void *data;
};

struct configuration
{
    // Navigational members
    adapters_t  adapters;
    listeners_t listeners;
    void *data;
};

typedef enum { CONFIGURATION_LISTENER, ADAPTER_LISTENER, DEVICE_LISTENER, SERVICE_LISTENER } hpd_listener_type_t;

struct hpd_listener {
    hpd_listener_type_t type;
    // Navigational members
    TAILQ_ENTRY(hpd_listener) HPD_TAILQ_FIELD;
    union {
        hpd_service_t  *service;
        configuration_t *configuration;
        hpd_adapter_t *adapter;
        hpd_device_t *device;
    };
    // Data members
    hpd_value_f on_change;
    hpd_device_f on_attach;
    hpd_device_f on_detach;
    // User data
    void *data;
    hpd_free_f on_free;
};

#define OBJ_GET_CONF_DATA(OBJ, DATA) do {\
    (DATA) = (OBJ)->configuration->data; \
} while (0)

#define OBJ_GET_ADAPTER(OBJ, ADAPTER) do { \
    (ADAPTER) = (OBJ)->adapter; \
} while(0)

#define OBJ_GET_DEVICE(OBJ, DEVICE) do { \
    (DEVICE) = (OBJ)->device; \
} while(0)

#define OBJ_GET_METHOD(ACTION, METHOD) do { \
    (METHOD) = (ACTION)->method; \
} while(0)

#define OBJ_GET_SERVICE(OBJ, SERVICE) do { \
    (SERVICE) = (OBJ)->service; \
} while(0)

#define OBJ_GET_VALUE(OBJ, VALUE) do { \
    (VALUE) = (OBJ)->value; \
} while(0)

#define LISTENERS_REMOVE_IF_EXIST(LISTENERS, LISTENER) do { \
    hpd_listener_t *_iter; \
    HPD_TAILQ_FOREACH(_iter, (LISTENERS)) { \
        if (_iter == (LISTENER)) { \
            TAILQ_REMOVE((LISTENERS), (LISTENER), HPD_TAILQ_FIELD); \
            break; \
        } \
    } \
} while (0)

#define LISTENERS_INFORM(LISTENERS, FUNC, ...) do { \
    hpd_listener_t *listener; \
    HPD_TAILQ_FOREACH(listener, &(LISTENERS)) { \
        if (listener->FUNC) listener->FUNC(listener, ##__VA_ARGS__); \
    } \
} while(0)

#define ADAPTER_INFORM(ADAPTER, FUNC) do { \
    hpd_device_t *device; \
    HPD_TAILQ_FOREACH(device, &(ADAPTER)->devices) { \
        LISTENERS_INFORM(device->listeners, FUNC, device); \
        LISTENERS_INFORM((ADAPTER)->listeners, FUNC, device); \
        LISTENERS_INFORM((ADAPTER)->configuration->listeners, FUNC, device); \
    } \
} while(0)

#define ADAPTER_FIRST_DEVICE(ADAPTER, DEVICE) do { \
    (DEVICE) = TAILQ_FIRST(&(ADAPTER)->devices); \
} while(0)

#define ADAPTER_NEXT_DEVICE(DEVICE) do { \
    (DEVICE) = TAILQ_NEXT((DEVICE), HPD_TAILQ_FIELD); \
} while(0)

#define ADAPTER_FIRST_SERVICE(ADAPTER, SERVICE) do { \
    hpd_device_t *device; \
    ADAPTER_FIRST_DEVICE((ADAPTER), device); \
    if (device) DEVICE_FIRST_SERVICE(device, (SERVICE)); \
    else (SERVICE) = NULL; \
} while(0)

#define ADAPTER_NEXT_SERVICE(SERVICE) do { \
    if (TAILQ_NEXT((SERVICE), HPD_TAILQ_FIELD)) { \
        DEVICE_NEXT_SERVICE(SERVICE); \
    } else { \
        hpd_device_t *_device = TAILQ_NEXT((SERVICE)->device, HPD_TAILQ_FIELD); \
        if (_device) DEVICE_FIRST_SERVICE(_device, (SERVICE)); \
        else (SERVICE) = NULL; \
    } \
} while(0)



#define CONF_FIRST_ADAPTER(CONF, ADAPTER) do { \
    (ADAPTER) = TAILQ_FIRST(&(CONF)->adapters); \
} while(0)

#define CONF_NEXT_ADAPTER(ADAPTER) do { \
    (ADAPTER) = TAILQ_NEXT((ADAPTER), HPD_TAILQ_FIELD); \
} while(0)

#define CONF_FIRST_DEVICE(CONF, DEVICE) do { \
    hpd_adapter_t *adapter; \
    CONF_FIRST_ADAPTER((CONF), adapter); \
    if (adapter) ADAPTER_FIRST_DEVICE(adapter, (DEVICE)); \
    else (DEVICE) = NULL; \
} while(0)

#define CONF_NEXT_DEVICE(DEVICE) do { \
    if (TAILQ_NEXT((DEVICE), HPD_TAILQ_FIELD)) { \
        ADAPTER_NEXT_DEVICE(DEVICE); \
    } else { \
        hpd_adapter_t *_adapter = TAILQ_NEXT((DEVICE)->adapter, HPD_TAILQ_FIELD); \
        if (_adapter) ADAPTER_FIRST_DEVICE(_adapter, (DEVICE)); \
        else (DEVICE) = NULL; \
    } \
} while(0)

#define CONF_FIRST_SERVICE(CONF, SERVICE) do { \
    hpd_adapter_t *adapter; \
    CONF_FIRST_ADAPTER((CONF), adapter); \
    if (adapter) ADAPTER_FIRST_SERVICE(adapter, (SERVICE)); \
    else (SERVICE) = NULL; \
} while(0)

#define CONF_NEXT_SERVICE(SERVICE) do { \
    if (TAILQ_NEXT((SERVICE), HPD_TAILQ_FIELD)) { \
        DEVICE_NEXT_SERVICE(SERVICE); \
    } else if (TAILQ_NEXT((SERVICE)->device, HPD_TAILQ_FIELD)) { \
        ADAPTER_NEXT_SERVICE(SERVICE); \
    } else { \
        hpd_adapter_t *_adapter = TAILQ_NEXT((SERVICE)->device->adapter, HPD_TAILQ_FIELD); \
        if (_adapter) ADAPTER_FIRST_SERVICE(_adapter, (SERVICE)); \
        else (SERVICE) = NULL; \
    } \
} while(0)

#define CONF_FOREACH_ATTACHED(LISTENER) do { \
    hpd_adapter_t *_adapter; \
    hpd_device_t *_device; \
    if (LISTENER->on_attach) { \
        switch (LISTENER->type) { \
            case CONFIGURATION_LISTENER: \
                HPD_TAILQ_FOREACH(_adapter, &LISTENER->configuration->adapters) \
                HPD_TAILQ_FOREACH(_device, &_adapter->devices) \
                LISTENER->on_attach(LISTENER, _device); \
                break; \
            case ADAPTER_LISTENER: \
                HPD_TAILQ_FOREACH(_device, &LISTENER->adapter->devices) \
                LISTENER->on_attach(LISTENER, _device); \
                break; \
            case DEVICE_LISTENER: \
                LISTENER->on_attach(LISTENER, LISTENER->device); \
                break; \
            case SERVICE_LISTENER: \
                goto invalid; \
        } \
    } \
} while(0)

#define DEVICE_INFORM(DEVICE, FUNC) do { \
    LISTENERS_INFORM((DEVICE)->listeners, FUNC, (DEVICE)); \
    LISTENERS_INFORM((DEVICE)->adapter->listeners, FUNC, (DEVICE)); \
    LISTENERS_INFORM((DEVICE)->adapter->configuration->listeners, FUNC, (DEVICE)); \
} while(0)

#define DEVICE_ATTACH(ADAPTER, DEVICE) do { \
    TAILQ_INSERT_TAIL(&(ADAPTER)->devices, (DEVICE), HPD_TAILQ_FIELD); \
    (DEVICE)->adapter = (ADAPTER); \
    DEVICE_INFORM((DEVICE), on_attach); \
} while(0)

#define DEVICE_DETACH(DEVICE) do { \
    DEVICE_INFORM((DEVICE), on_detach); \
    TAILQ_REMOVE(&(DEVICE)->adapter->devices, (DEVICE), HPD_TAILQ_FIELD); \
    (DEVICE)->adapter = NULL; \
} while(0)

#define DEVICE_ATTACHED(DEVICE) ((DEVICE)->adapter)

#define DEVICE_FIRST_SERVICE(DEVICE, SERVICE) do { \
    (SERVICE) = TAILQ_FIRST(&(DEVICE)->services); \
} while(0)

#define DEVICE_NEXT_SERVICE(SERVICE) do { \
    (SERVICE) = TAILQ_NEXT((SERVICE), HPD_TAILQ_FIELD); \
} while(0)

#define LISTENER_ALLOC_ADAPTER(LISTENER, ADAPTER) do { \
    HPD_CALLOC((LISTENER), 1, hpd_listener_t); \
    (LISTENER)->type = ADAPTER_LISTENER; \
    (LISTENER)->adapter = ADAPTER; \
} while (0)

#define LISTENER_ALLOC_DEVICE(LISTENER, DEVICE) do { \
    HPD_CALLOC((LISTENER), 1, hpd_listener_t); \
    (LISTENER)->type = DEVICE_LISTENER; \
    (LISTENER)->device = DEVICE; \
} while (0)

#define LISTENER_ALLOC_CONF(LISTENER, CONF) do { \
    HPD_CALLOC((LISTENER), 1, hpd_listener_t); \
    (LISTENER)->type = CONFIGURATION_LISTENER; \
    (LISTENER)->configuration = CONF; \
} while (0)

#define LISTENER_ALLOC_SERVICE(LISTENER, SERVICE) do { \
    HPD_CALLOC((LISTENER), 1, hpd_listener_t); \
    (LISTENER)->type = SERVICE_LISTENER; \
    (LISTENER)->service = SERVICE; \
} while (0)

#define LISTENER_FREE(LISTENER) do { \
    switch ((LISTENER)->type) { \
        case CONFIGURATION_LISTENER: \
            LISTENERS_REMOVE_IF_EXIST(&(LISTENER)->configuration->listeners, (LISTENER)); \
            break; \
        case ADAPTER_LISTENER: \
            LISTENERS_REMOVE_IF_EXIST(&(LISTENER)->adapter->listeners, (LISTENER)); \
            break; \
        case DEVICE_LISTENER: \
            LISTENERS_REMOVE_IF_EXIST(&(LISTENER)->device->listeners, (LISTENER)); \
            break; \
        case SERVICE_LISTENER: \
            LISTENERS_REMOVE_IF_EXIST(&(LISTENER)->service->listeners, (LISTENER)); \
            break; \
    } \
    if ((LISTENER)->on_free) (LISTENER)->on_free((LISTENER)->data); \
    free((LISTENER)); \
} while (0)

#define LISTENER_SET_DEVICE_CALLBACK(LISTENER, ON_ATTACH, ON_DETACH) do { \
    (LISTENER)->on_attach = (ON_ATTACH); \
    (LISTENER)->on_detach = (ON_DETACH); \
} while (0)

#define LISTENER_SET_VALUE_CALLBACK(LISTENER, ON_CHANGE) do { \
    (LISTENER)->on_change = (ON_CHANGE); \
} while (0)

#define PARAMETER_ATTACH(SERVICE, PARAM) do { \
    TAILQ_INSERT_TAIL(&(SERVICE)->parameters, (PARAM), HPD_TAILQ_FIELD); \
    (PARAM)->service = (SERVICE); \
} while (0)

#define PARAMETER_DETACH(PARAM) do { \
    TAILQ_REMOVE(&(PARAM)->service->parameters, (PARAM), HPD_TAILQ_FIELD); \
    (PARAM)->service = NULL; \
} while(0)

#define PARAMETER_ATTACHED(PARAM) ((PARAM)->service)

#define REQUEST_ALLOC(REQUEST, SERVICE, METHOD, ON_RESPONSE) do { \
    HPD_CALLOC((REQUEST), 1, hpd_request_t); \
    (REQUEST)->service = (SERVICE); \
    (REQUEST)->method = (METHOD); \
    (REQUEST)->on_response = (ON_RESPONSE); \
} while (0)

#define REQUEST_FREE(REQUEST) do { \
    VALUE_FREE((REQUEST)->value); \
    if ((REQUEST)->on_free) (REQUEST)->on_free((REQUEST)->data); \
    free((REQUEST)); \
} while (0)

#define SERVICE_CHANGED(SERVICE, VAL) do { \
    LISTENERS_INFORM((SERVICE)->listeners, on_change, (SERVICE), (VAL)); \
    LISTENERS_INFORM((SERVICE)->device->listeners, on_change, (SERVICE), (VAL)); \
    LISTENERS_INFORM((SERVICE)->device->adapter->listeners, on_change, (SERVICE), (VAL)); \
    LISTENERS_INFORM((SERVICE)->device->adapter->configuration->listeners, on_change, (SERVICE), (VAL)); \
} while(0)

#define SERVICE_ATTACHED(SERVICE) ((SERVICE)->device)

#define VALUE_FREE(VALUE) do { \
    MAP_FREE(&(VALUE)->headers); \
    free((VALUE)->body); \
    free((VALUE)); \
} while (0)

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_MODEL_H
