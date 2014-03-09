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
create_device_struct(
    char *description,
    char *ID,
    char *vendorID,
    char *productID,
    char *version,
    char *IP,
    char *port,
    char *location,
    char *type,
    int secure_device)
{
  Device *device = (Device*)malloc(sizeof(Device));

  if( ID == NULL )
  {
    printf("Device ID cannot be NULL\n");
    free(device);
    return NULL;
  }
  else
  {
    device->ID = malloc(sizeof(char)*(strlen(ID)+1));
    strcpy(device->ID, ID);
  }

  if( type == NULL )
  {
    printf("Device type cannot be NULL\n");
    free(device->ID);
    free(device);
    return NULL;
  }
  else
  {
    device->type = malloc(sizeof(char)*(strlen(type)+1));
    strcpy(device->type, type);
  }

  if( description == NULL )
  {
    device->description = NULL;
  }
  else
  {
    device->description = malloc(sizeof(char)*(strlen(description)+1));
    strcpy(device->description, description);
  }

  if( vendorID == NULL )
  {
    device->vendorID = NULL;
  }
  else
  {
    device->vendorID = malloc(sizeof(char)*(strlen(vendorID)+1));
    strcpy(device->vendorID, vendorID);
  }

  if( productID == NULL )
  {
    device->productID = NULL;
  }
  else
  {
    device->productID = malloc(sizeof(char)*(strlen(productID)+1));
    strcpy(device->productID, productID);
  }

  if( version == NULL)
  {
    device->version = NULL;
  }
  else
  {
    device->version = malloc(sizeof(char)*(strlen(version)+1));
    strcpy(device->version, version);
  }

  if( IP == NULL )
  {
    device->IP = NULL;
  }
  else
  {
    device->IP = malloc(sizeof(char)*(strlen(IP)+1));
    strcpy(device->IP, IP);
  }

  if( port == NULL )
  {
    device->port = NULL;
  }
  else
  {
    device->port = malloc(sizeof(char)*(strlen(port)+1));
    strcpy(device->port, port);
  }

  if( location == NULL )
  {
    device->location = NULL;
  }
  else
  {
    device->location = malloc(sizeof(char)*(strlen(location)+1));
    strcpy(device->location, location);
  }

  if( secure_device == HPD_SECURE_DEVICE || secure_device == HPD_NON_SECURE_DEVICE )
    device->secure_device = secure_device;
  else
    device->secure_device = HPD_NON_SECURE_DEVICE;

  device->service_head = NULL;

  return device;
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
int 
destroy_device_struct( Device *device_to_destroy )
{

  if(device_to_destroy)
  {

    ServiceElement *tmp, *iterator = NULL;

    DL_FOREACH_SAFE( device_to_destroy->service_head, iterator, tmp )
    {
      if( is_service_registered( iterator->service ) )
      {
	printf("Device has registered service(s). Call HPD_unregister_device before destroying\n");
	return HPD_E_SERVICE_IN_USE;
      }
      DL_DELETE( device_to_destroy->service_head, iterator );
      destroy_service_struct (iterator->service);
      destroy_service_element_struct( iterator );
      iterator = NULL;
    }		

    if( device_to_destroy->description )
      free(device_to_destroy->description);

    if( device_to_destroy->ID )
      free(device_to_destroy->ID);

    if( device_to_destroy->vendorID )
      free(device_to_destroy->vendorID);

    if( device_to_destroy->productID )
      free(device_to_destroy->productID);

    if( device_to_destroy->version )
      free(device_to_destroy->version);

    if( device_to_destroy->IP )
      free(device_to_destroy->IP);

    if( device_to_destroy->port )
      free(device_to_destroy->port);

    if( device_to_destroy->location )
      free(device_to_destroy->location);

    if( device_to_destroy->type )
      free(device_to_destroy->type);

    free(device_to_destroy);

    return HPD_E_SUCCESS;
  }

  return HPD_E_NULL_POINTER;
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
add_service_to_device( Device *device, Service *service )
{

  if( service == NULL || device == NULL ) 
    return HPD_E_NULL_POINTER;

  ServiceElement *new_se;

  if( findService(device, service->ID) )
  {
    return HPD_E_SERVICE_ALREADY_IN_LIST;
  }

  new_se = create_service_element_struct( service );
  if( !new_se )
    return HPD_E_NULL_POINTER;

  DL_APPEND( device->service_head, new_se);

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
remove_service_from_device( Device *device, Service *service )
{

  if( service == NULL || device == NULL ) return HPD_E_NULL_POINTER;

  ServiceElement *toDelete;

  DL_FOREACH_SAFE( device->service_head, iterator, tmp )
  {
    if( strcmp( service->type, iterator->service->type ) == 0
	&& strcmp ( service->ID, iterator->service->ID ) == 0 )
    {
      DL_DELETE( service->device->service_head, iterator );
      destroy_service_element_struct( iterator );
      iterator = NULL;
      break;
    }			
  }

  return HPD_E_SUCCESS;
}

ServiceElement*
findServiceElement(Device, char *service_id)
{
  if(device == NULL || service_id == NULL ) return HPD_E_NULL_POINTER;

  ServiceElement *iterator;

  DL_FOREACH( device->service_head, iterator )
  {
    if( strcmp ( service_id, iterator->service->ID ) == 0 )
    {
      return iterator;
    }			
  }
  
  return NULL;
}

Service*
findService(Device *device, char *service_id)
{
  if(device == NULL || service_id == NULL ) return HPD_E_NULL_POINTER;

  ServiceElement *serviceElement;

  if( serviceElement = findServiceElement(device, service_id) )
  {
    return serviceElement->service;
  }

  return NULL;

}

mxml_node_t*
deviceToXml(Device *device)
{
  if(device == NULL) return HPD_E_NULL_POINTER;

  mxml_node_t *deviceXml;

  deviceXml = mxmlNewElement(MXML_NO_PARENT, "device");
  if(device->description != NULL) mxmlElementSetAttr(deviceXml, "desc", device->description);
  if(device->ID != NULL) mxmlElementSetAttr(deviceXml, "id", device->ID);
  if(device->vendorID != NULL) mxmlElementSetAttr(deviceXml, "vendorID", device->vendorID);
  if(device->productID != NULL) mxmlElementSetAttr(deviceXml, "productID", device->productID);
  if(device->version != NULL) mxmlElementSetAttr(deviceXml, "version", device->version);
  if(device->IP != NULL) mxmlElementSetAttr(deviceXml, "ip", device->IP);
  if(device->port != NULL) mxmlElementSetAttr(deviceXml, "port", device->port);
  if(device->location != NULL) mxmlElementSetAttr(deviceXml, "location", device->location);
  if(device->type != NULL) mxmlElementSetAttr(deviceXml, "type", device->type);

  return deviceXml;
}
