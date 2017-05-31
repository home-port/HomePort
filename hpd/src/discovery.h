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

#ifndef HOMEPORT_DISCOVERY_H
#define HOMEPORT_DISCOVERY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hpd/hpd_types.h"
#include <stdarg.h>

typedef struct hpd_adapter_id {
    hpd_t *hpd;
    char *aid;
} hpd_adapter_id_t;

typedef struct hpd_device_id {
    hpd_adapter_id_t adapter;
    char *did;
} hpd_device_id_t;

typedef struct hpd_service_id {
    hpd_device_id_t device;
    char *sid;
} hpd_service_id_t;

typedef struct hpd_parameter_id {
    hpd_service_id_t service;
    char *pid;
} hpd_parameter_id_t;

hpd_error_t discovery_alloc_aid(hpd_adapter_id_t **id, hpd_t *hpd, const char *aid);
hpd_error_t discovery_alloc_did(hpd_device_id_t **id, hpd_t *hpd, const char *aid, const char *did);
hpd_error_t discovery_alloc_sid(hpd_service_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid);
hpd_error_t discovery_alloc_pid(hpd_parameter_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid, const char *pid);

hpd_error_t discovery_copy_aid(hpd_adapter_id_t **dst, const hpd_adapter_id_t *src);
hpd_error_t discovery_copy_did(hpd_device_id_t **dst, const hpd_device_id_t *src);
hpd_error_t discovery_copy_sid(hpd_service_id_t **dst, const hpd_service_id_t *src);
hpd_error_t discovery_copy_pid(hpd_parameter_id_t **dst, const hpd_parameter_id_t *src);

hpd_error_t discovery_free_aid(hpd_adapter_id_t *id);
hpd_error_t discovery_free_did(hpd_device_id_t *id);
hpd_error_t discovery_free_sid(hpd_service_id_t *id);
hpd_error_t discovery_free_pid(hpd_parameter_id_t *id);

hpd_error_t discovery_set_aid(hpd_adapter_id_t *id, hpd_t *hpd, const char *aid);
hpd_error_t discovery_set_did(hpd_device_id_t *id, hpd_t *hpd, const char *aid, const char *did);
hpd_error_t discovery_set_sid(hpd_service_id_t *id, hpd_t *hpd, const char *aid, const char *did, const char *sid);
hpd_error_t discovery_set_pid(hpd_parameter_id_t *id, hpd_t *hpd, const char *aid, const char *did, const char *sid, const char *pid);

hpd_error_t discovery_find_adapter(const hpd_adapter_id_t *id, hpd_adapter_t **adapter);
hpd_error_t discovery_find_device(const hpd_device_id_t *id, hpd_device_t **device);
hpd_error_t discovery_find_service(const hpd_service_id_t *id, hpd_service_t **service);
hpd_error_t discovery_find_parameter(const hpd_parameter_id_t *id, hpd_parameter_t **parameter);

hpd_error_t discovery_get_aid_hpd(const hpd_adapter_id_t *aid, hpd_t **hpd);
hpd_error_t discovery_get_did_hpd(const hpd_device_id_t *did, hpd_t **hpd);
hpd_error_t discovery_get_did_adapter(const hpd_device_id_t *did, const hpd_adapter_id_t **aid);
hpd_error_t discovery_get_sid_hpd(const hpd_service_id_t *sid, hpd_t **hpd);
hpd_error_t discovery_get_sid_adapter(const hpd_service_id_t *sid, const hpd_adapter_id_t **aid);
hpd_error_t discovery_get_sid_device(const hpd_service_id_t *sid, const hpd_device_id_t **did);
hpd_error_t discovery_get_pid_hpd(const hpd_parameter_id_t *pid, hpd_t **hpd);
hpd_error_t discovery_get_pid_adapter(const hpd_parameter_id_t *pid, const hpd_adapter_id_t **aid);
hpd_error_t discovery_get_pid_device(const hpd_parameter_id_t *pid, const hpd_device_id_t **did);
hpd_error_t discovery_get_pid_service(const hpd_parameter_id_t *pid, const hpd_service_id_t **sid);

hpd_error_t discovery_get_aid_aid(const hpd_adapter_id_t *adapter, const char **id);
hpd_error_t discovery_get_did_did(const hpd_device_id_t *device, const char **id);
hpd_error_t discovery_get_did_aid(const hpd_device_id_t *device, const char **id);
hpd_error_t discovery_get_sid_sid(const hpd_service_id_t *service, const char **id);
hpd_error_t discovery_get_sid_aid(const hpd_service_id_t *service, const char **id);
hpd_error_t discovery_get_sid_did(const hpd_service_id_t *service, const char **id);
hpd_error_t discovery_get_pid_pid(const hpd_parameter_id_t *parameter, const char **id);
hpd_error_t discovery_get_pid_aid(const hpd_parameter_id_t *parameter, const char **id);
hpd_error_t discovery_get_pid_did(const hpd_parameter_id_t *parameter, const char **id);
hpd_error_t discovery_get_pid_sid(const hpd_parameter_id_t *parameter, const char **id);

