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

#include <hpd/common/hpd_json.h>
#include <hpd/common/hpd_serialize_shared.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"
#include <curl/curl.h>
#pragma clang diagnostic pop

#include <hpd/hpd_shared_api.h>
#include <hpd/common/hpd_common.h>
#include <hpd/hpd_api.h>

#define HPD_JSON_RETURN_JSON_ERROR(CONTEXT) HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Json error")
#define HPD_JSON_RETURN_PARSE_ERROR(CONTEXT) HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Parse error")

static hpd_error_t json_add_str(json_t *parent, const char *key, const char *val, const hpd_module_t *context)
{
    json_t *json;
    if (!(json = json_string(val)))
        HPD_JSON_RETURN_JSON_ERROR(context);
    if (json_object_set_new(parent, key, json)) {
        json_decref(json);
        HPD_JSON_RETURN_JSON_ERROR(context);
    }
    return HPD_E_SUCCESS;
}

static hpd_error_t json_add_int(json_t *parent, const char *key, int val, const hpd_module_t *context)
{
    json_t *json;
    if (!(json = json_integer(val)))
        HPD_JSON_RETURN_JSON_ERROR(context);
    if (json_object_set_new(parent, key, json)) {
        json_decref(json);
        HPD_JSON_RETURN_JSON_ERROR(context);
    }
    return HPD_E_SUCCESS;
}

static hpd_error_t json_add_pair(json_t *parent, const hpd_pair_t *pair, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Get key and value
    const char *key, *val;
    if ((rc = hpd_pair_get(pair, &key, &val))) return rc;

    return json_add_str(parent, key, val, context);
}

