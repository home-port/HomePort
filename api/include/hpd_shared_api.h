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

#ifndef HOMEPORT_HPD_SHARED_API_H
#define HOMEPORT_HPD_SHARED_API_H

#include <stddef.h>

#include "hpd_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/// [hpd_t functions]
hpd_error_t hpd_module_add_option(hpd_module_t *context, const char *name, const char *arg, int flags, const char *doc);
hpd_error_t hpd_module_get_id(hpd_module_t *context, const char **id);
hpd_error_t hpd_get_loop(hpd_t *hpd, hpd_ev_loop_t **loop);
/// [hpd_t functions]

/// [id_t functions]
hpd_error_t hpd_adapter_id_alloc(hpd_adapter_id_t **id, hpd_t *hpd, const char *aid);
hpd_error_t hpd_adapter_id_free(hpd_adapter_id_t *id);

hpd_error_t hpd_device_id_alloc(hpd_device_id_t **id, hpd_t *hpd, const char *aid, const char *did);
hpd_error_t hpd_device_id_free(hpd_device_id_t *id);

hpd_error_t hpd_service_id_alloc(hpd_service_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid);
hpd_error_t hpd_service_id_free(hpd_service_id_t *id);

hpd_error_t hpd_parameter_id_alloc(hpd_parameter_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid, const char *pid);
hpd_error_t hpd_parameter_id_free(hpd_parameter_id_t *id);
/// [id_t functions]

/// [hpd_adapter_t functions]
hpd_error_t hpd_adapter_get_id(hpd_adapter_id_t *aid, const char **id);
hpd_error_t hpd_adapter_get_attr(hpd_adapter_id_t *id, const char *key, const char **val);
hpd_error_t hpd_adapter_get_attrs(hpd_adapter_id_t *id, ...);
/// [hpd_adapter_t functions]

/// [hpd_device_t functions]
hpd_error_t hpd_device_get_id(hpd_device_id_t *did, const char **id);
hpd_error_t hpd_device_get_attr(hpd_device_id_t *id, const char *key, const char **val);
hpd_error_t hpd_device_get_attrs(hpd_device_id_t *id, ...);
/// [hpd_device_t functions]

/// [hpd_service_t functions]
hpd_error_t hpd_service_get_id(hpd_service_id_t *sid, const char **id);
hpd_error_t hpd_service_get_attr(hpd_service_id_t *id, const char *key, const char **val);
hpd_error_t hpd_service_get_attrs(hpd_service_id_t *id, ...);
hpd_error_t hpd_service_has_action(hpd_service_id_t *id, const hpd_method_t method, char *boolean);
hpd_error_t hpd_service_first_action(hpd_service_id_t *id, hpd_action_t **action);
hpd_error_t hpd_service_next_action(hpd_action_t **action);
/// [hpd_service_t functions]

/// [hpd_service_t foreach loops]
#define hpd_service_foreach_action(RC, ACTION, ID) for ( \
    (RC) = hpd_service_first_action((ID), (ACTION)); \
    !(RC) && (ACTION); \
    (RC) = hpd_service_next_action((ACTION)))
/// [hpd_service_t foreach loops]

/// [hpd_parameter_t functions]
hpd_error_t hpd_parameter_get_id(hpd_parameter_id_t *pid, const char **id);
hpd_error_t hpd_parameter_get_attr(hpd_parameter_id_t *id, const char *key, const char **val);
hpd_error_t hpd_parameter_get_attrs(hpd_parameter_id_t *id, ...);
/// [hpd_parameter_t functions]

/// [hpd_action_t functions]
hpd_error_t hpd_action_get_method(hpd_action_t *action, hpd_method_t *method);
/// [hpd_action_t functions]

/// [Browsing functions]
hpd_error_t hpd_adapter_get_hpd(hpd_adapter_id_t *aid, hpd_t **hpd);
hpd_error_t hpd_device_get_hpd(hpd_device_id_t *did, hpd_t **hpd);
hpd_error_t hpd_device_get_adapter(hpd_device_id_t *did, hpd_adapter_id_t **aid);
hpd_error_t hpd_service_get_hpd(hpd_service_id_t *sid, hpd_t **hpd);
hpd_error_t hpd_service_get_adapter(hpd_service_id_t *sid, hpd_adapter_id_t **aid);
hpd_error_t hpd_service_get_device(hpd_service_id_t *sid, hpd_device_id_t **did);
hpd_error_t hpd_parameter_get_hpd(hpd_parameter_id_t *pid, hpd_t **hpd);
hpd_error_t hpd_parameter_get_adapter(hpd_parameter_id_t *pid, hpd_adapter_id_t **aid);
hpd_error_t hpd_parameter_get_device(hpd_parameter_id_t *pid, hpd_device_id_t **did);
hpd_error_t hpd_parameter_get_service(hpd_parameter_id_t *pid, hpd_service_id_t **sid);

