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

#include "rest_xml.h"
#include <time.h>
#include <mxml.h>
#include <hpd/common/hpd_common.h>
#include <hpd/common/hpd_serialize_shared.h>
#include "hpd/hpd_application_api.h"

static const char * const REST_XML_VERSION = "1.0";

// TODO Fix ugly hack to get context through mxml ?
static const hpd_module_t *rest_xml_global_context = NULL;

static void rest_xml_on_mxml_error(const char *msg)
{
    HPD_LOG_DEBUG(rest_xml_global_context, "%s", msg);
}

#define REST_XML_BEGIN(CONTEXT) do { \
    if (rest_xml_global_context) HPD_LOG_WARN((CONTEXT), "Global context is already set (trying anyways)..."); \
    rest_xml_global_context = (CONTEXT); \
    mxmlSetErrorCallback(rest_xml_on_mxml_error); \
} while (0) \

#define REST_XML_END() do { \
    rest_xml_global_context = NULL; \
} while (0)

#define REST_XML_RETURN_XML_ERROR(CONTEXT) HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Xml error")

static hpd_error_t rest_xml_add(mxml_node_t *parent, const char *key, const char *val, const hpd_module_t *context)
{
    mxmlElementSetAttr(parent, key, val);
    if (!mxmlElementGetAttr(parent, key)) REST_XML_RETURN_XML_ERROR(context);
    return HPD_E_SUCCESS;
}

static hpd_error_t rest_xml_add_attr(mxml_node_t *parent, const hpd_pair_t *pair, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Get key and value
    const char *key, *val;
    if ((rc = hpd_pair_get(pair, &key, &val))) return rc;

    return rest_xml_add(parent, key, val, context);
}

static hpd_error_t rest_xml_add_parameter(mxml_node_t *parent, hpd_parameter_id_t *parameter, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create node
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_SERIALIZE_KEY_PARAMETER))) REST_XML_RETURN_XML_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_parameter_id_get_parameter_id_str(parameter, &id))) return rc;
    if ((rc = rest_xml_add(xml, HPD_SERIALIZE_KEY_ID, id, context))) return rc;

    // Add attributes
    const hpd_pair_t *pair;
    HPD_PARAMETER_ID_FOREACH_ATTR(rc, pair, parameter)
        if ((rc = rest_xml_add_attr(xml, pair, context))) return rc;
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

static hpd_error_t rest_xml_add_service(mxml_node_t *parent, hpd_service_id_t *service, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create node
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_SERIALIZE_KEY_SERVICE))) REST_XML_RETURN_XML_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_service_id_get_service_id_str(service, &id))) return rc;
    if ((rc = rest_xml_add(xml, HPD_SERIALIZE_KEY_ID, id, context))) return rc;

    // Add url
    char *url;
    if ((rc = hpd_serialize_url_create(context, service, &url))) return rc;
    if ((rc = rest_xml_add(xml, HPD_SERIALIZE_KEY_URI, url, context))) {
        free(url);
        return rc;
    }
    free(url);

    // Add actions
    const hpd_action_t *action;
    HPD_SERVICE_ID_FOREACH_ACTION(rc, action, service) {
        hpd_method_t method;
        if ((rc = hpd_action_get_method(action, &method))) return rc;
        switch (method) {
            case HPD_M_NONE:break;
            case HPD_M_GET:
                if ((rc = rest_xml_add(xml, HPD_SERIALIZE_KEY_GET, HPD_SERIALIZE_VAL_TRUE, context))) return rc;
                break;
            case HPD_M_PUT:
                if ((rc = rest_xml_add(xml, HPD_SERIALIZE_KEY_PUT, HPD_SERIALIZE_VAL_TRUE, context))) return rc;
                break;
            case HPD_M_COUNT:break;
        }
    }
    if (rc) return rc;

    // Add attributes
    const hpd_pair_t *pair;
    HPD_SERVICE_ID_FOREACH_ATTR(rc, pair, service)
        if ((rc = rest_xml_add_attr(xml, pair, context))) return rc;
    if (rc) return rc;

    // Add parameters
    hpd_parameter_id_t *parameter;
    HPD_SERVICE_ID_FOREACH_PARAMETER_ID(rc, parameter, service) {
        if ((rc = rest_xml_add_parameter(xml, parameter, context))) return rc;
    }
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

static hpd_error_t rest_xml_add_device(mxml_node_t *parent, hpd_device_id_t *device, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_SERIALIZE_KEY_DEVICE))) REST_XML_RETURN_XML_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_device_id_get_device_id_str(device, &id))) return rc;
    if ((rc = rest_xml_add(xml, HPD_SERIALIZE_KEY_ID, id, context))) return rc;

    // Add attributes
    const hpd_pair_t *pair;
    HPD_DEVICE_ID_FOREACH_ATTR(rc, pair, device)
        if ((rc = rest_xml_add_attr(xml, pair, context))) return rc;
    if (rc) return rc;

    // Add services
    hpd_service_id_t *service;
    HPD_DEVICE_ID_FOREACH_SERVICE_ID(rc, service, device) {
        if ((rc = rest_xml_add_service(xml, service, rest, context))) return rc;
    }
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

