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

#ifndef HOMEPORT_MODEL_H
#define HOMEPORT_MODEL_H

#include "hpd/hpd_types.h"
#include "hpd/common/hpd_queue.h"
#include "hpd/common/hpd_map.h"
#include "comm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hpd_configuration hpd_configuration_t;
typedef struct hpd_adapters hpd_adapters_t;
typedef struct hpd_devices hpd_devices_t;
typedef struct hpd_services hpd_services_t;
typedef struct hpd_parameters hpd_parameters_t;

TAILQ_HEAD(hpd_adapters, hpd_adapter);
TAILQ_HEAD(hpd_devices, hpd_device);
TAILQ_HEAD(hpd_services, hpd_service);
TAILQ_HEAD(hpd_parameters, hpd_parameter);

struct hpd_action {
    hpd_service_t *service;
    hpd_method_t method;           //< Method
    hpd_action_f action;           //< Action
};

struct hpd_configuration {
    // Navigational members
    hpd_adapters_t  adapters;
    hpd_listeners_t listeners;
    hpd_t *hpd;
};

struct hpd_adapter {
    const hpd_module_t *context;
    // Navigational members
    hpd_configuration_t *configuration;
    TAILQ_ENTRY(hpd_adapter) HPD_TAILQ_FIELD;
    hpd_devices_t *devices;
    // Data members
    char *id;
    hpd_map_t *attributes;
    // User data
    hpd_free_f on_free;
    void *data;
};

struct hpd_device {
    const hpd_module_t *context;
    // Navigational members
    hpd_adapter_t *adapter;
    TAILQ_ENTRY(hpd_device) HPD_TAILQ_FIELD;
    hpd_services_t *services;
    // Data members
    char *id;
    hpd_map_t *attributes;
    // User data
    hpd_free_f on_free;
    void *data;
};

struct hpd_service {
    const hpd_module_t *context;
    // Navigational members
    hpd_device_t *device;
    TAILQ_ENTRY(hpd_service) HPD_TAILQ_FIELD;
    hpd_parameters_t *parameters;
    // Data members
    char *id;
    hpd_map_t *attributes;
    hpd_action_t actions[HPD_M_COUNT];
    // User data
    hpd_free_f on_free;
    void *data;
};

struct hpd_parameter {
    const hpd_module_t *context;
    hpd_service_t *service;
    TAILQ_ENTRY(hpd_parameter) HPD_TAILQ_FIELD;
    char *id;
    hpd_map_t *attributes;
};

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_MODEL_H
