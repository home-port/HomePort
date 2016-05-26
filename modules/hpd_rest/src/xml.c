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

#include "xml.h"
#include <time.h>
#include <mxml.h>
#include <zlib.h>
#include <hpd_common.h>
#include "hpd_application_api.h"
#include "hpd_rest_intern.h"

static const char * const XML_VERSION = "1.0";

// TODO Ugly hack to get context through mxml
static hpd_module_t *global_context = NULL;

static void on_mxml_error(const char *msg)
{
    HPD_LOG_DEBUG(global_context, "%s", msg);
}

#define BEGIN(CONTEXT) do { \
    if (global_context) HPD_LOG_WARN((CONTEXT), "Global context is already set (trying anyways)..."); \
    global_context = (CONTEXT); \
    mxmlSetErrorCallback(on_mxml_error); \
} while (0) \

#define END() do { \
    global_context = NULL; \
} while (0)

#define RETURN_XML_ERROR(CONTEXT) HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Xml error")

static hpd_error_t add(mxml_node_t *parent, const char *key, const char *val, hpd_module_t *context)
{
    mxmlElementSetAttr(parent, key, val);
    if (!mxmlElementGetAttr(parent, key)) RETURN_XML_ERROR(context);
    return HPD_E_SUCCESS;
}

static hpd_error_t add_attr(mxml_node_t *parent, hpd_pair_t *pair, hpd_module_t *context)
{
    hpd_error_t rc;

    // Get key and value
    const char *key, *val;
    if ((rc = hpd_pair_get(pair, &key, &val))) return rc;

    return add(parent, key, val, context);
}

static hpd_error_t add_parameter(mxml_node_t *parent, hpd_parameter_id_t *parameter, hpd_module_t *context)
{
    hpd_error_t rc;

    // Create node
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_REST_KEY_PARAMETER))) RETURN_XML_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_parameter_get_id(parameter, &id))) return rc;
    if ((rc = add(xml, HPD_REST_KEY_ID, id, context))) return rc;

    // Add attributes
    hpd_pair_t *pair;
    hpd_parameter_foreach_attr(rc, pair, parameter)
        if ((rc = add_attr(xml, pair, context))) return rc;
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

static hpd_error_t add_service(mxml_node_t *parent, hpd_service_id_t *service, hpd_rest_t *rest, hpd_module_t *context)
{
    hpd_error_t rc;

    // Create node
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_REST_KEY_SERVICE))) RETURN_XML_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_service_get_id(service, &id))) return rc;
    if ((rc = add(xml, HPD_REST_KEY_ID, id, context))) return rc;

    // Add url
    char *url;
    if ((rc = hpd_rest_url_create(rest, service, &url))) return rc;
    if ((rc = add(xml, HPD_REST_KEY_URI, url, context))) {
        free(url);
        return rc;
    }
    free(url);

    // Add actions
    hpd_action_t *action;
    hpd_service_foreach_action(rc, action, service) {
        hpd_method_t method;
        if ((rc = hpd_action_get_method(action, &method))) return rc;
        switch (method) {
            case HPD_M_NONE:break;
            case HPD_M_GET:
                if ((rc = add(xml, HPD_REST_KEY_GET, HPD_REST_VAL_TRUE, context))) return rc;
                break;
            case HPD_M_PUT:
                if ((rc = add(xml, HPD_REST_KEY_PUT, HPD_REST_VAL_TRUE, context))) return rc;
                break;
            case HPD_M_COUNT:break;
        }
    }
    if (rc) return rc;

    // Add attributes
    hpd_pair_t *pair;
    hpd_service_foreach_attr(rc, pair, service)
        if ((rc = add_attr(xml, pair, context))) return rc;
    if (rc) return rc;

    // Add parameters
    hpd_parameter_id_t *parameter;
    hpd_service_foreach_parameter(rc, parameter, service) {
        if ((rc = add_parameter(xml, parameter, context))) return rc;
    }
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

static hpd_error_t add_device(mxml_node_t *parent, hpd_device_id_t *device, hpd_rest_t *rest, hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_REST_KEY_DEVICE))) RETURN_XML_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_device_get_id(device, &id))) return rc;
    if ((rc = add(xml, HPD_REST_KEY_ID, id, context))) return rc;

    // Add attributes
    hpd_pair_t *pair;
    hpd_device_foreach_attr(rc, pair, device)
        if ((rc = add_attr(xml, pair, context))) return rc;
    if (rc) return rc;

    // Add services
    hpd_service_id_t *service;
    hpd_device_foreach_service(rc, service, device) {
        if ((rc = add_service(xml, service, rest, context))) return rc;
    }
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

