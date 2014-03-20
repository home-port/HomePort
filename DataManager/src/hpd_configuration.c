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
 * @file hpd_configuration.h
 * @brief  Methods for managing the configuration structure
 * @author Thibaut Le Guilly
 */


#include "hpd_configuration.h"
#include "hp_macros.h"

char *confGenerateAdapterId(Configuration *conf);
AdapterElement* findAdapterElement(Configuration *configuration, char *adapter_id);

Configuration*
configurationNew()
{
  Configuration *config;
  alloc_struct(config);

  return config; 
cleanup:
  return NULL;
}

void
configurationFree(Configuration *config)
{
  if( config != NULL )
  {
    if( config->adapter_head != NULL )
    {
      AdapterElement *iterator, *tmp;
      DL_FOREACH_SAFE( config->adapter_head, iterator, tmp )
      {
	DL_DELETE(config->adapter_head, iterator);
	adapterElementFree(iterator);
      }
    }
    free_pointer(config);
  }
  
}

int
configurationAddAdapter(Configuration *configuration, Adapter *adapter)
{
  if(configuration == NULL || adapter == NULL) return HPD_E_NULL_POINTER;

  char *adapterId = confGenerateAdapterId( configuration );
  if( adapterId == NULL )
    return HPD_E_NULL_POINTER;

  adapterSetId( adapter, adapterId );

  AdapterElement *adapterElement = NULL;

  adapterElement = adapterElementNew( adapter );
  if( adapterElement == NULL )
  {
    adapterSetId( adapter, NULL );
    free(adapterId);
    return HPD_E_NULL_POINTER;
  }

  DL_APPEND( configuration->adapter_head, adapterElement);

  return HPD_E_SUCCESS;
}

int 
configurationRemoveAdapter( Configuration *configuration, Adapter *adapter )
{

  if( configuration == NULL || adapter == NULL ) return HPD_E_NULL_POINTER;

  AdapterElement *adapterElement = findAdapterElement(configuration, adapter->id);
  if(adapterElement == NULL)
  {
    return -1;
  }

  DL_DELETE(configuration->adapter_head, adapterElement);
  adapterElementFree(adapterElement);

  return HPD_E_SUCCESS;
}


AdapterElement*
findAdapterElement(Configuration *configuration, char *adapter_id)
{
  if( configuration== NULL || adapter_id == NULL ) return NULL;

  AdapterElement *iterator = NULL;

  DL_FOREACH( configuration->adapter_head, iterator )
  {
    if( strcmp ( adapter_id, iterator->adapter->id ) == 0 )
    {
      return iterator;
    }			
  }
  
  return NULL;
}

Adapter*
findAdapter(Configuration *configuration, char *adapter_id)
{
  if(configuration == NULL || adapter_id == NULL ) return NULL;

  AdapterElement *adapterElement;

  if( ( adapterElement = findAdapterElement(configuration, adapter_id) ) )
  {
    return adapterElement->adapter;
  }

  return NULL;

}

mxml_node_t*
configurationToXml(Configuration *configuration, mxml_node_t *parent)
{
  mxml_node_t *configXml;

  configXml = mxmlNewElement(parent, "configuration");

  AdapterElement *iterator;

  DL_FOREACH(configuration->adapter_head, iterator)
  {
    adapterToXml(iterator->adapter, configXml);
  }

  return configXml;
}

char *confGenerateAdapterId(Configuration *conf)
{
  char *adapter_id = (char*)malloc((ADAPTER_ID_SIZE+1)*sizeof(char));
  if( adapter_id == NULL )
    return NULL;
  do{
    rand_str(adapter_id, ADAPTER_ID_SIZE);
  }while(findAdapter(conf, adapter_id) != NULL);

  return adapter_id;
}

int
confDeviceIdExists(Configuration *conf, char *deviceId)
{
  AdapterElement *iterator = NULL;
  DL_FOREACH( conf->adapter_head, iterator )
  {
   if(findDevice(iterator->adapter, deviceId) != NULL)
   {
     return 1;
   }
  }
  return 0;
}

char *confGenerateDeviceId(Configuration *conf)
{
  char *deviceId = malloc((DEVICE_ID_SIZE+1)*sizeof(char));
  if( deviceId == NULL )
    return NULL;
  do{
    rand_str(deviceId, DEVICE_ID_SIZE);
  }while(confDeviceIdExists(conf, deviceId) != 0);

  return deviceId;
}
