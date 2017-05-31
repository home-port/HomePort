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

#include "rest_json.h"
#include "hpd/common/hpd_jansson.h"
#include "hpd/hpd_application_api.h"
#include <string.h>
#include <hpd/common/hpd_serialize_shared.h>
#include <hpd/common/hpd_json.h>

#define REST_JSON_RETURN_JSON_ERROR(CONTEXT) HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Json error")

hpd_error_t hpd_rest_json_get_configuration(const hpd_module_t *context, hpd_rest_t *rest, char **out)
{
    hpd_error_t rc;

    json_t *json;
    if (!(json = json_object())) REST_JSON_RETURN_JSON_ERROR(context);

    json_t *child;
    if ((rc = hpd_json_configuration_to_json(context, &child))) return rc;
    if (json_object_set_new(json, HPD_SERIALIZE_KEY_CONFIGURATION, child)) goto json_error;

    if (!((*out) = json_dumps(json, 0))) goto json_error;

    json_decref(json);
    return HPD_E_SUCCESS;

    json_error:
    json_decref(json);
    REST_JSON_RETURN_JSON_ERROR(context);
}

hpd_error_t hpd_rest_json_get_value(const hpd_value_t *value, const hpd_module_t *context, char **out)
{
    hpd_error_t rc;

    json_t *json;
    if ((rc = hpd_json_value_to_json(context, value, &json))) return rc;

    if (!((*out) = json_dumps(json, 0))) {
        json_decref(json);
        REST_JSON_RETURN_JSON_ERROR(context);
    }

    json_decref(json);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_rest_json_parse_value(const char *in, const hpd_module_t *context, hpd_value_t **out)
{
    hpd_error_t rc;

    // Load json
    json_t *json = NULL;
    json_error_t *error = NULL;
    if (!(json = json_loads(in, 0, error))) {
        HPD_LOG_RETURN(context, HPD_E_ARGUMENT, "Json parsing error: %s", error->text);
    }

    // Get value
    if ((rc = hpd_json_value_parse(context, json, out))) {
        json_decref(json);
        return rc;
    }

    json_decref(json);
    return HPD_E_SUCCESS;
}


