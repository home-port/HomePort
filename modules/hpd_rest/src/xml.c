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
#include "hpd_application_api.h"
#include "hpd_rest_intern.h"

static const char * const XML_VERSION = "1.0";

#define RETURN_XML_ERROR(CONTEXT) HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Xml error")

static hpd_error_t add_parameter(mxml_node_t *parent, hpd_parameter_id_t *parameter, hpd_module_t *context)
{
    hpd_error_t rc;

    // Create node
    mxml_node_t *xml;
    if (!(xml = mxmlNewElement(parent, HPD_REST_KEY_PARAMETER))) RETURN_XML_ERROR(context)

    // Add id
    const char *id;
    if ((rc = hpd_parameter_get_id(parameter, &id))) return rc;
    mxmlElementSetAttr(xml, HPD_REST_KEY_ID, id);

    // Add attributes
    hpd_pair_t *pair;
    hpd_parameter_foreach_attr(rc, pair, parameter) {
        const char *key, *val;
        hpd_pair_get(pair, &key, &val);
        mxmlElementSetAttr(xml, key, val);
    }

    return xml;
}

static hpd_error_t add_service(mxml_node_t *parent, hpd_service_id_t *service, hpd_rest_t *rest, hpd_module_t *context)
{
    hpd_error_t rc;
    hpd_pair_t *pair;
    mxml_node_t *xml = mxmlNewElement(parent, HPD_REST_KEY_SERVICE);

    const char *id;
    hpd_service_get_id(service, &id);
    mxmlElementSetAttr(xml, HPD_REST_KEY_ID, id);

    hpd_service_foreach_attr(rc, pair, service) {
        const char *key, *val;
        hpd_pair_get(pair, &key, &val);
        mxmlElementSetAttr(xml, key, val);
    }

    char *url;
    hpd_rest_url_create(rest, service, &url); // TODO error check
    if(url != NULL) {
        mxmlElementSetAttr(xml, HPD_REST_KEY_URI, url);
        free(url);
    }

    hpd_action_t *action;
    hpd_service_foreach_action(rc, action, service) {
        hpd_method_t method;
        hpd_action_get_method(action, &method);
        switch (method) {
            case HPD_M_NONE:break;
            case HPD_M_GET:
                mxmlElementSetAttr(xml, HPD_REST_KEY_GET, HPD_REST_VAL_TRUE);
                break;
            case HPD_M_PUT:
                mxmlElementSetAttr(xml, HPD_REST_KEY_PUT, HPD_REST_VAL_TRUE);
                break;
            case HPD_M_COUNT:break;
        }
    }

    hpd_parameter_id_t *parameter;
    hpd_service_foreach_parameter(rc, parameter, service)
        add_parameter(xml, parameter);

    return xml;
}

static hpd_error_t add_device(mxml_node_t *parent, hpd_device_id_t *device, hpd_rest_t *rest, hpd_module_t *context)
{
    if(device == NULL) return NULL;

    hpd_error_t rc;
    hpd_pair_t *pair;
    mxml_node_t *xml = mxmlNewElement(parent, HPD_REST_KEY_DEVICE);

    const char *id;
    hpd_device_get_id(device, &id);
    mxmlElementSetAttr(xml, HPD_REST_KEY_ID, id);

    hpd_device_foreach_attr(rc, pair, device) {
        const char *key, *val;
        hpd_pair_get(pair, &key, &val);
        mxmlElementSetAttr(xml, key, val);
    }

    hpd_service_id_t *iterator;
    hpd_device_foreach_service(rc, iterator, device)
    {
        add_service(xml, iterator, rest);
    }

    return xml;
}

static hpd_error_t add_adapter(mxml_node_t *parent, hpd_adapter_id_t *adapter, hpd_rest_t *rest, hpd_module_t *context)
{
    if(adapter == NULL) return NULL;

    hpd_error_t rc;
    hpd_pair_t *pair;
    mxml_node_t *xml = mxmlNewElement(parent, HPD_REST_KEY_ADAPTER);

    const char *id;
    hpd_adapter_get_id(adapter, &id);
    mxmlElementSetAttr(xml, HPD_REST_KEY_ID, id);

    hpd_adapter_foreach_attr(rc, pair, adapter) {
        const char *key, *val;
        hpd_pair_get(pair, &key, &val);
        mxmlElementSetAttr(xml, key, val);
    }

    hpd_device_id_t *iterator;
    hpd_adapter_foreach_device(rc, iterator, adapter)
    {
        add_device(xml, iterator, rest);
    }

    return xml;
}