static hpd_error_t add_adapter(mxml_node_t *parent, hpd_adapter_id_t *adapter, hpd_rest_t *rest, hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    mxml_node_t *json;
    if (!(json = mxmlNewElement(parent, HPD_REST_KEY_ADAPTER))) RETURN_XML_ERROR(context);

    // Add id
    const char *id;
    if ((rc = hpd_adapter_get_id(adapter, &id))) return rc;
    if ((rc = add(json, HPD_REST_KEY_ID, id, context))) return rc;

    // Add attributes
    hpd_pair_t *pair;
    hpd_adapter_foreach_attr(rc, pair, adapter) {
        if ((rc = add_attr(json, pair, context))) return rc;
    }
    if (rc) return rc;

    // Add devices
    hpd_device_id_t *device;
    hpd_adapter_foreach_device(rc, device, adapter) {
        if ((rc = add_device(json, device, rest, context))) return rc;
    }
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

static hpd_error_t add_configuration(mxml_node_t *parent, hpd_t *hpd, hpd_rest_t *rest, hpd_module_t *context)
{
    hpd_error_t rc;

    // Create object
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_REST_KEY_CONFIGURATION))) RETURN_XML_ERROR(context);

    // Add encoded charset
#ifdef CURL_ICONV_CODESET_OF_HOST
    curl_version_info_data *curl_ver = curl_version_info(CURLVERSION_NOW);
    if (curl_ver->features & CURL_VERSION_CONV && curl_ver->iconv_ver_num != 0)
        if ((rc = add(xml, HPD_REST_KEY_URL_ENCODED_CHARSET, CURL_ICONV_CODESET_OF_HOST, context))) return rc;
    else
        if ((rc = add(xml, HPD_REST_KEY_URL_ENCODED_CHARSET, HPD_REST_VAL_ASCII, context))) return rc;
#else
    if ((rc = add(xml, HPD_REST_KEY_URL_ENCODED_CHARSET, HPD_REST_VAL_ASCII, context))) return rc;
#endif

    // Add adapters
    hpd_adapter_id_t *adapter;
    hpd_foreach_adapter(rc, adapter, hpd) {
        if ((rc = add_adapter(xml, adapter, rest, context))) return rc;
    }
    if (rc) return rc;

    return HPD_E_SUCCESS;
}

hpd_error_t hpd_rest_xml_get_configuration(hpd_t *hpd, hpd_rest_t *rest, hpd_module_t *context, char **out)
{
    BEGIN(context);

    hpd_error_t rc;

    mxml_node_t *xml;
    if (!(xml = mxmlNewXML(XML_VERSION))) {
        END();
        RETURN_XML_ERROR(context);
    }

    if ((rc = add_configuration(xml, hpd, rest, context))) goto error;

    if (!((*out) = mxmlSaveAllocString(xml, MXML_NO_CALLBACK))) {
        mxmlDelete(xml);
        END();
        RETURN_XML_ERROR(context);
    }

    mxmlDelete(xml);
    END();
    return HPD_E_SUCCESS;

    error:
    mxmlDelete(xml);
    END();
    return rc;
}

static hpd_error_t add_value(mxml_node_t *parent, char *value, hpd_module_t *context)
{
    hpd_error_t rc;

    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_REST_KEY_VALUE))) RETURN_XML_ERROR(context);

    char timestamp[21];
    if ((rc = hpd_rest_get_timestamp(context, timestamp))) return rc;
    if ((rc = add(xml, HPD_REST_KEY_TIMESTAMP, timestamp, context))) return rc;

    if (!mxmlNewText(xml, 0, value)) RETURN_XML_ERROR(context);

    return HPD_E_SUCCESS;
}

hpd_error_t hpd_rest_xml_get_value(char *value, hpd_module_t *context, char **out)
{
    BEGIN(context);

    hpd_error_t rc;

    mxml_node_t *xml;
    if (!(xml = mxmlNewXML(XML_VERSION))) {
        END();
        RETURN_XML_ERROR(context);
    }

    if ((rc = add_value(xml, value, context))) goto error;

    if (!((*out) = mxmlSaveAllocString(xml, MXML_NO_CALLBACK))) {
        mxmlDelete(xml);
        END();
        RETURN_XML_ERROR(context);
    }

    mxmlDelete(xml);
    END();
    return HPD_E_SUCCESS;

    error:
    mxmlDelete(xml);
    END();
    return rc;
}

hpd_error_t hpd_rest_xml_parse_value(const char *in, hpd_module_t *context, char **out)
{
    BEGIN(context);

    mxml_node_t *xml;
    if (!(xml = mxmlLoadString(NULL, in, MXML_TEXT_CALLBACK))) {
        END();
        HPD_LOG_RETURN(context, HPD_E_ARGUMENT, "XML parsing error");
    }

    mxml_node_t *node;
    node = mxmlFindElement(xml, xml, HPD_REST_KEY_VALUE, NULL, NULL, MXML_DESCEND);
    if (!node) {
        mxmlDelete(xml);
        END();
        HPD_LOG_RETURN(context, HPD_E_ARGUMENT, "XML parsing error");
    }

    size_t len = 1;
    HPD_CALLOC(*out, len, char);
    for (node = node->child; node && node->type == MXML_TEXT; node = node->next) {
        int whitespaces = node->value.text.whitespace;
        char *string = node->value.text.string;
        HPD_REALLOC(*out, len + whitespaces + strlen(string), char);
        for (int i = 0; i < whitespaces; i++) strcat(*out, " ");
        strcat(*out, string);
    }

    mxmlDelete(xml);
    END();
    return HPD_E_SUCCESS;

    alloc_error:
        mxmlDelete(xml);
        END();
        HPD_LOG_RETURN_E_ALLOC(context);
}


