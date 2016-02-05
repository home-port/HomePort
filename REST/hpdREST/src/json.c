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

#include "json.h"
#include "datamanager.h"
#include <jansson.h>
#include "utlist.h"
#include "lr_interface.h"

static json_t*
serviceToJson(Service *service)
{
  json_t *serviceJson;
  json_t *value;

  if( ( serviceJson = json_object() ) == NULL )
  {
    return NULL;
  }
  if(service->description != NULL)
  {
    if( ( ( value = json_string(service->description) ) == NULL ) || ( json_object_set_new(serviceJson, "desc", value) != 0 ) )
    {
      return NULL;
    }
  }
  if(service->id != NULL)
  {
    if( ( ( value = json_string(service->id) ) == NULL ) || ( json_object_set_new(serviceJson, "id", value) != 0 ) )
    {
      return NULL;
    }
  }
  char *uri = lri_alloc_uri(service);
  if(uri != NULL)
  {
    if( ( ( value = json_string(uri) ) == NULL ) || ( json_object_set_new(serviceJson, "uri", value) != 0 ) )
    {
      free(uri);
      return NULL;
    }
    free(uri);
  }
  if( ( ( value = json_string( service->isActuator ? "1" : "0") ) == NULL ) || (json_object_set_new(serviceJson, "isActuator", value) != 0 ) )
  {
    return NULL;
  }
  if(service->type != NULL)
  {
    if( ( ( value = json_string( service->type ) ) == NULL ) || ( json_object_set_new(serviceJson, "type", value) != 0 ) )
    {
      return NULL;
    }
  }
  if(service->unit != NULL)
  {
    if( ( ( value = json_string( service->unit ) ) == NULL ) || ( json_object_set_new(serviceJson, "unit", value) != 0 ) )
    {
      return NULL;
    }
  }


  if(service->parameter != NULL)
  {
    json_t *parameterJson = json_object();
    if( parameterJson == NULL )
      return NULL;
    if(service->parameter->max != NULL)
    {
      if( ( ( value = json_string( service->parameter->max ) ) == NULL ) || ( json_object_set_new(parameterJson, "max", value) != 0 ) )
      {
        return NULL;
      }
    }
    if(service->parameter->min != NULL)
    {
      if( ( ( value = json_string( service->parameter->min ) ) == NULL ) || ( json_object_set_new(parameterJson, "min", value) != 0 ) )
      {
        return NULL;
      }
    }
    if(service->parameter->scale != NULL)
    {
      if( ( ( value = json_string( service->parameter->scale ) ) == NULL ) || ( json_object_set_new(parameterJson, "scale", value) != 0 ) )
      {
        return NULL;
      }
    }
    if(service->parameter->step != NULL)
    {
      if( ( ( value = json_string( service->parameter->step ) ) == NULL ) || ( json_object_set_new(parameterJson, "step", value) != 0 ) )
      {
        return NULL;
      }
    }
    if(service->parameter->type != NULL)
    {
      if( ( (value = json_string( service->parameter->type ) ) == NULL ) || ( json_object_set_new(parameterJson, "type", value) != 0 ) )
      {
        return NULL;
      }
    }
    if(service->parameter->unit != NULL)
    {
      if( ( ( value = json_string( service->parameter->unit ) ) == NULL ) || ( json_object_set_new(parameterJson, "unit", value) != 0 ) )
      {
        return NULL;
      }
    }
    if(service->parameter->values != NULL)
    {
      if( ( ( value = json_string( service->parameter->values ) ) == NULL ) || ( json_object_set_new(parameterJson, "values", value) != 0 ) )
      {
        return NULL;
      }
    }
    if( json_object_set_new(serviceJson, "parameter", parameterJson) != 0 )
    {
      return NULL;
    }
  }
  return serviceJson;
}

static json_t*
deviceToJson(Device *device)
{
  json_t *deviceJson=NULL;
  json_t *value=NULL;
  json_t *serviceArray=NULL;

  if( ( deviceJson = json_object() ) == NULL )
  {
    return NULL;
  }
  if(device->description != NULL)
  {
    if( ( ( value = json_string(device->description) ) == NULL ) || ( json_object_set_new(deviceJson, "desc", value) != 0 ) )
    {
      return NULL;
    }
  }
  if(device->id != NULL)
  {
    if( ( ( value = json_string(device->id) ) == NULL ) || ( json_object_set_new(deviceJson, "id", value) != 0 ) )
    {
      return NULL;
    }
  }
  if(device->vendorId != NULL)
  {
    if( ( ( value = json_string(device->vendorId) ) == NULL ) || ( json_object_set_new(deviceJson, "vendorId", value) != 0 ) )
    {
      return NULL;
    }
  }
  if(device->productId != NULL)
  {
    if( ( ( value = json_string( device->productId ) ) == NULL ) || ( json_object_set_new(deviceJson, "productId", value) != 0 ) )
    {
      return NULL;
    }
  }
  if(device->version != NULL)
  {
    if( ( ( value = json_string( device->version ) ) == NULL ) || ( json_object_set_new(deviceJson, "version", value) != 0 ) )
    {
      return NULL;
    }
  }
  if(device->productId != NULL)
  {
    if( ( ( value = json_string( device->productId ) ) == NULL ) || ( json_object_set_new(deviceJson, "productId", value) != 0 ) )
    {
      return NULL;
    }
  }
  if(device->type != NULL)
  {
    if( ( ( value = json_string( device->type ) ) == NULL ) || ( json_object_set_new(deviceJson, "type", value) != 0 ) )
    {
      return NULL;
    }
  }

  Service *iterator;

  if( ( serviceArray = json_array() ) == NULL )
  {
    return NULL;
  }

  DL_FOREACH( device->service_head, iterator )
  {
    if( json_array_append_new(serviceArray, serviceToJson(iterator)) != 0 )
    {
      return NULL;
    }
  }

  if( json_object_set_new(deviceJson, "service", serviceArray) != 0 )
  {
    return NULL;
  }

  return deviceJson;
//error:
//  if(value) json_decref(value);
//  if(serviceArray) json_decref(serviceArray);
//  if(deviceJson) json_decref(deviceJson);
}

static json_t*
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

static json_t*
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

char *
jsonGetConfiguration(HomePort *homeport)
{
   char *res;
   json_t * configurationJson = configurationToJson( homeport->configuration );
   res = json_dumps( configurationJson, 0 );
   json_decref(configurationJson);
   return res;
}

char*
jsonGetState(char *state)
{
  json_t *json=NULL;
  json_t *value=NULL;

  if( ( json = json_object() ) == NULL )
  {
    goto error;
  }
  if( ( ( value = json_string(state) ) == NULL ) || ( json_object_set_new(json, "value", value) != 0 ) )
  {
    goto error;
  }

  char *ret = json_dumps( json, 0 );
  json_decref(json);
  return ret;

error:
  if(value) json_decref(value);
  if(json) json_decref(json);
  return NULL;
}

const char*
jsonParseState(char *json_value)
{
  json_t *json = NULL;
  json_error_t *error=NULL;
  json_t *value = NULL;

  if( ( json = json_loads(json_value, 0, error) ) == NULL )
  {
    goto error;
  }

  if( json_is_object(json) == 0 )
  {
    goto error;
  }

  if( ( value = json_object_get(json, "value") ) == NULL )
  {
    goto error;
  }

  const char *ret = json_string_value(value);
  char *ret_alloc = malloc((strlen(ret)+1)*sizeof(char));
  strcpy(ret_alloc, ret);

  json_decref(json);

  return ret_alloc;

error:
  return NULL;
}


