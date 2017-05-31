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

#ifndef HOMEPORT_HPD_ADAPTER_API_H
#define HOMEPORT_HPD_ADAPTER_API_H

#include <hpd/hpd_shared_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/// [hpd_adapter_t functions]
hpd_error_t hpd_adapter_alloc(hpd_adapter_t **adapter, const char *id);
hpd_error_t hpd_adapter_free(hpd_adapter_t *adapter);
hpd_error_t hpd_adapter_attach(hpd_t *hpd, hpd_adapter_t *adapter);
hpd_error_t hpd_adapter_detach(hpd_adapter_t *adapter);
hpd_error_t hpd_adapter_id_detach(const hpd_adapter_id_t *id, hpd_adapter_t **adapter);
hpd_error_t hpd_adapter_set_attr(hpd_adapter_t *adapter, const char *key, const char *val);
hpd_error_t hpd_adapter_set_attrs(hpd_adapter_t *adapter, ...);
hpd_error_t hpd_adapter_set_data(hpd_adapter_t *adapter, void *data, hpd_free_f on_free);
hpd_error_t hpd_adapter_get_data(const hpd_adapter_t *adapter, void **data);
hpd_error_t hpd_adapter_get_adapter_id_str(const hpd_adapter_t *adapter, const char **id);
hpd_error_t hpd_adapter_get_attr(const hpd_adapter_t *adapter, const char *key, const char **val);
hpd_error_t hpd_adapter_get_attrs(const hpd_adapter_t *adapter, ...);
hpd_error_t hpd_adapter_first_attr(const hpd_adapter_t *adapter, const hpd_pair_t **pair);
hpd_error_t hpd_adapter_next_attr(const hpd_pair_t **pair);
/// [hpd_adapter_t functions]

/// [hpd_device_t functions]
hpd_error_t hpd_device_alloc(hpd_device_t **device, const char *id);
hpd_error_t hpd_device_free(hpd_device_t *device);
hpd_error_t hpd_device_attach(hpd_adapter_t *adapter, hpd_device_t *device);
hpd_error_t hpd_device_id_attach(const hpd_adapter_id_t *id, hpd_device_t *device);
hpd_error_t hpd_device_detach(hpd_device_t *device);
hpd_error_t hpd_device_id_detach(const hpd_device_id_t *id, hpd_device_t **device);
hpd_error_t hpd_device_set_data(hpd_device_t *device, void *data, hpd_free_f on_free);
hpd_error_t hpd_device_set_attr(hpd_device_t *device, const char *key, const char *val);
hpd_error_t hpd_device_set_attrs(hpd_device_t *device, ...);
hpd_error_t hpd_device_get_data(const hpd_device_t *device, void **data);
hpd_error_t hpd_device_get_adapter_id_str(const hpd_device_t *device, const char **id);
hpd_error_t hpd_device_get_device_id_str(const hpd_device_t *device, const char **id);
hpd_error_t hpd_device_get_attr(const hpd_device_t *device, const char *key, const char **val);
hpd_error_t hpd_device_get_attrs(const hpd_device_t *device, ...);
hpd_error_t hpd_device_first_attr(const hpd_device_t *device, const hpd_pair_t **pair);
hpd_error_t hpd_device_next_attr(const hpd_pair_t **pair);
/// [hpd_device_t functions]