static hpd_error_t add_configuration(mxml_node_t *parent, hpd_t *hpd, hpd_rest_t *rest, hpd_module_t *context)
{
    mxml_node_t *xml;

    xml = mxmlNewElement(parent, HPD_REST_KEY_CONFIGURATION);

#ifdef CURL_ICONV_CODESET_OF_HOST
    curl_version_info_data *curl_ver = curl_version_info(CURLVERSION_NOW);
  if (curl_ver->features & CURL_VERSION_CONV && curl_ver->iconv_ver_num != 0)
    mxmlElementSetAttr(xml, HPD_REST_KEY_URL_ENCODED_CHARSET, CURL_ICONV_CODESET_OF_HOST);
  else
    mxmlElementSetAttr(xml, HPD_REST_KEY_URL_ENCODED_CHARSET, HPD_REST_VAL_ASCII);
#else
    mxmlElementSetAttr(xml, HPD_REST_KEY_URL_ENCODED_CHARSET, HPD_REST_VAL_ASCII);
#endif

    hpd_error_t rc;
    hpd_adapter_id_t *iterator;
    hpd_foreach_adapter(rc, iterator, hpd)
    {
        add_adapter(xml, iterator, rest);
    }

    return xml;
}

// TODO Ugly hack to get context through mxml
static hpd_module_t *global_context = NULL;

static void on_mxml_error(const char *msg)
{

}

#define BEGIN(CONTEXT) do { \
    if (global_context) HPD_LOG_WARN((CONTEXT), "Global context is already set (trying anyways)..."); \
    global_context = (CONTEXT); \
    mxmlSetErrorCallback(on_mxml_error); \
} while (0) \

#define END(CODE) do { \
    global_context = NULL; \
    return (CODE); \
} while (0)

hpd_error_t hpd_rest_xml_get_configuration(hpd_t *hpd, hpd_rest_t *rest, hpd_module_t *context, char **out)
{
    BEGIN(context);

    char *res;
    mxml_node_t *xml = mxmlNewXML(XML_VERSION);
    add_configuration(xml, hpd, rest);
    res = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
    mxmlDelete(xml);

    END(HPD_E_SUCCESS);
}

hpd_error_t hpd_rest_xml_get_value(char *value, hpd_module_t *context, char **out)
{
    BEGIN(context);

    mxml_node_t *xml;
    mxml_node_t *stateXml;

    xml = mxmlNewXML(XML_VERSION);
    stateXml = mxmlNewElement(xml, HPD_REST_KEY_VALUE);
    mxmlElementSetAttr(stateXml, HPD_REST_KEY_TIMESTAMP, timestamp());
    mxmlNewText(stateXml, 0, value);

    char* return_value = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
    mxmlDelete(xml);

    END(HPD_E_SUCCESS);
}

hpd_error_t hpd_rest_xml_parse_value(const char *in, hpd_module_t *context, char **out)
{
    BEGIN(context);

    mxml_node_t *xml;
    mxml_node_t *node;

    xml = mxmlLoadString(NULL, in, MXML_TEXT_CALLBACK);
    if(xml == NULL)
    {
        printf("XML value format uncompatible with HomePort\n"); // TODO Wrong!
        return NULL;
    }

    node = mxmlFindElement(xml, xml, HPD_REST_KEY_VALUE, NULL, NULL, MXML_DESCEND);
    if(node == NULL || node-> child == NULL || node->child->value.text.string == NULL)
    {
        mxmlDelete(xml);
        printf("No \"value\" in the XML file\n"); // TODO Wrong!
        return NULL;
    }

    char *state = malloc(sizeof(char)*(strlen(node->child->value.text.string)+1));
    strcpy(state, node->child->value.text.string);

    mxmlDelete(xml);

    END(HPD_E_SUCCESS);
}