hpd_error_t discovery_alloc_adapter(hpd_adapter_t **adapter, const char *id);
hpd_error_t discovery_alloc_device(hpd_device_t **device, const char *id);
hpd_error_t discovery_alloc_service(hpd_service_t **service, const char *id);
hpd_error_t discovery_alloc_parameter(hpd_parameter_t **parameter, const char *id);

hpd_error_t discovery_free_adapter(hpd_adapter_t *adapter);
hpd_error_t discovery_free_device(hpd_device_t *device);
hpd_error_t discovery_free_service(hpd_service_t *service);
hpd_error_t discovery_free_parameter(hpd_parameter_t *parameter);

hpd_error_t discovery_attach_adapter(hpd_t *hpd, hpd_adapter_t *adapter);
hpd_error_t discovery_attach_device(hpd_adapter_t *adapter, hpd_device_t *device);
hpd_error_t discovery_attach_service(hpd_device_t *device, hpd_service_t *service);
hpd_error_t discovery_attach_parameter(hpd_service_t *service, hpd_parameter_t *parameter);

hpd_error_t discovery_detach_adapter(hpd_adapter_t *adapter);
hpd_error_t discovery_detach_device(hpd_device_t *device);
hpd_error_t discovery_detach_service(hpd_service_t *service);
hpd_error_t discovery_detach_parameter(hpd_parameter_t *parameter);

hpd_error_t discovery_get_adapter_hpd(const hpd_adapter_t *adapter, hpd_t **hpd);
hpd_error_t discovery_get_adapter_device(const hpd_adapter_t *adapter, const char *id, hpd_device_t **device);
hpd_error_t discovery_get_device_hpd(const hpd_device_t *device, hpd_t **hpd);
hpd_error_t discovery_get_device_adapter(const hpd_device_t *device, const hpd_adapter_t **adapter);
hpd_error_t discovery_get_device_service(const hpd_device_t *device, const char *id, hpd_service_t **service);
hpd_error_t discovery_get_service_hpd(const hpd_service_t *service, hpd_t **hpd);
hpd_error_t discovery_get_service_adapter(const hpd_service_t *service, const hpd_adapter_t **adapter);
hpd_error_t discovery_get_service_device(const hpd_service_t *service, const hpd_device_t **device);
hpd_error_t discovery_get_service_parameter(const hpd_service_t *service, const char *id, hpd_parameter_t **parameter);
hpd_error_t discovery_get_parameter_hpd(const hpd_parameter_t *parameter, hpd_t **hpd);
hpd_error_t discovery_get_parameter_adapter(const hpd_parameter_t *parameter, const hpd_adapter_t **adapter);
hpd_error_t discovery_get_parameter_device(const hpd_parameter_t *parameter, const hpd_device_t **device);
hpd_error_t discovery_get_parameter_service(const hpd_parameter_t *parameter, const hpd_service_t **service);

hpd_error_t discovery_get_adapter_id(const hpd_adapter_t *adapter, const char **id);
hpd_error_t discovery_get_adapter_data(const hpd_adapter_t *adapter, void **data);
hpd_error_t discovery_get_adapter_attr(const hpd_adapter_t *adapter, const char *key, const char **val);
hpd_error_t discovery_get_adapter_attrs_v(const hpd_adapter_t *adapter, va_list vp);
hpd_error_t discovery_get_device_adapter_id(const hpd_device_t *device, const char **id);
hpd_error_t discovery_get_device_id(const hpd_device_t *device, const char **id);
hpd_error_t discovery_get_device_data(const hpd_device_t *device, void **data);
hpd_error_t discovery_get_device_attr(const hpd_device_t *device, const char *key, const char **val);
hpd_error_t discovery_get_device_attrs_v(const hpd_device_t *device, va_list vp);
hpd_error_t discovery_get_service_adapter_id(const hpd_service_t *service, const char **id);
hpd_error_t discovery_get_service_device_id(const hpd_service_t *service, const char **id);
hpd_error_t discovery_get_service_id(const hpd_service_t *service, const char **id);
hpd_error_t discovery_get_service_data(const hpd_service_t *service, void **data);
hpd_error_t discovery_get_service_attr(const hpd_service_t *service, const char *key, const char **val);
hpd_error_t discovery_get_service_attrs_v(const hpd_service_t *service, va_list vp);
hpd_error_t discovery_get_parameter_adapter_id(const hpd_parameter_t *parameter, const char **id);
hpd_error_t discovery_get_parameter_device_id(const hpd_parameter_t *parameter, const char **id);
hpd_error_t discovery_get_parameter_service_id(const hpd_parameter_t *parameter, const char **id);
hpd_error_t discovery_get_parameter_id(const hpd_parameter_t *parameter, const char **id);
hpd_error_t discovery_get_parameter_attr(const hpd_parameter_t *parameter, const char *key, const char **val);
hpd_error_t discovery_get_parameter_attrs_v(const hpd_parameter_t *parameter, va_list vp);
hpd_error_t discovery_get_action_method(const hpd_action_t *action, hpd_method_t *method);
hpd_error_t discovery_get_action_action(const hpd_action_t *action, hpd_action_f *cb);

