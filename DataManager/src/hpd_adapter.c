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

Adapter*
create_adapter_struct( char *id, char *network, void *data )
{
  Adapter * adapter;

  if(id == NULL)
    return NULL;
  adapter = (Adapter*)malloc(sizeof(Adapter));

  adapter->id = (char*)malloc(sizeof(char)*(strlen(id)+1));
  if(adapter->id == NULL)
  {
    free(adapter);
    return NULL;
  }

  strcpy(adapter->id, id);

  if(network != NULL)
  {
    adapter->network = (char*)malloc(sizeof(char)*(strlen(network)+1));
    if(adapter->network == NULL)
    {
      free(adapter->id);
      free(adapter);
      return NULL;
    }
    strcpy(adapter->network, network);
  }

  adapter->data = data;

  return adapter;
}

int
destroy_adapter_struct(Adapter *adapter)
{
  if(adapter == NULL)
    return HPD_E_NULL_POINTER;

  DeviceElement *tmp, *iterator;

  DL_FOREACH_SAFE( adapter->device_head, iterator, tmp )
  {
    DL_DELETE( adapter->device_head, iterator );
    destroy_device_struct( iterator->device );
    destroy_device_element_struct( iterator );
  }

  if(adapter->network != NULL)
    free(adapter->network);

  if(adapter->id != NULL)
    free(adapter->id);

  return HPD_E_SUCCESS;
}


int
adapter_add_device(Adapter *adapter, Device *device)
{
  if(adapter == NULL || device == NULL) return HPD_E_NULL_POINTER;

  DeviceElement *new_se;

  if( findDevice(device, device->ID) )
  {
    return HPD_E_SERVICE_ALREADY_IN_LIST;
  }

  new_se = create_device_element_struct( device );
  if( !new_se )
    return HPD_E_NULL_POINTER;

  DL_APPEND( device->device_head, new_se);

  return HPD_E_SUCCESS;
}

int 
adapter_remove_device( Adapter *adapter, Device *device )
{

  if( device == NULL || device == NULL ) return HPD_E_NULL_POINTER;

  DeviceElement *toDelete;

  DL_FOREACH_SAFE( device->device_head, iterator, tmp )
  {
    if( strcmp( device->type, iterator->device->type ) == 0
	&& strcmp ( device->ID, iterator->device->ID ) == 0 )
    {
      DL_DELETE( device->device->device_head, iterator );
      destroy_device_element_struct( iterator );
      iterator = NULL;
      break;
    }			
  }

  return HPD_E_SUCCESS;
}

DeviceElement*
findDeviceElement(Device, char *device_id)
{
  if(device == NULL || device_id == NULL ) return HPD_E_NULL_POINTER;

  DeviceElement *iterator;

  DL_FOREACH( device->device_head, iterator )
  {
    if( strcmp ( device_id, iterator->device->ID ) == 0 )
    {
      return iterator;
    }			
  }
  
  return NULL;
}

Device*
findDevice(Adapter *Adapter, char *device_id)
{
  if(adapter == NULL || device_id == NULL ) return HPD_E_NULL_POINTER;

  DeviceElement *deviceElement;

  if( deviceElement = findDeviceElement(device, device_id) )
  {
    return deviceElement->device;
  }

  return NULL;

}

mxml_node_t*
adapterToXml(Adapter *adapter)
{
  if(adapter == NULL) return HPD_E_NULL_POINTER;

  mxml_node_t *adapterXml;

  adapterXml = mxmlNewElement(MXML_NO_PARENT, "adapter");
  if(adapter->id != NULL) mxmlElementSetAttr(adapterXml, "id", adapter->id);
  if(adapter->network != NULL) mxmlElementSetAttr(adapterXml, "network", adapter->network);

  return adapterXml;
}
