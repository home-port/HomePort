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

#include "hpd_adapter.h"
#include "hpd_configuration.h"
#include "hpd_device.h"
#include "hp_macros.h"
#include "hpd_error.h"
#include "utlist.h"
#include "idgen.h"

#define ADAPTER_ID_SIZE 2

Adapter*
adapterNew( const char *network, void *data )
{
  Adapter * adapter;

  alloc_struct(adapter);

  adapter->id = NULL;

  null_ok_string_copy(adapter->network, network);

  adapter->data = data;

  adapter->device_head = NULL;
  adapter->configuration = NULL;

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
    free_pointer(adapter->network);
    free_pointer(adapter->id);

    Device *tmp=NULL, *iterator=NULL;

    DL_FOREACH_SAFE( adapter->device_head, iterator, tmp )
    {
      DL_DELETE( adapter->device_head, iterator );
      iterator->adapter = NULL;
    }

    free(adapter);
  }
}


int
adapterAddDevice(Adapter *adapter, Device *device)
{
  if(adapter == NULL || device == NULL) return HPD_E_NULL_POINTER;

  device->adapter = adapter;
  int stat = deviceGenerateId(device);
  if (stat) {
     device->adapter = NULL;
     return stat;
  }

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
adapterFindDevice(Adapter *adapter, char *device_id)
{
  if( adapter== NULL || device_id == NULL ) return NULL;

  Device *iterator=NULL;

  DL_FOREACH( adapter->device_head, iterator )
  {
    if( strcmp ( device_id, iterator->id ) == 0 )
    {
      return iterator;
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
    json_t *device;
    if( ( ( device = deviceToJson(iterator) ) == NULL ) || ( json_array_append_new(deviceArray, device) != 0 ) )
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

