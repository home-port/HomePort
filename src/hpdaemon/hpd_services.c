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

#include <stdio.h>
#include "hpd_services.h"
#include "utlist.h"
#include "hpd_web_server.h"
#include "hpd_error.h"

/**
 * Creates the structure Service with all its parameters
 *
 * @param description The Service description
 *
 * @param ID The Service ID
 *
 * @param type The Service type
 *
 * @param unit The unit provided by the Service
 *
 * @param device The Device that contains the 
 * 			   Service
 *
 * @param get_function A pointer to the GET function
 * 				   of the Service
 *
 * @param put_function A pointer to the PUT function
 *				    of the Service
 *
 * @param parameter The first parameter of the Service
 *
 * @param user_data_pointer A generic pointer used by the user to store his structures
 *
 * @return returns the Service or NULL if failed, note that
 * 		ID, type, device and get_function can not be NULL
 */
Service* 
create_service_struct(
                      char *description,
                      char *ID,
                      char *type,
                      char *unit,
                      Device *device,
                      HPD_GetFunction get_function,
                      HPD_PutFunction put_function,
                      Parameter *parameter,
                      void* user_data_pointer)
{
	Service *service = (Service*)malloc(sizeof(Service));

	if( ID == NULL )
	{
		printf("Service ID cannot be NULL\n");
		free(service);
		return NULL;
	}
	else
	{
		service->ID = malloc(sizeof(char)*(strlen(ID)+1));
		strcpy(service->ID, ID);
	}

	if( type == NULL )
	{
		printf("Service type cannot be NULL\n");
		free(service->ID);
		free(service);
		return NULL;
	}
	else
	{
		service->type = malloc(sizeof(char)*(strlen(type)+1));
		strcpy(service->type, type);
	}

	if( device == NULL )
	{
		printf("Service's device cannot be NULL\n");
		free(service->ID);
		free(service->type);
		free(service);
		return NULL;
	}
	else
	{
		service->device = device;
	}

	if( get_function == NULL )
	{
		printf("Service's get_function cannot be NULL\n");
		free(service->ID);
		free(service->type);
		free(service);
		return NULL;
	}
	else
	{
		service->get_function = get_function;
	}

	if( description == NULL )
	{
		service->description = NULL;
	}
	else
	{
		service->description = malloc(sizeof(char)*(strlen(description)+1));
		strcpy(service->description, description);
	}

	if( unit == NULL )
	{
		service->unit = NULL;
	}
	else
	{
		service->unit = malloc(sizeof(char)*(strlen(unit)+1));
		strcpy(service->unit, unit);
	}

	if( put_function == NULL )
	{
		service->put_function = NULL;
	}
	else
	{
		service->put_function = put_function;
	}

	if( user_data_pointer == NULL )
	{
		service->user_data_pointer = NULL;
	}
	else
	{
		service->user_data_pointer = user_data_pointer;
	}

	service->parameter_head = NULL;

	if( parameter != NULL )
	{
		DL_APPEND( service->parameter_head, parameter );
	}

	/*Creation of the URL*/
	service->value_url = malloc(sizeof(char)*( strlen("/") + strlen(service->device->type) + strlen("/") 
	                                           + strlen(service->device->ID) + strlen("/") + strlen(service->type)
	                                           + strlen("/") + strlen(service->ID) + 1 ) );
	sprintf( service->value_url,"/%s/%s/%s/%s", service->device->type, service->device->ID, service->type,
	         service->ID );

	/*Creation of the ZeroConf Name*/
	service->zeroConfName = malloc(sizeof(char)*( strlen(service->device->type) + strlen(" ") 
	                                              + strlen(service->device->ID) + strlen(" ") 
	                                              + strlen(service->type) + strlen(" ") + strlen(service->ID) + 1) );
	sprintf(service->zeroConfName,"%s %s %s %s", service->device->type, service->device->ID, service->type, service->ID);

	/*Determination of the type*/
	if( service->device->secure_device == HPD_SECURE_DEVICE ) service->DNS_SD_type = "_homeport-secure._tcp";
	else service->DNS_SD_type = "_homeport._tcp";

	service->get_function_buffer = malloc(sizeof(char)*MHD_MAX_BUFFER_SIZE);

	service-> prev = NULL;
	service-> next = NULL;

	service->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(service->mutex, NULL);

	/*Adding the service to the Device's Service List*/
	if( add_service_to_device (service,service->device) == -1 )
	{
		free(service);
		return NULL;
	}

	return service;
}

/**
 * Frees all the memory allocated for the Service. Note
 * that it only frees the memory used by the API, if the
 * user allocates memory for Services attributes, he needs
 * to free it before/after calling this function. Also note
 * that the user can't destroy a Service that is
 * registered on the server.
 *
 * @param service_to_destroy The service to destroy
 *
 * @return returns A HPD error code
 */
