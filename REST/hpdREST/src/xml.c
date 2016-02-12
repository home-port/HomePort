/*Copyright 2011 Aalborg University. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of
  conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list
  of conditions and the following disclaimer in the documentation and/or other materials
  provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed*/

#include "xml.h"
#include "datamanager.h"
#include <time.h>
#include <mxml.h>
#include <curl/curl.h>
#include "utlist.h"
#include "lr_interface.h"

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
serviceToXml(Service *service, mxml_node_t *parent)
{
  mxml_node_t *serviceXml;

  serviceXml = mxmlNewElement(parent, "service");
  if(service->description != NULL) mxmlElementSetAttr(serviceXml, "desc", service->description);
  char *sid = lri_url_encode(service->id);
  if(sid != NULL) mxmlElementSetAttr(serviceXml, "id", sid);
  free(sid);
  char *uri = lri_alloc_uri(service);
  if(uri != NULL) {
    mxmlElementSetAttr(serviceXml, "uri", uri);
    free(uri);
  }
  mxmlElementSetAttr(serviceXml, "isActuator", service->isActuator ? "1" : "0");
  char *stype = lri_url_encode(service->type);
  if(stype != NULL) mxmlElementSetAttr(serviceXml, "type", stype);
  free(stype);
  if(service->unit != NULL) mxmlElementSetAttr(serviceXml, "unit", service->unit);


  if(service->parameter != NULL)
  {
    mxml_node_t *parameterXml = mxmlNewElement(serviceXml, "parameter");
    if(service->parameter->max != NULL) mxmlElementSetAttr(parameterXml, "max", service->parameter->max);
    if(service->parameter->min != NULL) mxmlElementSetAttr(parameterXml, "min", service->parameter->min);
    if(service->parameter->scale != NULL) mxmlElementSetAttr(parameterXml, "scale", service->parameter->scale);
    if(service->parameter->step != NULL) mxmlElementSetAttr(parameterXml, "step", service->parameter->step);
    if(service->parameter->type != NULL) mxmlElementSetAttr(parameterXml, "type", service->parameter->type);
    if(service->parameter->unit != NULL) mxmlElementSetAttr(parameterXml, "unit", service->parameter->unit);
    if(service->parameter->values != NULL) mxmlElementSetAttr(parameterXml, "values", service->parameter->values);
  }


  return serviceXml;
}

static mxml_node_t*
deviceToXml(Device *device, mxml_node_t *parent)
{
  if(device == NULL) return NULL;

  mxml_node_t *deviceXml;

  deviceXml = mxmlNewElement(parent, "device");
  if(device->description != NULL) mxmlElementSetAttr(deviceXml, "desc", device->description);
  char *did = lri_url_encode(device->id);
  if(did != NULL) mxmlElementSetAttr(deviceXml, "id", did);
  free(did);
  if(device->vendorId != NULL) mxmlElementSetAttr(deviceXml, "vendorId", device->vendorId);
  if(device->productId != NULL) mxmlElementSetAttr(deviceXml, "productId", device->productId);
  if(device->version != NULL) mxmlElementSetAttr(deviceXml, "version", device->version);
  if(device->location != NULL) mxmlElementSetAttr(deviceXml, "location", device->location);
  char *dtype = lri_url_encode(device->type);
  if(dtype != NULL) mxmlElementSetAttr(deviceXml, "type", dtype);
  free(dtype);

  Service *iterator;

  DL_FOREACH( device->service_head, iterator )
  {
    serviceToXml(iterator, deviceXml);
  }

  return deviceXml;
}

static mxml_node_t*
adapterToXml(Adapter *adapter, mxml_node_t *parent)
{
  if(adapter == NULL) return NULL;

  mxml_node_t *adapterXml;

  adapterXml = mxmlNewElement(parent, "adapter");
  char *aid = lri_url_encode(adapter->id);
  if(aid != NULL) mxmlElementSetAttr(adapterXml, "id", aid);
  free(aid);
  if(adapter->network != NULL) mxmlElementSetAttr(adapterXml, "network", adapter->network);

  Device *iterator;

  DL_FOREACH( adapter->device_head, iterator)
  {
    if (iterator->attached)
      deviceToXml(iterator, adapterXml);
  }

  return adapterXml;
}

static mxml_node_t*
configurationToXml(Configuration *configuration, mxml_node_t *parent)
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

  Adapter *iterator;

  DL_FOREACH(configuration->adapter_head, iterator)
  {
    adapterToXml(iterator, configXml);
  }

  return configXml;
}

char *
xmlGetConfiguration(HomePort *homeport)
{
   char *res;
   mxml_node_t *xml = mxmlNewXML("1.0");
   configurationToXml(homeport->configuration, xml);
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
    printf("XML value format uncompatible with HomePort\n");
    return NULL;
  }

  node = mxmlFindElement(xml, xml, "value", NULL, NULL, MXML_DESCEND);
  if(node == NULL || node-> child == NULL || node->child->value.text.string == NULL)
  {
    mxmlDelete(xml);
    printf("No \"value\" in the XML file\n");
    return NULL;
  }

  char *state = malloc(sizeof(char)*(strlen(node->child->value.text.string)+1));
  strcpy(state, node->child->value.text.string);

  mxmlDelete(xml);

  return state;
}