/// [hpd_service_t functions]
hpd_error_t hpd_service_alloc(hpd_service_t **service, const char *id);
hpd_error_t hpd_service_free(hpd_service_t *service);
hpd_error_t hpd_service_attach(hpd_device_t *device, hpd_service_t *service);
hpd_error_t hpd_service_id_attach(const hpd_device_id_t *id, hpd_service_t *service);
hpd_error_t hpd_service_detach(hpd_service_t *service);
hpd_error_t hpd_service_id_detach(const hpd_service_id_t *id, hpd_service_t **service);
hpd_error_t hpd_service_set_data(hpd_service_t *service, void *data, hpd_free_f on_free);
hpd_error_t hpd_service_set_attr(hpd_service_t *service, const char *key, const char *val);
hpd_error_t hpd_service_set_attrs(hpd_service_t *service, ...);
hpd_error_t hpd_service_set_action(hpd_service_t *service, const hpd_method_t method, hpd_action_f action);
hpd_error_t hpd_service_set_actions(hpd_service_t *service, ...);
hpd_error_t hpd_service_get_data(const hpd_service_t *service, void **data);
hpd_error_t hpd_service_get_adapter_id_str(const hpd_service_t *service, const char **id);
hpd_error_t hpd_service_get_device_id_str(const hpd_service_t *service, const char **id);
hpd_error_t hpd_service_get_service_id_str(const hpd_service_t *service, const char **id);
hpd_error_t hpd_service_get_attr(const hpd_service_t *service, const char *key, const char **val);
hpd_error_t hpd_service_get_attrs(const hpd_service_t *service, ...);
hpd_error_t hpd_service_has_action(const hpd_service_t *service, const hpd_method_t method, hpd_bool_t *boolean);
hpd_error_t hpd_service_first_action(const hpd_service_t *service, const hpd_action_t **action);
hpd_error_t hpd_service_next_action(const hpd_action_t **action);
hpd_error_t hpd_service_first_attr(const hpd_service_t *service, const hpd_pair_t **pair);
hpd_error_t hpd_service_next_attr(const hpd_pair_t **pair);
/// [hpd_service_t functions]

/// [hpd_action_t adapter functions]
hpd_error_t hpd_action_get_action(const hpd_action_t *action, hpd_action_f *cb);
/// [hpd_action_t adapter functions]

/// [hpd_parameter_t functions]
hpd_error_t hpd_parameter_alloc(hpd_parameter_t **parameter, const char *id);
hpd_error_t hpd_parameter_free(hpd_parameter_t *parameter);
hpd_error_t hpd_parameter_attach(hpd_service_t *service, hpd_parameter_t *parameter);
hpd_error_t hpd_parameter_id_attach(const hpd_service_id_t *id, hpd_parameter_t *parameter);
hpd_error_t hpd_parameter_detach(hpd_parameter_t *parameter);
hpd_error_t hpd_parameter_id_detach(const hpd_parameter_id_t *id, hpd_parameter_t **parameter);
hpd_error_t hpd_parameter_set_attr(hpd_parameter_t *parameter, const char *key, const char *val);
hpd_error_t hpd_parameter_set_attrs(hpd_parameter_t *parameter, ...);
hpd_error_t hpd_parameter_get_adapter_id_str(const hpd_parameter_t *parameter, const char **id);
hpd_error_t hpd_parameter_get_device_id_str(const hpd_parameter_t *parameter, const char **id);
hpd_error_t hpd_parameter_get_service_id_str(const hpd_parameter_t *parameter, const char **id);
hpd_error_t hpd_parameter_get_parameter_id_str(const hpd_parameter_t *parameter, const char **id);
hpd_error_t hpd_parameter_get_attr(const hpd_parameter_t *parameter, const char *key, const char **val);
hpd_error_t hpd_parameter_get_attrs(const hpd_parameter_t *parameter, ...);
hpd_error_t hpd_parameter_first_attr(const hpd_parameter_t *parameter, const hpd_pair_t **pair);
hpd_error_t hpd_parameter_next_attr(const hpd_pair_t **pair);
/// [hpd_parameter_t functions]

// TODO Document the following lot of functions/defines
#define HPD_SERVICE_FOREACH_ACTION(RC, ACTION, ID) for ( \
    (RC) = hpd_service_first_action((ID), &(ACTION)); \
    !(RC) && (ACTION); \
    (RC) = hpd_service_next_action(&(ACTION)))
#define HPD_ADAPTER_FOREACH_ATTR(RC, PAIR, ID) for ( \
    (RC) = hpd_adapter_first_attr((ID), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_adapter_next_attr(&(PAIR)))
#define HPD_DEVICE_FOREACH_ATTR(RC, PAIR, ID) for ( \
    (RC) = hpd_device_first_attr((ID), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_device_next_attr(&(PAIR)))
#define HPD_SERVICE_FOREACH_ATTR(RC, PAIR, ID) for ( \
    (RC) = hpd_service_first_attr((ID), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_service_next_attr(&(PAIR)))
#define HPD_PARAMETER_FOREACH_ATTR(RC, PAIR, ID) for ( \
    (RC) = hpd_parameter_first_attr((ID), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_parameter_next_attr(&(PAIR)))