hpd_error_t hpd_json_parameter_to_json(const hpd_module_t *context, const hpd_parameter_id_t *parameter, json_t **out)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_parameter_id_get_parameter_id_str(parameter, &id))) goto error;
    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_ID, id, context))) goto error;

    // Add attributes
    json_t *attrs;
    if (!(attrs = json_object())) goto json_error;
    const hpd_pair_t *pair;
    HPD_PARAMETER_ID_FOREACH_ATTR(rc, pair, parameter) {
        if ((rc = json_add_pair(attrs, pair, context))) {
            json_decref(attrs);
            goto error;
        }
    }
    if (rc) {
        json_decref(attrs);
        goto error;
    }
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_ATTRS, attrs)) {
        json_decref(attrs);
        goto json_error;
    }

    (*out) = json;
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;

    json_error:
    if (json) json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_parameters_to_json(const hpd_module_t *context, const hpd_service_id_t *service, json_t **out)
{
    hpd_error_t rc;

    // Create array
    json_t *json;
    if (!(json = json_array())) HPD_JSON_RETURN_JSON_ERROR(context);

    // Add parameters
    hpd_parameter_id_t *parameter;
    HPD_SERVICE_ID_FOREACH_PARAMETER_ID(rc, parameter, service) {
        json_t *child;
        if ((rc = hpd_json_parameter_to_json(context, parameter, &child))) goto error;
        if (json_array_append_new(json, child)) goto json_error;
    }
    if (rc) goto error;

    (*out) = json;
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;

    json_error:
    json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_service_to_json(const hpd_module_t *context, const hpd_service_id_t *service, json_t **out)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_service_id_get_service_id_str(service, &id))) goto error;
    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_ID, id, context))) goto error;

    // Add url
    char *url;
    if ((rc = hpd_serialize_url_create(context, service, &url))) goto error;
    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_URI, url, context))) {
        free(url);
        goto error;
    }
    free(url);

    // Add actions
    const hpd_action_t *action;
    HPD_SERVICE_ID_FOREACH_ACTION(rc, action, service) {
        hpd_method_t method;
        if ((rc = hpd_action_get_method(action, &method))) goto error;
        switch (method) {
            case HPD_M_NONE:break;
            case HPD_M_GET:
                if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_GET, HPD_SERIALIZE_VAL_TRUE, context))) goto error;
                break;
            case HPD_M_PUT:
                if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_PUT, HPD_SERIALIZE_VAL_TRUE, context))) goto error;
                break;
            case HPD_M_COUNT:break;
        }
    }
    if (rc) goto error;

    // Add attributes
    json_t *attrs;
    if (!(attrs = json_object())) goto json_error;
    const hpd_pair_t *pair;
    HPD_SERVICE_ID_FOREACH_ATTR(rc, pair, service) {
        if ((rc = json_add_pair(attrs, pair, context))) {
            json_decref(attrs);
            goto error;
        }
    }
    if (rc) {
        json_decref(attrs);
        goto error;
    }
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_ATTRS, attrs)) {
        json_decref(attrs);
        goto json_error;
    }

    // Add parameters
    json_t *child;
    if ((rc = hpd_json_parameters_to_json(context, service, &child))) goto error;
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_PARAMETERS, child)) goto json_error;

    (*out) = json;
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;

    json_error:
    json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_services_to_json(const hpd_module_t *context, const hpd_device_id_t *device, json_t **out)
{
    hpd_error_t rc;

    // Create array
    json_t *json;
    if (!(json = json_array())) HPD_JSON_RETURN_JSON_ERROR(context);

    // Add services
    hpd_service_id_t *service;
    HPD_DEVICE_ID_FOREACH_SERVICE_ID(rc, service, device) {
        json_t *child;
        if ((rc = hpd_json_service_to_json(context, service, &child))) goto error;
        if (json_array_append_new(json, child)) goto json_error;
    }
    if (rc) goto error;

    (*out) = json;
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;

    json_error:
    json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_device_to_json(const hpd_module_t *context, const hpd_device_id_t *device, json_t **out)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_device_id_get_device_id_str(device, &id))) goto error;
    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_ID, id, context))) goto error;

    // Add attributes
    json_t *attrs;
    if (!(attrs = json_object())) goto json_error;
    const hpd_pair_t *pair;
    HPD_DEVICE_ID_FOREACH_ATTR(rc, pair, device) {
        if ((rc = json_add_pair(attrs, pair, context))) {
            json_decref(attrs);
            goto error;
        }
    }
    if (rc) {
        json_decref(attrs);
        goto error;
    }
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_ATTRS, attrs)) {
        json_decref(attrs);
        goto json_error;
    }

    // Add services
    json_t *child;
    if ((rc = hpd_json_services_to_json(context, device, &child))) goto error;
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_SERVICES, child)) goto json_error;

    (*out) = json;
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;

    json_error:
    json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_devices_to_json(const hpd_module_t *context, const hpd_adapter_id_t *adapter, json_t **out)
{
    hpd_error_t rc;

    // Create array
    json_t *json;
    if (!(json = json_array())) HPD_JSON_RETURN_JSON_ERROR(context);

    // Add devices
    hpd_device_id_t *device;
    HPD_ADAPTER_ID_FOREACH_DEVICE_ID(rc, device, adapter) {
        json_t *child;
        if ((rc = hpd_json_device_to_json(context, device, &child))) goto error;
        if (json_array_append_new(json, child)) goto json_error;
    }
    if (rc) goto error;

    (*out) = json;
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;

    json_error:
    json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_adapter_to_json(const hpd_module_t *context, const hpd_adapter_id_t *adapter, json_t **out)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_adapter_id_get_adapter_id_str(adapter, &id))) goto error;
    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_ID, id, context))) goto error;

    // Add attributes
    json_t *attrs;
    if (!(attrs = json_object())) goto json_error;
    const hpd_pair_t *pair;
    HPD_ADAPTER_ID_FOREACH_ATTR(rc, pair, adapter) {
        if ((rc = json_add_pair(attrs, pair, context))) {
            json_decref(attrs);
            goto error;
        }
    }
    if (rc) {
        json_decref(attrs);
        goto error;
    }
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_ATTRS, attrs)) {
        json_decref(attrs);
        goto json_error;
    }

    // Add devices
    json_t *child;
    if ((rc = hpd_json_devices_to_json(context, adapter, &child))) goto error;
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_DEVICES, child)) goto json_error;

    (*out) = json;
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;

    json_error:
    json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_adapters_to_json(const hpd_module_t *context, json_t **out)
{
    hpd_error_t rc;

    // Create array
    json_t *json;
    if (!(json = json_array())) HPD_JSON_RETURN_JSON_ERROR(context);

    // Add adapters
    hpd_adapter_id_t *adapter;
    HPD_FOREACH_ADAPTER_ID(rc, adapter, context) {
        json_t *child;
        if ((rc = hpd_json_adapter_to_json(context, adapter, &child))) goto error;
        if (json_array_append_new(json, child)) goto json_error;
    }
    if (rc) goto error;

    (*out) = json;
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;

    json_error:
    json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_configuration_to_json(const hpd_module_t *context, json_t **out)
{
    hpd_error_t rc;

    // Create object
    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    // Add encoded charset
#ifdef CURL_ICONV_CODESET_OF_HOST
    curl_version_info_data *curl_ver = curl_version_info(CURLVERSION_NOW);
    if (curl_ver->features & CURL_VERSION_CONV && curl_ver->iconv_ver_num != 0)
        if ((rc = json_add(json, HPD_SERIALIZE_KEY_URL_ENCODED_CHARSET, CURL_ICONV_CODESET_OF_HOST, context))) goto error;
    else
        if ((rc = json_add(json, HPD_SERIALIZE_KEY_URL_ENCODED_CHARSET, HPD_SERIALIZE_VAL_ASCII, context))) goto error;
#else
    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_URL_ENCODED_CHARSET, HPD_SERIALIZE_VAL_ASCII, context))) goto error;
