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

/**
 * @file hpd_adapter.h
 * @brief  Methods for managing the Service structure
 * @author Thibaut Le Guilly
 */

#include "dm_internal.h"
#include "hp_macros.h"
#include "hpd_error.h"
#include "utlist.h"
#include "idgen.h"

#define ADAPTER_ID_SIZE 2

Adapter*
adapterNew(Configuration *configuration, const char *network, void *data, free_f free_data )
{
  Adapter * adapter;

  alloc_struct(adapter);

  adapter->id = NULL;

  null_ok_string_copy(adapter->network, network);

  adapter->data = data;
  adapter->free_data = free_data;

  adapter->device_head = NULL;
  adapter->configuration = NULL;

  configurationAddAdapter(configuration, adapter);

  return adapter;

cleanup:
  adapterFree(adapter);
  return NULL;
}

void
adapterFree(Adapter *adapter)
{
  if( adapter != NULL )
  {
    configurationRemoveAdapter(adapter);
    free_pointer(adapter->network);
    free_pointer(adapter->id);

    Device *tmp=NULL, *iterator=NULL;

    if (adapter->device_head) {
      DL_FOREACH_SAFE( adapter->device_head, iterator, tmp )
      {
         deviceFree(iterator);
      }
    }

    if (adapter->free_data) adapter->free_data(adapter->data);
    free(adapter);
  }
}


int
adapterAddDevice(Adapter *adapter, Device *device)
{
  if(adapter == NULL || device == NULL) return HPD_E_NULL_POINTER;

  device->adapter = adapter;
  DL_APPEND( adapter->device_head, device);

  return HPD_E_SUCCESS;
}

int 
adapterRemoveDevice(Device *device)
{
   Adapter *adapter = device->adapter;

  if( adapter == NULL || device == NULL ) return HPD_E_NULL_POINTER;

  DL_DELETE(adapter->device_head, device);
  device->adapter = NULL;

  return HPD_E_SUCCESS;
}

Device*
adapterFindFirstDevice(Adapter *adapter,
      const char *description,
      const char *id,
      const char *vendorId,
      const char *productId,
      const char *version,
      const char *location,
      const char *type)
{
  if( adapter== NULL ) return NULL;

  Device *iterator=NULL;

  DL_FOREACH( adapter->device_head, iterator )
  {
    if ( description == NULL || (iterator->description != NULL && strcmp(description, iterator->description) == 0) )
      if ( id == NULL || (iterator->id != NULL && strcmp(id, iterator->id) == 0) )
        if ( vendorId == NULL || (iterator->vendorId != NULL && strcmp(vendorId, iterator->vendorId) == 0) )
          if ( productId == NULL || (iterator->productId != NULL && strcmp(productId, iterator->productId) == 0) )
            if ( version == NULL || (iterator->version != NULL && strcmp(version, iterator->version) == 0) )
              if ( location == NULL || (iterator->location != NULL && strcmp(location, iterator->location) == 0) )
                if ( type == NULL || (iterator->type != NULL && strcmp(type, iterator->type) == 0) )
                  return iterator;
  }
  
  return NULL;
}

Service *adapterServiceLookup(Adapter *adapter, const char *dtype, const char *did, const char *stype, const char *sid)
{
  if( adapter == NULL ) return NULL;

  Device *device = NULL;
  Service *service = NULL;

  DL_FOREACH(adapter->device_head, device)
  {
    if (strcmp(device->type, dtype) == 0 && strcmp(device->id, did) == 0) {
      service = deviceFindFirstService(device, NULL, NULL, stype, NULL, sid, NULL);
      if (service) return service;
    }
  }

  return NULL;
}

mxml_node_t*
adapterToXml(Adapter *adapter, mxml_node_t *parent)
{
  if(adapter == NULL) return NULL;

  mxml_node_t *adapterXml;

  adapterXml = mxmlNewElement(parent, "adapter");
  if(adapter->id != NULL) mxmlElementSetAttr(adapterXml, "id", adapter->id);
  if(adapter->network != NULL) mxmlElementSetAttr(adapterXml, "network", adapter->network);

  Device *iterator;

  DL_FOREACH( adapter->device_head, iterator)
  {
     if (iterator->attached)
        deviceToXml(iterator, adapterXml);
  }

  return adapterXml;
}

json_t*
adapterToJson(Adapter *adapter)
{
  json_t *adapterJson=NULL;
  json_t *value=NULL;
  json_t *deviceArray=NULL;

  if( ( adapterJson = json_object() ) == NULL )
  {
    goto error;
  }
  if(adapter->id != NULL)
  {
    if( ( ( value = json_string(adapter->id) ) == NULL ) || ( json_object_set_new(adapterJson, "id", value) != 0 ) )
    {
      goto error;
    }
  }
  if(adapter->network != NULL)
  {
    if( ( ( value = json_string(adapter->network) ) == NULL ) || ( json_object_set_new(adapterJson, "network", value) != 0 ) )
    {
      goto error;
    }
  }

  Device *iterator;

  if( ( deviceArray = json_array() ) == NULL )
  {
    goto error;
  }

  DL_FOREACH( adapter->device_head, iterator )
  {
     if (iterator->attached) {
        json_t *device;
        if( ( ( device = deviceToJson(iterator) ) == NULL ) || ( json_array_append_new(deviceArray, device) != 0 ) )
        {
          goto error;
        }
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

int adapterGenerateId(Adapter *adapter)
{
   Configuration *conf = adapter->configuration;
  char *adapter_id = (char*)malloc((ADAPTER_ID_SIZE+1)*sizeof(char));
   if (!adapter_id) return HPD_E_MALLOC_ERROR;
  do{
    rand_str(adapter_id, ADAPTER_ID_SIZE);
  }while(configurationFindAdapter(conf, adapter_id) != NULL);

  adapter->id = adapter_id;
  return HPD_E_SUCCESS;
}