hpd_error_t hpd_adapter_get_hpd(const hpd_adapter_t *adapter, hpd_t **hpd);
hpd_error_t hpd_adapter_get_device(const hpd_adapter_t *adapter, const char *id, hpd_device_t **device);
hpd_error_t hpd_device_get_hpd(const hpd_device_t *device, hpd_t **hpd);
hpd_error_t hpd_device_get_adapter(const hpd_device_t *device, const hpd_adapter_t **adapter);
hpd_error_t hpd_device_get_service(const hpd_device_t *device, const char *id, hpd_service_t **service);
hpd_error_t hpd_service_get_hpd(const hpd_service_t *service, hpd_t **hpd);
hpd_error_t hpd_service_get_adapter(const hpd_service_t *service, const hpd_adapter_t **adapter);
hpd_error_t hpd_service_get_device(const hpd_service_t *service, const hpd_device_t **device);
hpd_error_t hpd_service_get_parameter(const hpd_service_t *service, const char *id, hpd_parameter_t **parameter);
hpd_error_t hpd_parameter_get_hpd(const hpd_parameter_t *parameter, hpd_t **hpd);
hpd_error_t hpd_parameter_get_adapter(const hpd_parameter_t *parameter, const hpd_adapter_t **adapter);
hpd_error_t hpd_parameter_get_device(const hpd_parameter_t *parameter, const hpd_device_t **device);
hpd_error_t hpd_parameter_get_service(const hpd_parameter_t *parameter, const hpd_service_t **service);

hpd_error_t hpd_adapter_first_device(const hpd_adapter_t *adapter, hpd_device_t **device);
hpd_error_t hpd_adapter_first_service(const hpd_adapter_t *adapter, hpd_service_t **service);
hpd_error_t hpd_device_first_service(const hpd_device_t *device, hpd_service_t **service);
hpd_error_t hpd_service_first_parameter(const hpd_service_t *service, hpd_parameter_t **parameter);

hpd_error_t hpd_adapter_next_device(hpd_device_t **device);
hpd_error_t hpd_adapter_next_service(hpd_service_t **service);
hpd_error_t hpd_device_next_service(hpd_service_t **service);
hpd_error_t hpd_service_next_parameter(hpd_parameter_t **parameter);

#define HPD_ADAPTER_FOREACH_DEVICE(RC, DEVICE, ADAPTER) for ( \
    (RC) = hpd_adapter_first_device((ADAPTER), &(DEVICE)); \
    !(RC) && (DEVICE); \
    (RC) = hpd_adapter_next_device(&(DEVICE)))
#define HPD_ADAPTER_FOREACH_SERVICE(RC, SERVICE, ADAPTER) for ( \
    (RC) = hpd_adapter_first_service((ADAPTER), &(SERVICE)); \
    !(RC) && (SERVICE); \
    (RC) = hpd_adapter_next_service(&(SERVICE)))
#define HPD_DEVICE_FOREACH_SERVICE(RC, SERVICE, DEVICE) for ( \
    (RC) = hpd_device_first_service((DEVICE), &(SERVICE)); \
    !(RC) && (SERVICE); \
    (RC) = hpd_device_next_service(&(SERVICE)))
#define HPD_SERVICE_FOREACH_PARAMETER(RC, PARAMETER, SERVICE) for ( \
    (RC) = hpd_service_first_parameter((SERVICE), &(PARAMETER)); \
    !(RC) && (PARAMETER); \
    (RC) = hpd_service_next_parameter(&(PARAMETER)))

/// [hpd_request_t functions]
hpd_error_t hpd_request_get_service(const hpd_request_t *req, const hpd_service_id_t **id);
hpd_error_t hpd_request_get_method(const hpd_request_t *req, hpd_method_t *method);
hpd_error_t hpd_request_get_value(const hpd_request_t *req, const hpd_value_t **value);
/// [hpd_request_t functions]

/// [hpd_response_t functions]
hpd_error_t hpd_response_alloc(hpd_response_t **response, hpd_request_t *request, hpd_status_t status);
hpd_error_t hpd_response_free(hpd_response_t *response);
hpd_error_t hpd_response_set_value(hpd_response_t *response, hpd_value_t *value);
hpd_error_t hpd_respond(hpd_response_t *response);
/// [hpd_response_t functions]

/// [hpd_changed]
hpd_error_t hpd_id_changed(const hpd_service_id_t *id, hpd_value_t *val);
hpd_error_t hpd_changed(const hpd_service_t *service, hpd_value_t *val);
/// [hpd_changed]

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_HPD_ADAPTER_API_H
