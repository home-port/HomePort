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

#ifndef HOMEPORT_HPD_SHARED_API_H
#define HOMEPORT_HPD_SHARED_API_H

#include <hpd/hpd_types.h>

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/// [hpd_t functions]
hpd_error_t hpd_module_add_option(const hpd_module_t *context, const char *name, const char *arg, int flags,
                                  const char *doc);
hpd_error_t hpd_module_get_id(const hpd_module_t *context, const char **id);
hpd_error_t hpd_get_loop(hpd_t *hpd, hpd_ev_loop_t **loop);
/// [hpd_t functions]

/// [log functions]
hpd_error_t hpd_logf(const hpd_module_t *context, hpd_log_level_t level, const char *file, int line, const char *fmt, ...);
hpd_error_t hpd_vlogf(const hpd_module_t *context, hpd_log_level_t level, const char *file, int line, const char *fmt, va_list vp);
#define HPD_LOG_ERROR(CONTEXT, FMT, ...) hpd_logf((CONTEXT), HPD_L_ERROR, __FILE__, __LINE__, (FMT), ##__VA_ARGS__)
#define HPD_LOG_WARN(CONTEXT, FMT, ...) hpd_logf((CONTEXT), HPD_L_WARN , __FILE__, __LINE__, (FMT), ##__VA_ARGS__)
#define HPD_LOG_INFO(CONTEXT, FMT, ...) hpd_logf((CONTEXT), HPD_L_INFO , __FILE__, __LINE__, (FMT), ##__VA_ARGS__)
#define HPD_LOG_DEBUG(CONTEXT, FMT, ...) hpd_logf((CONTEXT), HPD_L_DEBUG, __FILE__, __LINE__, (FMT), ##__VA_ARGS__)
#define HPD_LOG_VERBOSE(CONTEXT, FMT, ...) hpd_logf((CONTEXT), HPD_L_VERBOSE, __FILE__, __LINE__, (FMT), ##__VA_ARGS__)

#define HPD_LOG_RETURN(CONTEXT, E, FMT, ...) do { HPD_LOG_DEBUG((CONTEXT), (FMT), ##__VA_ARGS__); return (E); } while(0)
#define HPD_LOG_RETURN_E_NULL(CONTEXT)  HPD_LOG_RETURN((CONTEXT), HPD_E_NULL,  "Unexpected null pointer.")
#define HPD_LOG_RETURN_E_ALLOC(CONTEXT) HPD_LOG_RETURN((CONTEXT), HPD_E_ALLOC, "Unable to allocate memory.")
// TODO New function, check if it can be used elsewhere
#define HPD_LOG_RETURN_E_SNPRINTF(CONTEXT) HPD_LOG_RETURN((CONTEXT), HPD_E_UNKNOWN, "snprintf failed.")
/// [log functions]

/// [id_t functions]
hpd_error_t hpd_adapter_id_alloc(hpd_adapter_id_t **id, hpd_t *hpd, const char *aid);
hpd_error_t hpd_adapter_id_copy(hpd_adapter_id_t **dst, const hpd_adapter_id_t *src);
hpd_error_t hpd_adapter_id_free(hpd_adapter_id_t *id);

hpd_error_t hpd_device_id_alloc(hpd_device_id_t **id, hpd_t *hpd, const char *aid, const char *did);
hpd_error_t hpd_device_id_copy(hpd_device_id_t **dst, const hpd_device_id_t *src);
hpd_error_t hpd_device_id_free(hpd_device_id_t *id);

hpd_error_t hpd_service_id_alloc(hpd_service_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid);
hpd_error_t hpd_service_id_copy(hpd_service_id_t **dst, const hpd_service_id_t *src);
hpd_error_t hpd_service_id_free(hpd_service_id_t *id);

hpd_error_t hpd_parameter_id_alloc(hpd_parameter_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid, const char *pid);
hpd_error_t hpd_parameter_id_copy(hpd_parameter_id_t **dst, const hpd_parameter_id_t *src);
hpd_error_t hpd_parameter_id_free(hpd_parameter_id_t *id);
/// [id_t functions]

/// [hpd_adapter_t functions]
hpd_error_t hpd_adapter_get_id(const hpd_adapter_id_t *aid, const char **id);
hpd_error_t hpd_adapter_get_attr(const hpd_adapter_id_t *id, const char *key, const char **val);
hpd_error_t hpd_adapter_get_attrs(const hpd_adapter_id_t *id, ...);
hpd_error_t hpd_adapter_first_attr(const hpd_adapter_id_t *id, hpd_pair_t **pair);
hpd_error_t hpd_adapter_next_attr(hpd_pair_t **pair);
/// [hpd_adapter_t functions]

