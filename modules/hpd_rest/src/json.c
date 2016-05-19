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

#include "json.h"
#include <jansson.h>
#include "hpd_application_api.h"
#include <string.h>
#include "hpd_rest_intern.h"

// TODO Error handling on the entire file !!!
// TODO Shouldn't json object be free'd on errors ?

static json_t *add_attr(json_t *json, hpd_pair_t *pair)
{
    json_t *value;
    const char *key, *val;

    hpd_pair_get(pair, &key, &val);
    // TODO Verify that key is not those below... (use encoding?)
    if (!(value = json_string(val)) || json_object_set_new(json, key, value) != 0) return NULL;
    return value;
}

static json_t *add_id(json_t *json, const char *id)
{
    json_t *value;
    if (!(value = json_string(id)) || json_object_set_new(json, "_id", value) != 0) return NULL;
    return value;
}

static json_t *parameterToJson(hpd_parameter_id_t *parameter)
{
    json_t *parameterJson;
    hpd_error_t rc;
    hpd_pair_t *pair;

    if (!(parameterJson = json_object())) return NULL;

    hpd_parameter_foreach_attr(rc, pair, parameter)
        if (!add_attr(parameterJson, pair)) return NULL;

    const char *id;
    hpd_parameter_get_id(parameter, &id);
    if (!add_id(parameterJson, id)) return NULL;

    return parameterJson;
}

static json_t*
serviceToJson(hpd_rest_t *rest, hpd_service_id_t *service)
{
    json_t *serviceJson;
    json_t *value;
    hpd_error_t rc;
    hpd_pair_t *pair;

    if (!(serviceJson = json_object())) return NULL;

    hpd_service_foreach_attr(rc, pair, service)
        if (!add_attr(serviceJson, pair)) return NULL;

    const char *id;
    hpd_service_get_id(service, &id);
    if (!add_id(serviceJson, id)) return NULL;

    char *url;
    hpd_rest_url_create(rest, service, &url); // TODO error check
    if (url) {
        if (!(value = json_string(url)) || json_object_set_new(serviceJson, "_uri", value) != 0) {
            free(url);
            return NULL;
        }
        free(url);
    }

    hpd_action_t *action;
    hpd_service_foreach_action(rc, action, service) {
        hpd_method_t method;
        hpd_action_get_method(action, &method);
        switch (method) {
            case HPD_M_NONE:break;
            case HPD_M_GET:
                if( ( ( value = json_string("1") ) == NULL ) || (json_object_set_new(serviceJson, "_get", value) != 0 ) )
                {
                    return NULL;
                }
                break;
            case HPD_M_PUT:
                if( ( ( value = json_string("1") ) == NULL ) || (json_object_set_new(serviceJson, "_put", value) != 0 ) )
                {
                    return NULL;
                }
                break;
            case HPD_M_COUNT:break;
        }
    }

    hpd_parameter_id_t *parameter;
    hpd_service_foreach_parameter(rc, parameter, service) {
        json_t *parameterJson = parameterToJson(parameter);
        if( parameterJson == NULL || json_object_set_new(serviceJson, "parameter", parameterJson) != 0 ) {
            return NULL;
        }
    }

    return serviceJson;
}

static json_t*
deviceToJson(hpd_rest_t *rest, hpd_device_id_t *device)
{
    json_t *deviceJson;
    json_t *serviceArray;
    hpd_error_t rc;
    hpd_pair_t *pair;

    if( ( deviceJson = json_object() ) == NULL )
    {
        return NULL;
    }

    hpd_device_foreach_attr(rc, pair, device)
        if (!add_attr(deviceJson, pair)) return NULL;

    const char *id;
    hpd_device_get_id(device, &id);
    if (!add_id(deviceJson, id)) return NULL;

    if( ( serviceArray = json_array() ) == NULL )
    {
        return NULL;
    }

    hpd_service_id_t *service;
    hpd_device_foreach_service(rc, service, device) {
        if( json_array_append_new(serviceArray, serviceToJson(rest, service)) != 0 )
        {
            return NULL;
        }
    }

    if( json_object_set_new(deviceJson, "service", serviceArray) != 0 )
    {
        return NULL;
    }

    return deviceJson;
}