hpd_error_t hpd_first_adapter              (hpd_t *hpd,              hpd_adapter_t **adapter);
hpd_error_t hpd_first_device               (hpd_t *hpd,              hpd_device_t **device  );
hpd_error_t hpd_first_service              (hpd_t *hpd,              hpd_service_t **service);
hpd_error_t hpd_adapter_first_device       (hpd_adapter_t *adapter,      hpd_device_t **device  );
hpd_error_t hpd_adapter_first_service      (hpd_adapter_t *adapter,      hpd_service_t **service);
hpd_error_t hpd_device_first_service       (hpd_device_t *device,        hpd_service_t **service);
hpd_error_t hpd_service_first_parameter    (hpd_service_t *service,      hpd_parameter_t **parameter);

hpd_error_t hpd_next_adapter               (hpd_adapter_t **adapter);
hpd_error_t hpd_next_device                (hpd_device_t **device  );
hpd_error_t hpd_next_service               (hpd_service_t **service);
hpd_error_t hpd_adapter_next_device        (hpd_device_t **device  );
hpd_error_t hpd_adapter_next_service       (hpd_service_t **service);
hpd_error_t hpd_device_next_service        (hpd_service_t **service);
hpd_error_t hpd_service_next_parameter     (hpd_parameter_t **parameter);

hpd_error_t hpd_find_adapter               (hpd_t *hpd,              hpd_adapter_t **adapter,     ...);
hpd_error_t hpd_find_device                (hpd_t *hpd,              hpd_device_t **device,       ...);
hpd_error_t hpd_find_service               (hpd_t *hpd,              hpd_service_t **service,     ...);
hpd_error_t hpd_adapter_find_device        (hpd_adapter_t *adapter,      hpd_device_t **device,       ...);
hpd_error_t hpd_adapter_find_service       (hpd_adapter_t *adapter,      hpd_service_t **service,     ...);
hpd_error_t hpd_device_find_service        (hpd_device_t *device,        hpd_service_t **service,     ...);
hpd_error_t hpd_service_find_parameter     (hpd_service_t *service,      hpd_parameter_t **parameter, ...);

hpd_error_t hpd_find_next_adapter          (hpd_adapter_t **adapter,     ...);
hpd_error_t hpd_find_next_device           (hpd_device_t **device,       ...);
hpd_error_t hpd_find_next_service          (hpd_service_t **service,     ...);
hpd_error_t hpd_adapter_find_next_device   (hpd_device_t **device,       ...);
hpd_error_t hpd_adapter_find_next_service  (hpd_service_t **service,     ...);
hpd_error_t hpd_device_find_next_service   (hpd_service_t **service,     ...);
hpd_error_t hpd_service_find_next_parameter(hpd_parameter_t **parameter, ...);
/// [Browsing functions]

/// [Browsing foreach loops]
#define hpd_foreach_adapter(RC, ADAPTER, HPD) for ( \
    (RC) = hpd_first_adapter((HPD), (ADAPTER)); \
    !(RC) && (ADAPTER); \
    (RC) = hpd_next_adapter((ADAPTER)))
#define hpd_foreach_device(RC, DEVICE, HPD) for ( \
    (RC) = hpd_first_device((HPD), (DEVICE)); \
    !(RC) && (DEVICE); \
    (RC) = hpd_next_device((DEVICE)))
#define hpd_foreach_service(RC, SERVICE, HPD) for ( \
    (RC) = hpd_first_service((HPD), (SERVICE)); \
    !(RC) && (SERVICE); \
    (RC) = hpd_next_service((SERVICE)))
#define hpd_adapter_foreach_device(RC, DEVICE, ADAPTER) for ( \
    (RC) = hpd_adapter_first_device((ADAPTER), &(DEVICE)); \
    !(RC) && (DEVICE); \
    (RC) = hpd_adapter_next_device(&(DEVICE)))