/// [hpd_device_t functions]
hpd_error_t hpd_device_get_id(const hpd_device_id_t *did, const char **id);
hpd_error_t hpd_device_get_attr(const hpd_device_id_t *id, const char *key, const char **val);
hpd_error_t hpd_device_get_attrs(const hpd_device_id_t *id, ...);
hpd_error_t hpd_device_first_attr(const hpd_device_id_t *id, hpd_pair_t **pair);
hpd_error_t hpd_device_next_attr(hpd_pair_t **pair);
/// [hpd_device_t functions]

/// [hpd_service_t functions]
hpd_error_t hpd_service_get_id(const hpd_service_id_t *sid, const char **id);
hpd_error_t hpd_service_get_attr(const hpd_service_id_t *id, const char *key, const char **val);
hpd_error_t hpd_service_get_attrs(const hpd_service_id_t *id, ...);
hpd_error_t hpd_service_has_action(const hpd_service_id_t *id, const hpd_method_t method, hpd_bool_t *boolean);
hpd_error_t hpd_service_first_action(const hpd_service_id_t *id, hpd_action_t **action);
hpd_error_t hpd_service_next_action(hpd_action_t **action);
hpd_error_t hpd_service_first_attr(const hpd_service_id_t *id, hpd_pair_t **pair);
hpd_error_t hpd_service_next_attr(hpd_pair_t **pair);
/// [hpd_service_t functions]

/// [hpd_parameter_t functions]
hpd_error_t hpd_parameter_get_id(const hpd_parameter_id_t *pid, const char **id);
hpd_error_t hpd_parameter_get_attr(const hpd_parameter_id_t *id, const char *key, const char **val);
hpd_error_t hpd_parameter_get_attrs(const hpd_parameter_id_t *id, ...);
hpd_error_t hpd_parameter_first_attr(const hpd_parameter_id_t *id, hpd_pair_t **pair);
hpd_error_t hpd_parameter_next_attr(hpd_pair_t **pair);
/// [hpd_parameter_t functions]

/// [model foreach loops]
#define hpd_service_foreach_action(RC, ACTION, ID) for ( \
    (RC) = hpd_service_first_action((ID), &(ACTION)); \
    !(RC) && (ACTION); \
    (RC) = hpd_service_next_action(&(ACTION)))
#define hpd_adapter_foreach_attr(RC, PAIR, ID) for ( \
    (RC) = hpd_adapter_first_attr((ID), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_adapter_next_attr(&(PAIR)))
#define hpd_device_foreach_attr(RC, PAIR, ID) for ( \
    (RC) = hpd_device_first_attr((ID), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_device_next_attr(&(PAIR)))
#define hpd_service_foreach_attr(RC, PAIR, ID) for ( \
    (RC) = hpd_service_first_attr((ID), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_service_next_attr(&(PAIR)))
#define hpd_parameter_foreach_attr(RC, PAIR, ID) for ( \
    (RC) = hpd_parameter_first_attr((ID), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_parameter_next_attr(&(PAIR)))
/// [model foreach loops]

/// [hpd_action_t functions]
hpd_error_t hpd_action_get_method(const hpd_action_t *action, hpd_method_t *method);
/// [hpd_action_t functions]

/// [hpd_pair_t functions]
hpd_error_t hpd_pair_get(const hpd_pair_t *pair, const char **key, const char **value);
/// [hpd_pair_t functions]

// TODO Rename get functions to get_alloc ?
/// [Browsing functions]
hpd_error_t hpd_adapter_get_hpd(const hpd_adapter_id_t *aid, hpd_t **hpd);
hpd_error_t hpd_device_get_hpd(const hpd_device_id_t *did, hpd_t **hpd);
hpd_error_t hpd_device_get_adapter(const hpd_device_id_t *did, hpd_adapter_id_t **aid);
hpd_error_t hpd_service_get_hpd(const hpd_service_id_t *sid, hpd_t **hpd);
hpd_error_t hpd_service_get_adapter(const hpd_service_id_t *sid, hpd_adapter_id_t **aid);
hpd_error_t hpd_service_get_device(const hpd_service_id_t *sid, hpd_device_id_t **did);
hpd_error_t hpd_parameter_get_hpd(const hpd_parameter_id_t *pid, hpd_t **hpd);
hpd_error_t hpd_parameter_get_adapter(const hpd_parameter_id_t *pid, hpd_adapter_id_t **aid);
hpd_error_t hpd_parameter_get_device(const hpd_parameter_id_t *pid, hpd_device_id_t **did);
hpd_error_t hpd_parameter_get_service(const hpd_parameter_id_t *pid, hpd_service_id_t **sid);

