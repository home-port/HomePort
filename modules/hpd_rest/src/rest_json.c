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

#include "rest_json.h"
#include "hpd/common/hpd_jansson.h"
#include "hpd/hpd_application_api.h"
#include <string.h>

#define REST_JSON_RETURN_JSON_ERROR(CONTEXT) HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Json error")

static hpd_error_t rest_json_add(json_t *parent, const char *key, const char *val, const hpd_module_t *context)
{
    json_t *json;
    if (!(json = json_string(val)))
        REST_JSON_RETURN_JSON_ERROR(context);
    if (json_object_set_new(parent, key, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }
    return HPD_E_SUCCESS;
}

static hpd_error_t rest_json_add_attr(json_t *parent, hpd_pair_t *pair, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Get key and value
    const char *key, *val;
    if ((rc = hpd_pair_get(pair, &key, &val))) return rc;

    return rest_json_add(parent, key, val, context);
}

static hpd_error_t rest_json_add_parameter(json_t *parent, hpd_parameter_id_t *parameter, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_parameter_get_id(parameter, &id))) goto error;
    if ((rc = rest_json_add(json, HPD_REST_KEY_ID, id, context))) goto error;

    // Add attributes
    hpd_pair_t *pair;
    hpd_parameter_foreach_attr(rc, pair, parameter)
        if ((rc = rest_json_add_attr(json, pair, context))) goto error;
    if (rc) goto error;

    // Add to parent
    if (json_array_append_new(parent, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    return HPD_E_SUCCESS;

    error:
        json_decref(json);
        return rc;
}

static hpd_error_t rest_json_add_parameters(json_t *parent, hpd_service_id_t *service, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create array
    json_t *json;
    if (!(json = json_array())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add parameters
    hpd_parameter_id_t *parameter;
    hpd_service_foreach_parameter(rc, parameter, service) {
        if ((rc = rest_json_add_parameter(json, parameter, context))) goto error;
    }
    if (rc) goto error;

    // Add to parent
    if (json_object_set_new(parent, HPD_REST_KEY_PARAMETER, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

static hpd_error_t rest_json_add_service(json_t *parent, hpd_service_id_t *service, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_service_get_id(service, &id))) goto error;
    if ((rc = rest_json_add(json, HPD_REST_KEY_ID, id, context))) goto error;

    // Add url
    char *url;
    if ((rc = hpd_rest_url_create(rest, service, &url))) goto error;
    if ((rc = rest_json_add(json, HPD_REST_KEY_URI, url, context))) {
        free(url);
        goto error;
    }
    free(url);

    // Add actions
    hpd_action_t *action;
    hpd_service_foreach_action(rc, action, service) {
        hpd_method_t method;
        if ((rc = hpd_action_get_method(action, &method))) goto error;
        switch (method) {
            case HPD_M_NONE:break;
            case HPD_M_GET:
                if ((rc = rest_json_add(json, HPD_REST_KEY_GET, HPD_REST_VAL_TRUE, context))) goto error;
                break;
            case HPD_M_PUT:
                if ((rc = rest_json_add(json, HPD_REST_KEY_PUT, HPD_REST_VAL_TRUE, context))) goto error;
                break;
            case HPD_M_COUNT:break;
        }
    }
    if (rc) goto error;

    // Add attributes
    hpd_pair_t *pair;
    hpd_service_foreach_attr(rc, pair, service)
        if ((rc = rest_json_add_attr(json, pair, context))) goto error;
    if (rc) goto error;

    // Add parameters
    if ((rc = rest_json_add_parameters(json, service, context))) goto error;

    // Add to parent
    if (json_array_append_new(parent, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

static hpd_error_t rest_json_add_services(json_t *parent, hpd_device_id_t *device, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create array
    json_t *json;
    if (!(json = json_array())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add services
    hpd_service_id_t *service;
    hpd_device_foreach_service(rc, service, device) {
        if ((rc = rest_json_add_service(json, service, rest, context))) goto error;
    }
    if (rc) goto error;

    // Add to parent
    if (json_object_set_new(parent, HPD_REST_KEY_SERVICE, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

static hpd_error_t rest_json_add_device(json_t *parent, hpd_device_id_t *device, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_device_get_id(device, &id))) goto error;
    if ((rc = rest_json_add(json, HPD_REST_KEY_ID, id, context))) goto error;

    // Add attributes
    hpd_pair_t *pair;
    hpd_device_foreach_attr(rc, pair, device)
        if ((rc = rest_json_add_attr(json, pair, context))) goto error;
    if (rc) goto error;

    // Add services
    if ((rc = rest_json_add_services(json, device, rest, context))) goto error;

    // Add to parent
    if (json_array_append_new(parent, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

static hpd_error_t rest_json_add_devices(json_t *parent, hpd_adapter_id_t *adapter, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create array
    json_t *json;
    if (!(json = json_array())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add devices
    hpd_device_id_t *device;
    hpd_adapter_foreach_device(rc, device, adapter) {
        if ((rc = rest_json_add_device(json, device, rest, context))) goto error;
    }
    if (rc) goto error;

    // Add to parent
    if (json_object_set_new(parent, HPD_REST_KEY_DEVICE, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

static hpd_error_t rest_json_add_adapter(json_t *parent, hpd_adapter_id_t *adapter, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_adapter_get_id(adapter, &id))) goto error;
    if ((rc = rest_json_add(json, HPD_REST_KEY_ID, id, context))) goto error;

    // Add attributes
    hpd_pair_t *pair;
    hpd_adapter_foreach_attr(rc, pair, adapter) {
        if ((rc = rest_json_add_attr(json, pair, context))) goto error;
    }
    if (rc) goto error;

    // Add devices
    if ((rc = rest_json_add_devices(json, adapter, rest, context))) goto error;

    // Add to parent
    if (json_array_append_new(parent, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

static hpd_error_t rest_json_add_adapters(json_t *parent, hpd_t *hpd, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create array
    json_t *json;
    if (!(json = json_array())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add adapters
    hpd_adapter_id_t *adapter;
    hpd_foreach_adapter(rc, adapter, hpd) {
        if ((rc = rest_json_add_adapter(json, adapter, rest, context))) goto error;
    }
    if (rc) goto error;

    // Add to parent
    if (json_object_set_new(parent, HPD_REST_KEY_ADAPTER, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

static hpd_error_t rest_json_add_configuration(json_t *parent, hpd_t *hpd, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add encoded charset
#ifdef CURL_ICONV_CODESET_OF_HOST
    curl_version_info_data *curl_ver = curl_version_info(CURLVERSION_NOW);
    if (curl_ver->features & CURL_VERSION_CONV && curl_ver->iconv_ver_num != 0)
        if ((rc = rest_json_add(json, HPD_REST_KEY_URL_ENCODED_CHARSET, CURL_ICONV_CODESET_OF_HOST, context))) goto error;
    else
        if ((rc = rest_json_add(json, HPD_REST_KEY_URL_ENCODED_CHARSET, HPD_REST_VAL_ASCII, context))) goto error;
#else
    if ((rc = rest_json_add(json, HPD_REST_KEY_URL_ENCODED_CHARSET, HPD_REST_VAL_ASCII, context))) goto error;
#endif

    // Add adapters
    if ((rc = rest_json_add_adapters(json, hpd, rest, context))) goto error;

    // Add to parent
    if (json_object_set_new(parent, HPD_REST_KEY_CONFIGURATION, json)) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

hpd_error_t hpd_rest_json_get_configuration(hpd_t *hpd, hpd_rest_t *rest, const hpd_module_t *context, char **out)
{
    hpd_error_t rc;

    json_t *json;
    if (!(json = json_object())) REST_JSON_RETURN_JSON_ERROR(context);

    if ((rc = rest_json_add_configuration(json, hpd, rest, context))) goto error;

    if (!((*out) = json_dumps(json, 0))) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    json_decref(json);
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

hpd_error_t hpd_rest_json_get_value(char *value, const hpd_module_t *context, char **out)
{
    hpd_error_t rc;

    json_t *json;
    if (!(json = json_object())) REST_JSON_RETURN_JSON_ERROR(context);

    // Add timestamp
    char timestamp[21];
    if ((rc = hpd_rest_get_timestamp(context, timestamp))) goto error;
    if ((rc = rest_json_add(json, HPD_REST_KEY_TIMESTAMP, timestamp, context))) goto error;

    // Add value
    if ((rc = rest_json_add(json, HPD_REST_KEY_VALUE, value, context))) goto error;

    if (!((*out) = json_dumps(json, 0))) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    json_decref(json);
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;
}

hpd_error_t hpd_rest_json_parse_value(const char *in, const hpd_module_t *context, char **out)
{
    // Load json
    json_t *json = NULL;
    json_error_t *error = NULL;
    if (!(json = json_loads(in, 0, error))) {
        HPD_LOG_RETURN(context, HPD_E_ARGUMENT, "Json parsing error: %s", error->text);
    }

    // Get value
    json_t *value;
    if (!json_is_object(json) || !(value = json_object_get(json, HPD_REST_KEY_VALUE))) {
        json_decref(json);
        HPD_LOG_RETURN(context, HPD_E_ARGUMENT, "Json parsing error");
    }

    // Allocate a copy
    (*out) = malloc((json_string_length(value)+1)*sizeof(char));
    if (!(*out)) {
        json_decref(json);
        HPD_LOG_RETURN_E_ALLOC(context);
    }
    strcpy((*out), json_string_value(value));

    json_decref(json);
    return HPD_E_SUCCESS;
}


