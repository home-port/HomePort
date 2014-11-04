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

#include "dm_internal.h"
#include "hp_macros.h"
#include "hpd_error.h"
#include "utlist.h"
#include "idgen.h"

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
      Adapter *iterator, *tmp;
      DL_FOREACH_SAFE( config->adapter_head, iterator, tmp )
      {
	DL_DELETE(config->adapter_head, iterator);
   iterator->configuration = NULL;
      }
    }
    free_pointer(config);
  }
  
}

int
configurationAddAdapter(Configuration *configuration, Adapter *adapter)
{
  if(configuration == NULL || adapter == NULL) return HPD_E_NULL_POINTER;

  adapter->configuration = configuration;
  int stat = adapterGenerateId(adapter);
  if (stat) {
     adapter->configuration = NULL;
     return stat;
  }

  DL_APPEND( configuration->adapter_head, adapter);

  return HPD_E_SUCCESS;
}

int 
configurationRemoveAdapter(Adapter *adapter )
{
  if(adapter == NULL ) return HPD_E_NULL_POINTER;
  Configuration *configuration = adapter->configuration;
  if( configuration == NULL) return HPD_E_NULL_POINTER;

  DL_DELETE(configuration->adapter_head, adapter);
  adapter->configuration = NULL;

  return HPD_E_SUCCESS;
}


Adapter*
configurationFindAdapter(Configuration *configuration, char *adapter_id)
{
  if( configuration== NULL || adapter_id == NULL ) return NULL;

  Adapter *iterator = NULL;

  DL_FOREACH( configuration->adapter_head, iterator )
  {
    if( strcmp ( adapter_id, iterator->id ) == 0 )
    {
      return iterator;
    }			
  }
  
  return NULL;

}

mxml_node_t*
configurationToXml(Configuration *configuration, mxml_node_t *parent)
{
  mxml_node_t *configXml;

  configXml = mxmlNewElement(parent, "configuration");

  Adapter *iterator;

  DL_FOREACH(configuration->adapter_head, iterator)
  {
    adapterToXml(iterator, configXml);
  }

  return configXml;
}

json_t*
configurationToJson(Configuration *configuration)
{
  json_t *configJson=NULL;
  json_t *adapterArray=NULL;
  json_t *adapter=NULL;

  if( ( configJson = json_object() ) == NULL )
  {
    goto error;
  }

  Adapter *iterator;

  if( ( adapterArray = json_array() ) == NULL )
  {
    goto error;
  } 

  DL_FOREACH(configuration->adapter_head, iterator)
  {
    adapter = adapterToJson(iterator);
    if( ( adapter == NULL ) || ( json_array_append_new(adapterArray, adapter) != 0 ) )
    {
      goto error;
    }
  }

  if( json_object_set_new(configJson, "adapter", adapterArray) != 0 )
  {
    goto error;
  }

  return configJson;
error:
  if(adapter) json_decref(adapter);
  if(adapterArray) json_decref(adapterArray);
  if(configJson) json_decref(configJson);
  return NULL;
}