hpd_error_t hpd_first_adapter(hpd_t *hpd, hpd_adapter_id_t **adapter_id);
hpd_error_t hpd_first_device(hpd_t *hpd, hpd_device_id_t **device_id);
hpd_error_t hpd_first_service(hpd_t *hpd, hpd_service_id_t **service_id);
hpd_error_t hpd_adapter_first_device(const hpd_adapter_id_t *adapter_id, hpd_device_id_t **device_id);
hpd_error_t hpd_adapter_first_service(const hpd_adapter_id_t *adapter_id, hpd_service_id_t **service_id);
hpd_error_t hpd_device_first_service(const hpd_device_id_t *device_id, hpd_service_id_t **service_id);
hpd_error_t hpd_service_first_parameter(const hpd_service_id_t *service_id, hpd_parameter_id_t **parameter_id);

hpd_error_t hpd_next_adapter(hpd_adapter_id_t **adapter_id);
hpd_error_t hpd_next_device(hpd_device_id_t **device_id);
hpd_error_t hpd_next_service(hpd_service_id_t **service_id);
hpd_error_t hpd_adapter_next_device(hpd_device_id_t **device_id);
hpd_error_t hpd_adapter_next_service(hpd_service_id_t **service_id);
hpd_error_t hpd_device_next_service(hpd_service_id_t **service_id);
hpd_error_t hpd_service_next_parameter(hpd_parameter_id_t **parameter_id);
/// [Browsing functions]

/// [Browsing foreach loops]
#define hpd_foreach_adapter(RC, ADAPTER, HPD) for ( \
    (RC) = hpd_first_adapter((HPD), &(ADAPTER)); \
    !(RC) && (ADAPTER); \
    (RC) = hpd_next_adapter(&(ADAPTER)))
#define hpd_foreach_device(RC, DEVICE, HPD) for ( \
    (RC) = hpd_first_device((HPD), &(DEVICE)); \
    !(RC) && (DEVICE); \
    (RC) = hpd_next_device(&(DEVICE)))
#define hpd_foreach_service(RC, SERVICE, HPD) for ( \
    (RC) = hpd_first_service((HPD), &(SERVICE)); \
    !(RC) && (SERVICE); \
    (RC) = hpd_next_service(&(SERVICE)))
#define hpd_adapter_foreach_device(RC, DEVICE, ADAPTER) for ( \
    (RC) = hpd_adapter_first_device((ADAPTER), &(DEVICE)); \
    !(RC) && (DEVICE); \
    (RC) = hpd_adapter_next_device(&(DEVICE)))
#define hpd_adapter_foreach_service(RC, SERVICE, ADAPTER) for ( \
    (RC) = hpd_adapter_first_service((ADAPTER), &(SERVICE)); \
    !(RC) && (SERVICE); \
    (RC) = hpd_adapter_next_service(&(SERVICE)))
#define hpd_device_foreach_service(RC, SERVICE, DEVICE) for ( \
    (RC) = hpd_device_first_service((DEVICE), &(SERVICE)); \
    !(RC) && (SERVICE); \
    (RC) = hpd_device_next_service(&(SERVICE)))
#define hpd_service_foreach_parameter(RC, PARAMETER, SERVICE) for ( \
    (RC) = hpd_service_first_parameter((SERVICE), &(PARAMETER)); \
    !(RC) && (PARAMETER); \
    (RC) = hpd_service_next_parameter(&(PARAMETER)))
/// [Browsing foreach loops]

/// [hpd_value_t functions]
hpd_error_t hpd_value_alloc(hpd_value_t **value, const char *body, int len);
// TODO New function, check if it can be used elsewhere
hpd_error_t hpd_value_allocf(hpd_value_t **value, const char *fmt, ...);
hpd_error_t hpd_value_vallocf(hpd_value_t **value, const char *fmt, va_list vp);
hpd_error_t hpd_value_copy(hpd_value_t **dst, const hpd_value_t *src);
hpd_error_t hpd_value_free(hpd_value_t *value);
hpd_error_t hpd_value_set_header(hpd_value_t *value, const char *key, const char *val);
hpd_error_t hpd_value_set_headers(hpd_value_t *value, ...);
hpd_error_t hpd_value_get_body(const hpd_value_t *value, const char **body, size_t *len);
hpd_error_t hpd_value_get_header(const hpd_value_t *value, const char *key, const char **val);
hpd_error_t hpd_value_get_headers(const hpd_value_t *value, ...);
hpd_error_t hpd_value_first_header(const hpd_value_t *value, hpd_pair_t **pair);
hpd_error_t hpd_value_next_header(hpd_pair_t **pair);
/// [hpd_value_t functions]

/// [hpd_value_t foreach loops]
#define hpd_value_foreach_header(RC, PAIR, VALUE) for ( \
    (RC) = hpd_value_first_header((VALUE), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_value_next_header(&(PAIR)))
/// [hpd_value_t foreach loops]

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_HPD_SHARED_API_H