#endif

    // Add child
    json_t *child;
    if ((rc = hpd_json_adapters_to_json(context, &child))) goto error;
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_ADAPTERS, child)) goto json_error;

    (*out) = json;
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    return rc;

    json_error:
    json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_value_to_json(const hpd_module_t *context, const hpd_value_t *value, json_t **out)
{
    hpd_error_t rc;

    const char *body;
    size_t len;
    if ((rc = hpd_value_get_body(value, &body, &len))) return rc;

    char *null_term = NULL;
    HPD_STR_N_CPY(null_term, body, len);

    json_t *json;
    if (!(json = json_object())) goto json_error;

    // Add value
    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_VALUE, body, context))) goto error;

    // Add headers
    json_t *headers;
    if (!(headers = json_object())) goto json_error;
    const hpd_pair_t *pair;
    hpd_value_foreach_header(rc, pair, value) {
        if ((rc = json_add_pair(headers, pair, context))) {
            json_decref(headers);
            goto error;
        }
    }
    if (rc) {
        json_decref(headers);
        goto error;
    }
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_HEADERS, headers)) {
        json_decref(headers);
        goto json_error;
    }

    (*out) = json;
    free(null_term);
    return HPD_E_SUCCESS;

    error:
    json_decref(json);
    free(null_term);
    return rc;

    alloc_error:
    HPD_LOG_RETURN_E_ALLOC(context);

    json_error:
    if (json) json_decref(json);
    free(null_term);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_value_parse(const hpd_module_t *context, json_t *json, hpd_value_t **out)
{
    hpd_error_t rc, rc2;

    if (!json_is_object(json)) HPD_JSON_RETURN_PARSE_ERROR(context);

    json_t *body;
    if (!(body = json_object_get(json, HPD_SERIALIZE_KEY_VALUE))) HPD_JSON_RETURN_PARSE_ERROR(context);
    if (!json_is_string(body)) HPD_JSON_RETURN_PARSE_ERROR(context);

    hpd_value_t *hpd_val;
    if ((rc = hpd_value_alloc(&hpd_val, context, json_string_value(body), -1))) return rc;

    {
        const char *k;
        json_t *v;
        json_object_foreach(json, k, v) {
            if (k[0] != '_') {
                if (!json_is_string(v)) {
                    if ((rc2 = hpd_value_free(hpd_val))) HPD_LOG_ERROR(context, "Free failed [code: %d]", rc2);
                    HPD_JSON_RETURN_PARSE_ERROR(context);
                }
                if ((rc = hpd_value_set_header(hpd_val, k, json_string_value(v)))) {
                    if ((rc2 = hpd_value_free(hpd_val))) HPD_LOG_ERROR(context, "Free failed [code: %d]", rc2);
                    return rc;
                };
            }
        }
    }

    (*out) = hpd_val;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_json_adapter_id_to_json(const hpd_module_t *context, const hpd_adapter_id_t *adapter, json_t **out)
{
    hpd_error_t rc;

    const char *aid;
    if ((rc = hpd_adapter_id_get_adapter_id_str(adapter, &aid))) return rc;

    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_ADAPTER, aid, context))) {
        json_decref(json);
        return rc;
    }

    (*out) = json;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_json_device_id_to_json(const hpd_module_t *context, const hpd_device_id_t *device, json_t **out)
{
    hpd_error_t rc;

    const char *aid;
    if ((rc = hpd_device_id_get_adapter_id_str(device, &aid))) return rc;
    const char *did;
    if ((rc = hpd_device_id_get_device_id_str(device, &did))) return rc;

    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_ADAPTER, aid, context)) ||
        (rc = json_add_str(json, HPD_SERIALIZE_KEY_DEVICE, did, context)) ) {
        json_decref(json);
        return rc;
    }

    (*out) = json;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_json_service_id_to_json(const hpd_module_t *context, const hpd_service_id_t *service, json_t **out)
{
    hpd_error_t rc;

    const char *aid;
    if ((rc = hpd_service_id_get_adapter_id_str(service, &aid))) return rc;
    const char *did;
    if ((rc = hpd_service_id_get_device_id_str(service, &did))) return rc;
    const char *sid;
    if ((rc = hpd_service_id_get_service_id_str(service, &sid))) return rc;

    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_ADAPTER, aid, context)) ||
        (rc = json_add_str(json, HPD_SERIALIZE_KEY_DEVICE, did, context)) ||
        (rc = json_add_str(json, HPD_SERIALIZE_KEY_SERVICE, sid, context)) ) {
        json_decref(json);
        return rc;
    }

    (*out) = json;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_json_service_id_parse(const hpd_module_t *context, json_t *json, hpd_service_id_t **out)
{
    hpd_error_t rc;

    if (!json_is_object(json)) HPD_JSON_RETURN_PARSE_ERROR(context);

    json_t *adapter;
    if (!(adapter = json_object_get(json, HPD_SERIALIZE_KEY_ADAPTER))) HPD_JSON_RETURN_PARSE_ERROR(context);
    if (!json_is_string(adapter)) HPD_JSON_RETURN_PARSE_ERROR(context);

    json_t *device;
    if (!(device = json_object_get(json, HPD_SERIALIZE_KEY_DEVICE))) HPD_JSON_RETURN_PARSE_ERROR(context);
    if (!json_is_string(device)) HPD_JSON_RETURN_PARSE_ERROR(context);

    json_t *service;
    if (!(service = json_object_get(json, HPD_SERIALIZE_KEY_SERVICE))) HPD_JSON_RETURN_PARSE_ERROR(context);
    if (!json_is_string(service)) HPD_JSON_RETURN_PARSE_ERROR(context);

    hpd_service_id_t *service_id;
    if ((rc = hpd_service_id_alloc(&service_id, context, json_string_value(adapter), json_string_value(device), json_string_value(service))))
        return rc;

    (*out) = service_id;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_json_parameter_id_to_json(const hpd_module_t *context, const hpd_parameter_id_t *parameter, json_t **out)
{
    hpd_error_t rc;

    const char *aid;
    if ((rc = hpd_parameter_id_get_adapter_id_str(parameter, &aid))) return rc;
    const char *did;
    if ((rc = hpd_parameter_id_get_device_id_str(parameter, &did))) return rc;
    const char *sid;
    if ((rc = hpd_parameter_id_get_service_id_str(parameter, &sid))) return rc;
    const char *pid;
    if ((rc = hpd_parameter_id_get_parameter_id_str(parameter, &pid))) return rc;

    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_ADAPTER, aid, context)) ||
        (rc = json_add_str(json, HPD_SERIALIZE_KEY_DEVICE, did, context)) ||
        (rc = json_add_str(json, HPD_SERIALIZE_KEY_SERVICE, sid, context)) ||
        (rc = json_add_str(json, HPD_SERIALIZE_KEY_PARAMETER, pid, context)) ) {
        json_decref(json);
        return rc;
    }

    (*out) = json;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_json_response_to_json(const hpd_module_t *context, const hpd_response_t *response, json_t **out)
{
    hpd_error_t rc;
    json_t *child;

    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    const hpd_service_id_t *service;
    if ((rc = hpd_response_get_request_service(response, &service))) return rc;
    if ((rc = hpd_json_service_id_to_json(context, service, &child))) goto hpd_error;
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_SERVICE, child)) goto json_error;

    hpd_status_t status;
    if ((rc = hpd_response_get_status(response, &status))) goto hpd_error;
    if ((rc = json_add_int(json, HPD_SERIALIZE_KEY_STATUS, status, context))) goto hpd_error;

    const hpd_value_t *value;
    if ((rc = hpd_response_get_value(response, &value))) return rc;
    if ((rc = hpd_json_value_to_json(context, value, &child))) return rc;
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_VALUE, child)) goto json_error;

    (*out) = json;
    return HPD_E_SUCCESS;

    hpd_error:
    if (json) json_decref(json);
    return rc;

    json_error:
    if (json) json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_request_to_json(const hpd_module_t *context, const hpd_request_t *request, json_t **out)
{
    hpd_error_t rc;
    json_t *child;

    json_t *json;
    if (!(json = json_object())) HPD_JSON_RETURN_JSON_ERROR(context);

    const hpd_service_id_t *service;
    if ((rc = hpd_request_get_service(request, &service))) return rc;
    if ((rc = hpd_json_service_id_to_json(context, service, &child))) goto rc_error;
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_SERVICE, child)) goto json_error;

    hpd_method_t method;
    if ((rc = hpd_request_get_method(request, &method))) return rc;
    if (method < 0 || method >= HPD_M_COUNT) method = HPD_M_COUNT;
    if ((rc = json_add_str(json, HPD_SERIALIZE_KEY_METHOD, HPD_SERIALIZE_VAL_METHOD[method], context))) goto rc_error;

    const hpd_value_t *value;
    if ((rc = hpd_request_get_value(request, &value))) return rc;
    if ((rc = hpd_json_value_to_json(context, value, &child))) return rc;
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_VALUE, child)) goto json_error;

    (*out) = json;
    return HPD_E_SUCCESS;

    rc_error:
    if (json) json_decref(json);
    return rc;

    json_error:
    if (json) json_decref(json);
    HPD_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_json_request_parse(const hpd_module_t *context, json_t *json, hpd_response_f on_response, hpd_request_t **out)
{
    hpd_error_t rc, rc2;

    if (!json_is_object(json)) HPD_JSON_RETURN_PARSE_ERROR(context);

    json_t *json_service;
    if (!(json_service = json_object_get(json, HPD_SERIALIZE_KEY_SERVICE))) HPD_JSON_RETURN_PARSE_ERROR(context);
    if (!json_is_object(json_service)) HPD_JSON_RETURN_PARSE_ERROR(context);

    json_t *json_method;
    if (!(json_method = json_object_get(json, HPD_SERIALIZE_KEY_METHOD))) HPD_JSON_RETURN_PARSE_ERROR(context);
    if (!json_is_string(json_method)) HPD_JSON_RETURN_PARSE_ERROR(context);

    json_t *json_value;
    if ((json_value = json_object_get(json, HPD_SERIALIZE_KEY_VALUE))) {
        if (!json_is_object(json_value)) HPD_JSON_RETURN_PARSE_ERROR(context);
    }

    hpd_service_id_t *service_id;
    if ((rc = hpd_json_service_id_parse(context, json_service, &service_id))) return rc;
    
    hpd_method_t method;
    for (method = HPD_M_NONE; method < HPD_M_COUNT; method++)
        if (strcmp(HPD_SERIALIZE_VAL_METHOD[method], json_string_value(json_method)) == 0)
            break;

    hpd_value_t *value = NULL;
    if (json_value && (rc = hpd_json_value_parse(context, json_value, &value))) {
        if ((rc2 = hpd_service_id_free(service_id))) HPD_LOG_ERROR_CODE(context, rc2);
        return rc;
    }

    hpd_request_t *request;
    if ((rc = hpd_request_alloc(&request, service_id, method, on_response))) {
        if ((rc2 = hpd_service_id_free(service_id))) HPD_LOG_ERROR_CODE(context, rc2);
        if ((rc2 = hpd_value_free(value))) HPD_LOG_ERROR_CODE(context, rc2);
        return rc;
    }

    if ((rc = hpd_service_id_free(service_id))) HPD_LOG_RETURN_E_UNKNOWN_CODE(context, rc);

    if (json_value && (rc = hpd_request_set_value(request, value))) {
        if ((rc2 = hpd_value_free(value))) HPD_LOG_ERROR_CODE(context, rc2);
        if ((rc2 = hpd_request_free(request))) HPD_LOG_ERROR_CODE(context, rc2);
        return rc;
    }

    (*out) = request;
    return HPD_E_SUCCESS;
}
