#define hpd_adapter_foreach_service(RC, SERVICE, ADAPTER) for ( \
    (RC) = hpd_adapter_first_service((ADAPTER), (SERVICE)); \
    !(RC) && (SERVICE); \
    (RC) = hpd_adapter_next_service((SERVICE)))
#define hpd_device_foreach_service(RC, SERVICE, DEVICE) for ( \
    (RC) = hpd_device_first_service((DEVICE), (SERVICE)); \
    !(RC) && (SERVICE); \
    (RC) = hpd_device_next_service((SERVICE)))
#define hpd_service_foreach_parameter(RC, PARAMETER, SERVICE) for ( \
    (RC) = hpd_service_first_parameter((SERVICE), (PARAMETER)); \
    !(RC) && (PARAMETER); \
    (RC) = hpd_service_next_parameter((PARAMETER)))
#define hpd_find_foreach_adapter(RC, ADAPTER, HPD, ...) for ( \
    (RC) = hpd_find_adapter((HPD), (ADAPTER), ##__VA_ARGS__); \
    !(RC) && (ADAPTER); \
    (RC) = hpd_find_next_adapter((ADAPTER), ##__VA_ARGS__))
#define hpd_find_foreach_device(RC, DEVICE, HPD, ...) for ( \
    (RC) = hpd_find_device((HPD), (DEVICE), ##__VA_ARGS__); \
    !(RC) && (DEVICE); \
    (RC) = hpd_find_next_device((DEVICE), ##__VA_ARGS__))
#define hpd_find_foreach_service(RC, SERVICE, HPD, ...) for ( \
    (RC) = hpd_find_service((HPD), (SERVICE), ##__VA_ARGS__); \
    !(RC) && (SERVICE); \
    (RC) = hpd_find_next_service((SERVICE), ##__VA_ARGS__))
#define hpd_adapter_find_foreach_device(RC, DEVICE, ADAPTER, ...) for ( \
    (RC) = hpd_adapter_find_device((ADAPTER), &(DEVICE), ##__VA_ARGS__); \
    !(RC) && (DEVICE); \
    (RC) = hpd_adapter_find_next_device(&(DEVICE), ##__VA_ARGS__))
#define hpd_adapter_find_foreach_service(RC, SERVICE, ADAPTER, ...) for ( \
    (RC) = hpd_adapter_find_service((ADAPTER), (SERVICE), ##__VA_ARGS__); \
    !(RC) && (SERVICE); \
    (RC) = hpd_adapter_find_next_service((SERVICE), ##__VA_ARGS__))
#define hpd_device_find_foreach_service(RC, SERVICE, DEVICE, ...) for ( \
    (RC) = hpd_device_find_service((DEVICE), (SERVICE), ##__VA_ARGS__); \
    !(RC) && (SERVICE); \
    (RC) = hpd_device_find_next_service((SERVICE), ##__VA_ARGS__))
#define hpd_service_find_foreach_parameter(RC, PARAMETER, SERVICE, ...) for ( \
    (RC) = hpd_service_find_parameter((SERVICE), (PARAMETER), ##__VA_ARGS__); \
    !(RC) && (PARAMETER); \
    (RC) = hpd_service_find_next_parameter((PARAMETER), ##__VA_ARGS__))
/// [Browsing foreach loops]

/// [hpd_value_t functions]
hpd_error_t hpd_value_alloc(hpd_value_t **value, const char *body, int len);
hpd_error_t hpd_value_free(hpd_value_t *value);
hpd_error_t hpd_value_set_header(hpd_value_t *value, const char *key, const char *val);
hpd_error_t hpd_value_set_headers(hpd_value_t *value, ...);
hpd_error_t hpd_value_get_body(hpd_value_t *value, const char **body, size_t *len);
hpd_error_t hpd_value_get_header(hpd_value_t *value, const char *key, const char **val);
hpd_error_t hpd_value_get_headers(hpd_value_t *value, ...);
hpd_error_t hpd_value_first_header(hpd_value_t *value, hpd_pair_t **pair);
hpd_error_t hpd_value_next_header(hpd_pair_t **pair);
hpd_error_t hpd_pair_get(hpd_pair_t *pair, const char **key, const char **value);
/// [hpd_value_t functions]

/// [hpd_value_t foreach loops]
#define hpd_value_foreach_header(RC, PAIR, VALUE) for ( \
    (RC) = hpd_value_first_header((VALUE), (PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_value_next_header((PAIR)))
/// [hpd_value_t foreach loops]

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_HPD_SHARED_API_H
