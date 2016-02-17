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

#include "dm_internal.h"
#include "hp_macros.h"
#include "hpd_error.h"
#include "utlist.h"
#include "idgen.h"


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
int deviceNew(Device** device, Adapter *adapter,
              const char *id, const char *description, const char *vendorId, const char *productId,
              const char *version, const char *location, const char *type,
              void *data, free_f free_data)
{
    if (adapterFindDevice(adapter, id) != NULL)
        return HPD_E_ID_NOT_UNIQUE;

    alloc_struct(*device);

    (*device)->attached = 0;
    (*device)->id = NULL;

    null_nok_string_copy((*device)->id, id);
    null_ok_string_copy((*device)->description, description);
    null_ok_string_copy((*device)->vendorId, vendorId);
    null_ok_string_copy((*device)->productId, productId);
    null_ok_string_copy((*device)->version, version);
    null_ok_string_copy((*device)->location, location);
    null_nok_string_copy((*device)->type, type);

    (*device)->service_head = NULL;
    (*device)->adapter = NULL;

    (*device)->data = data;
    (*device)->free_data = free_data;

    adapterAddDevice(adapter, *device);

    return HPD_E_SUCCESS;

    cleanup:
    deviceFree(*device);
    return HPD_E_ALLOC_ERROR;
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
     adapterRemoveDevice(device);
    free_pointer(device->description);
    free_pointer(device->id);
    free_pointer(device->vendorId);
    free_pointer(device->productId);
    free_pointer(device->version);
    free_pointer(device->location);
    free_pointer(device->type);

    if( device->service_head )
    {
      Service *iterator = NULL, *tmp = NULL;
      DL_FOREACH_SAFE( device->service_head, iterator, tmp)
      {
         serviceFree(iterator);
      }
    }

    if (device->free_data) device->free_data(device->data);
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

    if (deviceFindService(device, service->id))
        return HPD_E_ID_NOT_UNIQUE;

  service->device = device;
  DL_APPEND( device->service_head, service);

  return HPD_E_SUCCESS;
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
deviceRemoveService( Service *service )
{
   Device *device = service->device;
   if( service == NULL || device == NULL ) return HPD_E_NULL_POINTER;

   Service *iterator, *tmp;

   service->device = NULL;

   DL_FOREACH_SAFE( device->service_head, iterator, tmp )
   {
     if( strcmp( service->type, iterator->type ) == 0
    && strcmp ( service->id, iterator->id ) == 0 )
     {
       DL_DELETE( device->service_head, iterator );
       iterator->device = NULL;
       iterator = NULL;
       return HPD_E_SUCCESS; 
     }			
   }

   return -1;
}

Service *
deviceFindFirstService(Device *device, const char *description, const char *type,
                       const char *unit, const char *id)
{
  if (device == NULL) return NULL;

  Service *iterator;

  DL_FOREACH( device->service_head, iterator )
  {
    if ( description == NULL || (iterator->description != NULL && strcmp(description, iterator->description) == 0) )
      if ( type == NULL || (iterator->type != NULL && strcmp(type, iterator->type) == 0) )
        if ( unit == NULL || (iterator->unit != NULL && strcmp(unit, iterator->unit) == 0) )
          if ( id == NULL || (iterator->id != NULL && strcmp(id, iterator->id) == 0) )
              return iterator;
  }

  return NULL;
}

