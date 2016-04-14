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

/**
 * @file hpd_configuration.h
 * @brief  Methods for managing the configuration structure
 * @author Thibaut Le Guilly
 */

#include "datamanager.h"
#include "hp_macros.h"
#include "hpd_error.h"
#include "utlist.h"

configuration_t*
configurationNew()
{
  configuration_t *config;
  alloc_struct(config);

  return config; 
cleanup:
  return NULL;
}


void
configurationFree(configuration_t *config)
{
  if( config != NULL )
  {
    if( config->adapter_head != NULL )
    {
      adapter_t *iterator, *tmp;
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
configurationAddadapter_t(configuration_t *configuration, adapter_t *adapter)
{
  if(configuration == NULL || adapter == NULL) return HPD_E_NULL_POINTER;

    if (configurationFindAdapter(configuration, adapter->id))
        return HPD_E_ID_NOT_UNIQUE;

  adapter->configuration = configuration;

  DL_APPEND( configuration->adapter_head, adapter);

  return HPD_E_SUCCESS;
}

int 
configurationRemoveadapter_t(adapter_t *adapter )
{
  if(adapter == NULL ) return HPD_E_NULL_POINTER;
  configuration_t *configuration = adapter->configuration;
  if( configuration == NULL) return HPD_E_NULL_POINTER;

  DL_DELETE(configuration->adapter_head, adapter);
  adapter->configuration = NULL;

  return HPD_E_SUCCESS;
}


adapter_t*
configurationFindFirstadapter_t(configuration_t *configuration,
      const char *id,
      const char *network)
{
  if( configuration== NULL ) return NULL;

  adapter_t *iterator = NULL;

  DL_FOREACH( configuration->adapter_head, iterator )
  {
    if ( id == NULL || (iterator->id != NULL && strcmp(id, iterator->id) == 0) )
      if ( network == NULL || (iterator->network != NULL && strcmp(network, iterator->network) == 0) )
        return iterator;
  }
  
  return NULL;
}

service_t *configurationServiceLookup(configuration_t *configuration, const char *aid, const char *did, const char *sid)
{
    if( configuration== NULL ) return NULL;

    adapter_t *adapter = configurationFindAdapter(configuration, aid);
    if (adapter == NULL)
        return NULL;

    return adapterServiceLookup(adapter, did, sid);
}

int
configurationAddListener(configuration_t *configuration, listener_t *l)
{
   if( configuration == NULL || l == NULL ) 
      return HPD_E_NULL_POINTER;
   
   DL_APPEND( configuration->listener_head, l);
   return HPD_E_SUCCESS;
}

int 
configurationRemoveListener(configuration_t *configuration, listener_t *l)
{
   if( configuration == NULL || l == NULL ) return HPD_E_NULL_POINTER;
   DL_DELETE( configuration->listener_head, l );
   return HPD_E_SUCCESS; 
}


