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
 * @file hpd_services.c
 * @brief  Methods for managing the Service structure
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include "hpd_device.h"
#include "hp_macros.h"

char *deviceGenerateServiceId(Device *device);

/**
 * Create the structure Device with all its parameters
 *
 * @param description The Device description
 *
 * @param ID The Device ID
 *
 * @param vendorID The ID of the vendor
 *
 * @param productID The ID of the product
 *
 * @param version The Device version
 *
 * @param IP The IP address of the Device
 *
 * @param port The port that the Device uses
 *
 * @param location The location of the Device
 *
 * @param type The Device type
 *
 * @param secure_device A variable that specifies
 *				     	if the Device is a secured
 *				     	or not secured service
 *				     	(HPD_SECURE_DEVICE or
 *				       	HPD_NON_SECURE_DEVICE)
 *
 * @return returns the Device or NULL if failed, note that
 * 		ID and type can not be NULL
 */
Device* 
deviceNew(
    const char *description,
    const char *vendorId,
    const char *productId,
    const char *version,
    const char *location,
    const char *type)
{
  Device *device;

  alloc_struct(device);

  device->id = NULL;

  null_ok_string_copy(device->description, description);
  null_ok_string_copy(device->vendorId, vendorId);
  null_ok_string_copy(device->productId, productId);
  null_ok_string_copy(device->version, version);
  null_ok_string_copy(device->location, location);
  null_nok_string_copy(device->type, type);

  device->service_head = NULL;

  return device;

cleanup:
  deviceFree(device);
  return NULL;
}

/**
 * Frees all the memory allocated for the Device. Note
 * that it only frees the memory used by the API, if the
 * user allocates memory for Devices attributes, he needs
 * to free it before/after calling this function. Also note
 * that the user can't destroy a Device that has Services
 * registered on the server.
 *
 * @param device_to_destroy The Device to destroy
 *
 * @return A HPD error code
 */
void
deviceFree( Device *device )
{

  if( device != NULL)
  {
    free_pointer(device->description);
    free_pointer(device->id);
    free_pointer(device->vendorId);
    free_pointer(device->productId);
    free_pointer(device->version);
    free_pointer(device->location);
    free_pointer(device->type);

    if( device->service_head )
    {
      ServiceElement *iterator = NULL, *tmp = NULL;
      DL_FOREACH_SAFE( device->service_head, iterator, tmp)
      {
	DL_DELETE( device->service_head, iterator );
	serviceElementFree(iterator);
      }
    }

    free(device);
  }
}

/**
 * Adds a Service to the Device
 *
 * @param service The Service to add
 *
 * @param device The Device 
 *
 * @return returns A HPD error code
 */
int 
deviceAddService( Device *device, Service *service )
{

  if( service == NULL || device == NULL ) 
    return HPD_E_NULL_POINTER;

  char *deviceId = NULL;
  
  deviceId = deviceGenerateServiceId( device );
  if( deviceId == NULL )
    return HPD_E_MALLOC_ERROR;

  serviceSetId( service, deviceId );

  ServiceElement *serviceElement;

  serviceElement = serviceElementNew( service );
  if( serviceElement == NULL )
  {
    serviceSetId( service, NULL );
    free(deviceId);
    return HPD_E_MALLOC_ERROR;
  }

  DL_APPEND( device->service_head, serviceElement);
  service->device = device;

  return HPD_E_SUCCESS;
}

static int removeService(Device *device, Service *service)
{
  if( service == NULL || device == NULL ) return HPD_E_NULL_POINTER;

  ServiceElement *iterator, *tmp;

  service->device = NULL;

  DL_FOREACH_SAFE( device->service_head, iterator, tmp )
  {
    if( strcmp( service->type, iterator->service->type ) == 0
	&& strcmp ( service->id, iterator->service->id ) == 0 )
    {
      DL_DELETE( device->service_head, iterator );
      serviceElementFree( iterator );
      iterator = NULL;
      return HPD_E_SUCCESS; 
    }			
  }

  return -1;
}

/**
 * Removes a Service from the Device
 *
 * @param service The Service to remove
 *
 * @param device The Device 
 *
 * @return returns A HPD error code
 */
int 
deviceRemoveService( Device *device, Service *service )
{
   return removeService(device, service);
}

int 
deviceRemoveService2( Service *service )
{
   Device *device = service->device;
   return removeService(device, service);
}

ServiceElement*
findServiceElement(Device *device, char *service_id)
{
  if(device == NULL || service_id == NULL ) return NULL;

  ServiceElement *iterator;

  DL_FOREACH( device->service_head, iterator )
  {
    if( strcmp ( service_id, iterator->service->id ) == 0 )
    {
      return iterator;
    }			
  }
  
  return NULL;
}

Service*
findService(Device *device, char *service_id)
{
  if(device == NULL || service_id == NULL ) return NULL;

  ServiceElement *serviceElement;

  if( ( serviceElement = findServiceElement(device, service_id) ) )
  {
    return serviceElement->service;
  }

  return NULL;

}

mxml_node_t*
deviceToXml(Device *device, mxml_node_t *parent)
{
  if(device == NULL) return NULL;

  mxml_node_t *deviceXml;

  deviceXml = mxmlNewElement(parent, "device");
  if(device->description != NULL) mxmlElementSetAttr(deviceXml, "desc", device->description);
  if(device->id != NULL) mxmlElementSetAttr(deviceXml, "id", device->id);
  if(device->vendorId != NULL) mxmlElementSetAttr(deviceXml, "vendorId", device->vendorId);
  if(device->productId != NULL) mxmlElementSetAttr(deviceXml, "productId", device->productId);
  if(device->version != NULL) mxmlElementSetAttr(deviceXml, "version", device->version);
  if(device->location != NULL) mxmlElementSetAttr(deviceXml, "location", device->location);
  if(device->type != NULL) mxmlElementSetAttr(deviceXml, "type", device->type);

  ServiceElement *iterator;

  DL_FOREACH( device->service_head, iterator )
  {
    serviceToXml(iterator->service, deviceXml);
  }

  return deviceXml;
}

json_t*
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

  ServiceElement *iterator;

  if( ( serviceArray = json_array() ) == NULL )
  {
    return NULL;
  }

  DL_FOREACH( device->service_head, iterator )
  {
    if( json_array_append_new(serviceArray, serviceToJson(iterator->service)) != 0 )
    {
      return NULL;
    }
  }

  if( json_object_set_new(deviceJson, "service", serviceArray) != 0 )
  {
    return NULL;
  }

  return deviceJson;
error:
  if(value) json_decref(value);
  if(serviceArray) json_decref(serviceArray);
  if(deviceJson) json_decref(deviceJson);
}

DeviceElement* 
deviceElementNew( Device *device )
{
  DeviceElement *to_create;

  if( !device )
    return NULL;

  to_create = (DeviceElement*)malloc(sizeof(DeviceElement));
  if( !to_create )
    return NULL;

  to_create->device = device;

  to_create->next = NULL;
  to_create->prev = NULL;

  return to_create;
}

void 
deviceElementFree( DeviceElement *deviceElement )
{
  free_pointer(deviceElement);
}

char*
deviceGenerateServiceId(Device *device)
{
  char *service_id = (char*)malloc((SERVICE_ID_SIZE+1)*sizeof(char));
  do{
    rand_str(service_id, SERVICE_ID_SIZE);
  }while(findService(device, service_id) != NULL);

  return service_id;
}

void
deviceSetId( Device *device, char *id )
{
  device->id = id;
}