static json_t*
adapterToJson(hpd_rest_t *rest, hpd_adapter_id_t *adapter)
{
    json_t *adapterJson = NULL;
    json_t *value = NULL;
    json_t *deviceArray = NULL;
    hpd_error_t rc;
    hpd_pair_t *pair;

    if( ( adapterJson = json_object() ) == NULL )
    {
        goto error;
    }

    hpd_adapter_foreach_attr(rc, pair, adapter)
        if (!add_attr(adapterJson, pair)) return NULL;

    const char *id;
    hpd_adapter_get_id(adapter, &id);
    if (!add_id(adapterJson, id)) return NULL;

    if( ( deviceArray = json_array() ) == NULL )
    {
        goto error;
    }

    hpd_device_id_t *device;
    hpd_adapter_foreach_device(rc, device, adapter)
    {
            json_t *deviceJson;
            if( ( ( deviceJson = deviceToJson(rest, device) ) == NULL ) || ( json_array_append_new(deviceArray, deviceJson) != 0 ) )
            {
                goto error;
            }
    }

    if( json_object_set_new(adapterJson, "device", deviceArray) != 0 )
    {
        goto error;
    }

    return adapterJson;
    error:
    if(adapterJson) json_decref(adapterJson);
    if(value) json_decref(value);
    if(deviceArray) json_decref(deviceArray);
    return NULL;
}

static json_t*
configurationToJson(hpd_rest_t *rest, hpd_t *hpd)
{
    hpd_error_t rc;
    json_t *configJson=NULL;
    json_t *adapterArray=NULL;
    json_t *adapter=NULL;
    json_t *value=NULL;

    if( ( configJson = json_object() ) == NULL )
    {
        goto error;
    }

#ifdef CURL_ICONV_CODESET_OF_HOST
    curl_version_info_data *curl_ver = curl_version_info(CURLVERSION_NOW);
  if (curl_ver->features & CURL_VERSION_CONV && curl_ver->iconv_ver_num != 0)
    if(((value = json_string(CURL_ICONV_CODESET_OF_HOST)) == NULL) || (json_object_set_new(configJson, "urlEncodedCharset", value) != 0)) goto error;
  else
    if(((value = json_string("ASCII")) == NULL) || (json_object_set_new(configJson, "urlEncodedCharset", value) != 0)) goto error;
#else
    if(((value = json_string("ASCII")) == NULL) || (json_object_set_new(configJson, "urlEncodedCharset", value) != 0)) goto error;
#endif

    hpd_adapter_id_t *iterator;

    if( ( adapterArray = json_array() ) == NULL )
    {
        goto error;
    }

    hpd_foreach_adapter(rc, iterator, hpd)
    {
        adapter = adapterToJson(rest, iterator);
        if( ( adapter == NULL ) || ( json_array_append_new(adapterArray, adapter) != 0 ) )
        {
            goto error;
        }
    }

    if( json_object_set_new(configJson, "adapter", adapterArray) != 0 )
    {
        goto error;
    }

    return configJson;
    error:
    if(adapter) json_decref(adapter);
    if(adapterArray) json_decref(adapterArray);
    if(configJson) json_decref(configJson);
    return NULL;
}

char *
jsonGetConfiguration(hpd_rest_t *rest, hpd_t *hpd)
{
    char *res;
    json_t *configurationJson = configurationToJson(rest, hpd);
    res = json_dumps( configurationJson, 0 );
    json_decref(configurationJson);
    return res;
}

char*
jsonGetState(char *state)
{
    json_t *json=NULL;
    json_t *value=NULL;

    if( ( json = json_object() ) == NULL )
    {
        goto error;
    }
    if( ( ( value = json_string(state) ) == NULL ) || ( json_object_set_new(json, "value", value) != 0 ) )
    {
        goto error;
    }

    char *ret = json_dumps( json, 0 );
    json_decref(json);
    return ret;

    error:
    if(value) json_decref(value);
    if(json) json_decref(json);
    return NULL;
}

const char*
jsonParseState(char *json_value)
{
    json_t *json = NULL;
    json_error_t *error=NULL;
    json_t *value = NULL;

    if( ( json = json_loads(json_value, 0, error) ) == NULL )
    {
        goto error;
    }

    if( json_is_object(json) == 0 )
    {
        goto error;
    }

    if( ( value = json_object_get(json, "value") ) == NULL )
    {
        goto error;
    }

    const char *ret = json_string_value(value);
    char *ret_alloc = malloc((strlen(ret)+1)*sizeof(char));
    strcpy(ret_alloc, ret);

    json_decref(json);

    return ret_alloc;

    error:
    return NULL;
}