hpd_error_t discovery_set_adapter_data(hpd_adapter_t *adapter, void *data, hpd_free_f on_free);
hpd_error_t discovery_set_adapter_attr(hpd_adapter_t *adapter, const char *key, const char *val);
hpd_error_t discovery_set_adapter_attrs_v(hpd_adapter_t *adapter, va_list vp);
hpd_error_t discovery_set_device_data(hpd_device_t *device, void *data, hpd_free_f on_free);
hpd_error_t discovery_set_device_attr(hpd_device_t *device, const char *key, const char *val);
hpd_error_t discovery_set_device_attrs_v(hpd_device_t *device, va_list vp);
hpd_error_t discovery_set_service_data(hpd_service_t *service, void *data, hpd_free_f on_free);
hpd_error_t discovery_set_service_attr(hpd_service_t *service, const char *key, const char *val);
hpd_error_t discovery_set_service_attrs_v(hpd_service_t *service, va_list vp);
hpd_error_t discovery_set_service_action(hpd_service_t *service, const hpd_method_t method, hpd_action_f action);
hpd_error_t discovery_set_service_actions_v(hpd_service_t *service, va_list vp);
hpd_error_t discovery_set_parameter_attr(hpd_parameter_t *parameter, const char *key, const char *val);
hpd_error_t discovery_set_parameter_attrs_v(hpd_parameter_t *parameter, va_list vp);

hpd_error_t discovery_first_action_in_service(const hpd_service_t *service, const hpd_action_t **action);
hpd_error_t discovery_first_adapter_attr(const hpd_adapter_t *adapter, const hpd_pair_t **pair);
hpd_error_t discovery_first_device_attr(const hpd_device_t *device, const hpd_pair_t **pair);
hpd_error_t discovery_first_service_attr(const hpd_service_t *service, const hpd_pair_t **pair);
hpd_error_t discovery_first_parameter_attr(const hpd_parameter_t *parameter, const hpd_pair_t **pair);
hpd_error_t discovery_first_hpd_adapter(hpd_t *hpd, hpd_adapter_t **adapter);
hpd_error_t discovery_first_hpd_device(hpd_t *hpd, hpd_device_t **device);
hpd_error_t discovery_first_hpd_service(hpd_t *hpd, hpd_service_t **service);
hpd_error_t discovery_first_adapter_device(const hpd_adapter_t *adapter, hpd_device_t **device);
hpd_error_t discovery_first_adapter_service(const hpd_adapter_t *adapter, hpd_service_t **service);
hpd_error_t discovery_first_device_service(const hpd_device_t *device, hpd_service_t **service);
hpd_error_t discovery_first_service_parameter(const hpd_service_t *service, hpd_parameter_t **parameter);

hpd_error_t discovery_next_action_in_service(const hpd_action_t **action);
hpd_error_t discovery_next_adapter_attr(const hpd_pair_t **pair);
hpd_error_t discovery_next_device_attr(const hpd_pair_t **pair);
hpd_error_t discovery_next_service_attr(const hpd_pair_t **pair);
hpd_error_t discovery_next_parameter_attr(const hpd_pair_t **pair);
hpd_error_t discovery_next_hpd_adapter(hpd_adapter_t **adapter);
hpd_error_t discovery_next_hpd_device(hpd_device_t **device);
hpd_error_t discovery_next_hpd_service(hpd_service_t **service);
hpd_error_t discovery_next_adapter_device(hpd_device_t **device);
hpd_error_t discovery_next_adapter_service(hpd_service_t **service);
hpd_error_t discovery_next_device_service(hpd_service_t **service);
hpd_error_t discovery_next_service_parameter(hpd_parameter_t **parameter);

hpd_bool_t discovery_is_adapter_id_unique(hpd_t *hpd, hpd_adapter_t *adapter);
hpd_bool_t discovery_is_device_id_unique(hpd_adapter_t *adapter, hpd_device_t *device);
hpd_bool_t discovery_is_service_id_unique(hpd_device_t *device, hpd_service_t *service);
hpd_bool_t discovery_is_parameter_id_unique(hpd_service_t *service, hpd_parameter_t *parameter);

hpd_bool_t discovery_has_service_action(const hpd_service_t *service, const hpd_method_t method);

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_DISCOVERY_H