int 
destroy_service_struct( Service *service_to_destroy )
{

	if( service_to_destroy )
	{

		if( is_service_registered( service_to_destroy ) )
		{
			printf("Cannot destroy a registered service.\n Make a call to HPD_unregister_service before destroying service\n");
			return HPD_E_SERVICE_IN_USE;
		}

		if( service_to_destroy->device )
		{
			if( service_to_destroy->device->service_head )
			{

				remove_service_from_device(service_to_destroy, service_to_destroy->device);

				if( service_to_destroy->device->service_head == NULL )
				{
					destroy_device_struct( service_to_destroy->device );
					service_to_destroy->device = NULL;
				}
			}
		}

		if( service_to_destroy->description )
			free(service_to_destroy->description);

		if( service_to_destroy->ID )
			free(service_to_destroy->ID);

		if( service_to_destroy->type )
			free(service_to_destroy->type);

		if( service_to_destroy->unit )
			free(service_to_destroy->unit);

		if( service_to_destroy->value_url )
			free(service_to_destroy->value_url);

		if( service_to_destroy->zeroConfName )
			free(service_to_destroy->zeroConfName);

		if( service_to_destroy->get_function_buffer )
			free(service_to_destroy->get_function_buffer);

		if( service_to_destroy->parameter_head )
		{
			Parameter *tmp, *iterator;
			DL_FOREACH_SAFE(service_to_destroy->parameter_head, iterator, tmp)
			{
				DL_DELETE(service_to_destroy->parameter_head, iterator);
				free_parameter_struct (iterator);
			}
		}

		free(service_to_destroy);
	}
	return HPD_E_SUCCESS;
}


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

		Service *tmp, *iterator = NULL;

		DL_FOREACH_SAFE( device_to_destroy->service_head, iterator, tmp )
		{
			if( is_service_registered( iterator ) )
			{
				printf("Device has registered service(s). Call HPD_unregister_device before destroying\n");
				return HPD_E_SERVICE_IN_USE;
			}
			DL_DELETE( device_to_destroy->service_head, iterator );
			destroy_service_struct (iterator);
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
 * Adds a Parameter to the Service
 *
 * @param parameter The Parameter to add
 *
 * @param service The Service 
 *
 * @return A HPD error code
 */
int 
add_parameter_to_service( Parameter *parameter, Service *service )
{
	if( parameter == NULL || service == NULL ) 
		return HPD_E_NULL_POINTER;

	Parameter *elt;

	DL_SEARCH( service->parameter_head,
	           elt,
	           parameter,
	           cmp_Parameter);

	if( elt ) 
		return HPD_E_PARAMETER_ALREADY_IN_LIST;

	DL_APPEND( service->parameter_head, parameter );

	return HPD_E_SUCCESS;
}


/**
 * Removes a Parameter from the Service
 *
 * @param parameter The Parameter to remove
 *
 * @param service The Service 
 *
 * @return return A HPD error code 
 */
int 
remove_parameter_from_service( Parameter *parameter, Service *service )
{

	if( parameter == NULL || service == NULL ) 
		return HPD_E_NULL_POINTER;

	Parameter *elt;

	DL_SEARCH( service->parameter_head,
	           elt,
	           parameter,
	           cmp_Parameter );

	if( !elt ) 
		return HPD_E_PARAMETER_NOT_IN_LIST;

	DL_DELETE( service->parameter_head, parameter );

	return HPD_E_SUCCESS;
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
add_service_to_device( Service *service, Device *device )
{

	if( service == NULL || device == NULL ) 
		return HPD_E_NULL_POINTER;

	Service *elt;

	DL_SEARCH( device->service_head,
	           elt,
	           service,
	           cmp_Service );
	if( elt ) 
		return HPD_E_SERVICE_ALREADY_IN_LIST;

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
remove_service_from_device( Service *service, Device *device )
{

	if( service == NULL || device == NULL ) return HPD_E_NULL_POINTER;

	Service *iterator, *tmp;

	DL_FOREACH_SAFE( device->service_head, iterator, tmp )
	{
		if( strcmp( service->type, iterator->type ) == 0
		   && strcmp ( service->ID, iterator->ID ) == 0 )
		{
			DL_DELETE( service->device->service_head, iterator );
			iterator = NULL;
			break;
		}			
	}

	return HPD_E_SUCCESS;
}

/**
 * Compares two Services
 *
 * @param a The first Service
 *
 * @param b The second Service
 *
 * @return returns 0 if the same -1 or 1 if not
 */
int 
cmp_Service( Service *a, Service *b )
{
	return strcmp( a->value_url, b->value_url );
}

/**
 * Returns a Service that matches the URL in the Service List
 *
 * @param service_head The head of the service list
 *
 * @param url The URL to check
 *
 * @return returns the Service if it exists NULL if not
 */
Service* 
matching_service( Service *service_head, char *url )
{
	Service *iterator;
	DL_FOREACH( service_head, iterator )
	{
		pthread_mutex_lock(iterator->mutex);
		if( strcmp( iterator->value_url, url) == 0 )
		{
			pthread_mutex_unlock( iterator->mutex );
			return iterator;
		}
		pthread_mutex_unlock( iterator->mutex );
	}
	return  NULL;
}