static hpd_error_t rest_xml_add_adapter(mxml_node_t *parent, hpd_adapter_id_t *adapter, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    mxml_node_t *json;
    if (!(json = mxmlNewElement(parent, HPD_SERIALIZE_KEY_ADAPTER))) REST_XML_RETURN_XML_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_adapter_id_get_adapter_id_str(adapter, &id))) return rc;
    if ((rc = rest_xml_add(json, HPD_SERIALIZE_KEY_ID, id, context))) return rc;

    // Add attributes
    const hpd_pair_t *pair;
    HPD_ADAPTER_ID_FOREACH_ATTR(rc, pair, adapter) {
        if ((rc = rest_xml_add_attr(json, pair, context))) return rc;
    }
    if (rc) return rc;

    // Add devices
    hpd_device_id_t *device;
    HPD_ADAPTER_ID_FOREACH_DEVICE_ID(rc, device, adapter) {
        if ((rc = rest_xml_add_device(json, device, rest, context))) return rc;
    }
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

static hpd_error_t rest_xml_add_configuration(mxml_node_t *parent, hpd_rest_t *rest, const hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_SERIALIZE_KEY_CONFIGURATION))) REST_XML_RETURN_XML_ERROR(context);

    // Add encoded charset
#ifdef CURL_ICONV_CODESET_OF_HOST
    curl_version_info_data *curl_ver = curl_version_info(CURLVERSION_NOW);
    if (curl_ver->features & CURL_VERSION_CONV && curl_ver->iconv_ver_num != 0)
        if ((rc = rest_xml_add(xml, HPD_SERIALIZE_KEY_URL_ENCODED_CHARSET, CURL_ICONV_CODESET_OF_HOST, context))) return rc;
    else
        if ((rc = rest_xml_add(xml, HPD_SERIALIZE_KEY_URL_ENCODED_CHARSET, HPD_SERIALIZE_VAL_ASCII, context))) return rc;
#else
    if ((rc = rest_xml_add(xml, HPD_SERIALIZE_KEY_URL_ENCODED_CHARSET, HPD_SERIALIZE_VAL_ASCII, context))) return rc;
#endif

    // Add adapters
    hpd_adapter_id_t *adapter;
    HPD_FOREACH_ADAPTER_ID(rc, adapter, context) {
        if ((rc = rest_xml_add_adapter(xml, adapter, rest, context))) return rc;
    }
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

hpd_error_t hpd_rest_xml_get_configuration(const hpd_module_t *context, hpd_rest_t *rest, char **out)
{
    REST_XML_BEGIN(context);

    hpd_error_t rc;

    mxml_node_t *xml;
    if (!(xml = mxmlNewXML(REST_XML_VERSION))) {
        REST_XML_END();
        REST_XML_RETURN_XML_ERROR(context);
    }

    if ((rc = rest_xml_add_configuration(xml, rest, context))) goto error;

    if (!((*out) = mxmlSaveAllocString(xml, MXML_NO_CALLBACK))) {
        mxmlDelete(xml);
        REST_XML_END();
        REST_XML_RETURN_XML_ERROR(context);
    }

    mxmlDelete(xml);
    REST_XML_END();
    return HPD_E_SUCCESS;

    error:
    mxmlDelete(xml);
    REST_XML_END();
    return rc;
}

static hpd_error_t rest_xml_add_value(mxml_node_t *parent, char *value, const hpd_module_t *context)
{
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_SERIALIZE_KEY_VALUE))) REST_XML_RETURN_XML_ERROR(context);

    if (!mxmlNewText(xml, 0, value)) REST_XML_RETURN_XML_ERROR(context);

    return HPD_E_SUCCESS;
}

hpd_error_t hpd_rest_xml_get_value(char *value, const hpd_module_t *context, char **out)
{
    REST_XML_BEGIN(context);

    hpd_error_t rc;

    mxml_node_t *xml;
    if (!(xml = mxmlNewXML(REST_XML_VERSION))) {
        REST_XML_END();
        REST_XML_RETURN_XML_ERROR(context);
    }

    if ((rc = rest_xml_add_value(xml, value, context))) goto error;

    if (!((*out) = mxmlSaveAllocString(xml, MXML_NO_CALLBACK))) {
        mxmlDelete(xml);
        REST_XML_END();
        REST_XML_RETURN_XML_ERROR(context);
    }

    mxmlDelete(xml);
    REST_XML_END();
    return HPD_E_SUCCESS;

    error:
    mxmlDelete(xml);
    REST_XML_END();
    return rc;
}

hpd_error_t hpd_rest_xml_parse_value(const char *in, const hpd_module_t *context, char **out)
{
    REST_XML_BEGIN(context);

    mxml_node_t *xml;
    if (!(xml = mxmlLoadString(NULL, in, MXML_TEXT_CALLBACK))) {
        REST_XML_END();
        HPD_LOG_RETURN(context, HPD_E_ARGUMENT, "XML parsing error");
    }

    mxml_node_t *node;
    node = mxmlFindElement(xml, xml, HPD_SERIALIZE_KEY_VALUE, NULL, NULL, MXML_DESCEND);
    if (!node) {
        mxmlDelete(xml);
        REST_XML_END();
        HPD_LOG_RETURN(context, HPD_E_ARGUMENT, "XML parsing error");
    }

    size_t len = 1;
    HPD_CALLOC(*out, len, char);
    for (node = node->child; node && node->type == MXML_TEXT; node = node->next) {
        int whitespaces = node->value.text.whitespace;
        char *string = node->value.text.string;
        size_t str_len = strlen(string);
        HPD_REALLOC(*out, len + whitespaces + str_len, char);
        for (int i = 0; i < whitespaces; i++) strcat(*out, " ");
        strcat(*out, string);
        len += whitespaces + str_len;
    }

    mxmlDelete(xml);
    REST_XML_END();
    return HPD_E_SUCCESS;

    alloc_error:
        mxmlDelete(xml);
        REST_XML_END();
        HPD_LOG_RETURN_E_ALLOC(context);
}


