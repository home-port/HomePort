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

// TODO Verify that keys do not overlap (use encoding?)

/**
 *  * Simple timestamp function
 *   *
 *    * @return Returns the timestamp
 *     */
static char *
timestamp ( void )
{
    time_t ltime;
    ltime = time(NULL);
    const struct tm *timeptr = localtime(&ltime);

    static char wday_name[7][3] = {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static char mon_name[12][3] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    static char result[25];

    sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d",
            wday_name[timeptr->tm_wday],
            mon_name[timeptr->tm_mon],
            timeptr->tm_mday, timeptr->tm_hour,
            timeptr->tm_min, timeptr->tm_sec,
            1900 + timeptr->tm_year);
    return result;
}

static mxml_node_t *
parameterToXml(hpd_parameter_id_t *parameter, mxml_node_t *parent)
{
    hpd_error_t rc;
    hpd_pair_t *pair;
    mxml_node_t *parameterXml = mxmlNewElement(parent, "parameter");

    const char *id;
    hpd_parameter_get_id(parameter, &id);
    mxmlElementSetAttr(parameterXml, "_id", id);

    hpd_parameter_foreach_attr(rc, pair, parameter) {
        const char *key, *val;
        hpd_pair_get(pair, &key, &val);
        mxmlElementSetAttr(parameterXml, key, val);
    }

    return parameterXml;
}

static mxml_node_t *
serviceToXml(hpd_service_id_t *service, mxml_node_t *parent)
{
    hpd_error_t rc;
    hpd_pair_t *pair;
    mxml_node_t *serviceXml = mxmlNewElement(parent, "service");

    const char *id;
    hpd_service_get_id(service, &id);
    mxmlElementSetAttr(serviceXml, "_id", id);

    hpd_service_foreach_attr(rc, pair, service) {
        const char *key, *val;
        hpd_pair_get(pair, &key, &val);
        mxmlElementSetAttr(serviceXml, key, val);
    }

    char *uri = hpd_rest_url_create(service);
    if(uri != NULL) {
        mxmlElementSetAttr(serviceXml, "_uri", uri);
        free(uri);
    }

    hpd_action_t *action;
    hpd_service_foreach_action(rc, action, service) {
        hpd_method_t method;
        hpd_action_get_method(action, &method);
        switch (method) {
            case HPD_M_NONE:break;
            case HPD_M_GET:
                mxmlElementSetAttr(serviceXml, "_get", "1");
                break;
            case HPD_M_PUT:
                mxmlElementSetAttr(serviceXml, "_put", "1");
                break;
            case HPD_M_COUNT:break;
        }
    }

    hpd_parameter_id_t *parameter;
    hpd_service_foreach_parameter(rc, parameter, service)
        parameterToXml(parameter, serviceXml);

    return serviceXml;
}

static mxml_node_t*
deviceToXml(hpd_device_id_t *device, mxml_node_t *parent)
{
    if(device == NULL) return NULL;

    hpd_error_t rc;
    hpd_pair_t *pair;
    mxml_node_t *deviceXml = mxmlNewElement(parent, "device");

    const char *id;
    hpd_device_get_id(device, &id);
    mxmlElementSetAttr(deviceXml, "_id", id);

    hpd_device_foreach_attr(rc, pair, device) {
        const char *key, *val;
        hpd_pair_get(pair, &key, &val);
        mxmlElementSetAttr(deviceXml, key, val);
    }

    hpd_service_id_t *iterator;
    hpd_device_foreach_service(rc, iterator, device)
    {
        serviceToXml(iterator, deviceXml);
    }

    return deviceXml;
}

static mxml_node_t*
adapterToXml(hpd_adapter_id_t *adapter, mxml_node_t *parent)
{
    if(adapter == NULL) return NULL;

    hpd_error_t rc;
    hpd_pair_t *pair;
    mxml_node_t *adapterXml = mxmlNewElement(parent, "adapter");

    const char *id;
    hpd_adapter_get_id(adapter, &id);
    mxmlElementSetAttr(adapterXml, "_id", id);

    hpd_adapter_foreach_attr(rc, pair, adapter) {
        const char *key, *val;
        hpd_pair_get(pair, &key, &val);
        mxmlElementSetAttr(adapterXml, key, val);
    }

    hpd_device_id_t *iterator;
    hpd_adapter_foreach_device(rc, iterator, adapter)
    {
        deviceToXml(iterator, adapterXml);
    }

    return adapterXml;
}

static mxml_node_t*
configurationToXml(hpd_t *hpd, mxml_node_t *parent)
{
    mxml_node_t *configXml;

    configXml = mxmlNewElement(parent, "configuration");

#ifdef CURL_ICONV_CODESET_OF_HOST
    curl_version_info_data *curl_ver = curl_version_info(CURLVERSION_NOW);
  if (curl_ver->features & CURL_VERSION_CONV && curl_ver->iconv_ver_num != 0)
    mxmlElementSetAttr(configXml, "urlEncodedCharset", CURL_ICONV_CODESET_OF_HOST);
  else
    mxmlElementSetAttr(configXml, "urlEncodedCharset", "ASCII");
#else
    mxmlElementSetAttr(configXml, "urlEncodedCharset", "ASCII");
#endif

    hpd_error_t rc;
    hpd_adapter_id_t *iterator;
    hpd_foreach_adapter(rc, iterator, hpd)
    {
        adapterToXml(iterator, configXml);
    }

    return configXml;
}

char *
xmlGetConfiguration(hpd_t *homeport)
{
    char *res;
    mxml_node_t *xml = mxmlNewXML("1.0");
    configurationToXml(homeport, xml);
    res = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
    mxmlDelete(xml);
    return res;
}

char*
xmlGetState(char *state)
{
    mxml_node_t *xml;
    mxml_node_t *stateXml;

    xml = mxmlNewXML("1.0");
    stateXml = mxmlNewElement(xml, "value");
    mxmlElementSetAttr(stateXml, "timestamp", timestamp());
    mxmlNewText(stateXml, 0, state);

    char* return_value = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
    mxmlDelete(xml);

    return return_value;
}

char*
xmlParseState(char *xml_value)
{
    mxml_node_t *xml;
    mxml_node_t *node;

    xml = mxmlLoadString(NULL, xml_value, MXML_TEXT_CALLBACK);
    if(xml == NULL)
    {
        printf("XML value format uncompatible with HomePort\n"); // TODO Wrong!
        return NULL;
    }

    node = mxmlFindElement(xml, xml, "value", NULL, NULL, MXML_DESCEND);
    if(node == NULL || node-> child == NULL || node->child->value.text.string == NULL)
    {
        mxmlDelete(xml);
        printf("No \"value\" in the XML file\n"); // TODO Wrong!
        return NULL;
    }

    char *state = malloc(sizeof(char)*(strlen(node->child->value.text.string)+1));
    strcpy(state, node->child->value.text.string);

    mxmlDelete(xml);

    return state;
}


