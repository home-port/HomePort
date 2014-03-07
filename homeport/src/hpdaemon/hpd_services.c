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
#include "hpd_web_server_interface.h"
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
		      int isActuator,
                      char *type,
                      char *unit,
                      Device *device,
                      HPD_GetFunction get_function,
                      HPD_PutFunction put_function,
                      Parameter *parameter,
                      void* user_data_pointer)
{
	Service *service = (Service*)malloc(sizeof(Service));
	if( !service )
		return NULL;

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

	if(parameter == NULL)
	{
		printf("Service's parameter cannot be NULL\n");
		free(service->ID);
		free(service->type);
		free(service);
		return NULL;
	}
	else
	{
		service->parameter = parameter;
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

	service->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(service->mutex, NULL);

	/*Adding the service to the Device's Service List*/
	if( add_service_to_device (service,service->device) == -1 )
	{
		free(service);
		return NULL;
	}

   service->put_value = NULL;

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

		if( service_to_destroy->parameter )
			free_parameter_struct ( service_to_destroy->parameter );

		if( service_to_destroy->mutex )
			free(service_to_destroy->mutex);

      if (service_to_destroy->put_value)
         free(service_to_destroy->put_value);

		free(service_to_destroy);
	}
	return HPD_E_SUCCESS;
}

ServiceElement* 
create_service_element_struct( Service *service )
{
	ServiceElement *to_create;

	if( !service )
		return NULL;

	to_create = (ServiceElement*)malloc(sizeof(ServiceElement));
	if( !to_create )
		return NULL;

	to_create->service = service;

	to_create->next = NULL;
	to_create->prev = NULL;

	return to_create;
}


int 
destroy_service_element_struct( ServiceElement *service_element_to_destroy )
{
	if( !service_element_to_destroy )
		return HPD_E_NULL_POINTER;

	free( service_element_to_destroy );

	return 0;
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
add_service_to_device( Service *service, Device *device )
{

	if( service == NULL || device == NULL ) 
		return HPD_E_NULL_POINTER;

	ServiceElement *elt, *new_se;

	new_se = create_service_element_struct( service );
	if( !new_se )
		return HPD_E_NULL_POINTER;

	DL_SEARCH( device->service_head,
	           elt,
	           new_se,
	           cmp_ServiceElement );
	if( elt ) 
	{
		destroy_service_element_struct( new_se );
		return HPD_E_SERVICE_ALREADY_IN_LIST;
	}

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
remove_service_from_device( Service *service, Device *device )
{

	if( service == NULL || device == NULL ) return HPD_E_NULL_POINTER;

	ServiceElement *iterator, *tmp;

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
cmp_ServiceElement( ServiceElement *a, ServiceElement *b )
{
	if( !a || !b )
		return -1;
	return strcmp( a->service->value_url, b->service->value_url );
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
matching_service( ServiceElement *service_head, const char *url )
{
	ServiceElement *iterator;
	DL_FOREACH( service_head, iterator )
	{
		pthread_mutex_lock(iterator->service->mutex);
		if( strcmp( iterator->service->value_url, url) == 0 )
		{
			pthread_mutex_unlock( iterator->service->mutex );
			return iterator->service;
		}
		pthread_mutex_unlock( iterator->service->mutex );
	}
	return  NULL;
}

/**
 * Creates the structure Parameter with all its parameters
 *
 * @param ID The Parameter ID
 *
 * @param max The maximum value of the Parameter
 *
 * @param min The minimum value of the Parameter
 *
 * @param scale The Scale of the Parameter
 *
 * @param step The Step of the values of the Parameter
 *
 * @param type The Type of values for the Parameter
 *
 * @param unit The Unit of the values of the Parameter
 *
 * @param values The possible values for the Parameter
 *
 * @return returns the Parameter or NULL if failed, note that the ID can not be NULL
 */
Parameter* 
create_parameter_struct( char *ID,
                         char *max,
                         char *min,
                         char *scale,
                         char *step,
                         char *type,
                         char *unit,
                         char *values )
{
	Parameter *parameter = (Parameter*)malloc(sizeof(Parameter));
	if(!ID)
	{
		printf("Parameter ID cannot be NULL\n");
		free(parameter);
		return NULL;
	}
	else
	{
		parameter->ID = malloc(sizeof(char)*(strlen(ID)+1));
		strcpy(parameter->ID, ID);
	}

	if(max)
	{
		parameter->max = malloc(sizeof(char)*(strlen(max)+1));
		strcpy(parameter->max, max);
	}
	else parameter->max = NULL;

	if(min)
	{
		parameter->min = malloc(sizeof(char)*(strlen(min)+1));
		strcpy(parameter->min, min);
	}
	else parameter->min = NULL;

	if(scale)
	{
		parameter->scale = malloc(sizeof(char)*(strlen(scale)+1));
		strcpy(parameter->scale, scale);
	}
	else parameter->scale = NULL;

	if(step)
	{
		parameter->step = malloc(sizeof(char)*(strlen(step)+1));
		strcpy(parameter->step, step);
	}
	else parameter->step = NULL;

	if(type)
	{
		parameter->type = malloc(sizeof(char)*(strlen(type)+1));
		strcpy(parameter->type, type);
	}
	else parameter->type = NULL;

	if(unit)
	{
		parameter->unit = malloc(sizeof(char)*(strlen(unit)+1));
		strcpy(parameter->unit, unit);
	}
	else parameter->unit = NULL;

	if(values)
	{
		parameter->values = malloc(sizeof(char)*(strlen(values)+1));
		strcpy(parameter->values, values);
	}
	else parameter->values = NULL;

	return parameter;
}


/**
 * Frees all the memory allocated for the Parameter. Note
 * that it only frees the memory used by the API, if the
 * user allocates memory for Parameter attributes, he needs
 * to free it before/after calling this function.
 *
 * @param parameter The parameter to free
 *
 * @return 
 */
void 
free_parameter_struct(Parameter *parameter){

	if( parameter )
	{
		if(parameter->ID)
			free(parameter->ID);
		if(parameter->max)
			free(parameter->max);
		if(parameter->min)
			free(parameter->min);
		if(parameter->scale)
			free(parameter->scale);
		if(parameter->step)
			free(parameter->step);
		if(parameter->type)
			free(parameter->type);
		if(parameter->unit)
			free(parameter->unit);
		if(parameter->values)
			free(parameter->values);
		free(parameter);
	}
}


/**
 * Method that compares all the Parameter's Attributes
 *
 * @param a The Parameter to compare
 *
 * @param b The other Parameter to compare
 *
 * @return 0 if the same -1 or 1 if not
 */
int 
cmp_Parameter( Parameter *a, Parameter *b )
{
	if( a->ID != NULL )
	{
		if( strcmp(a->ID,b->ID) != 0 )
			return strcmp(a->ID,b->ID);
	}
	
	if( a->max != NULL )
	{
		if( strcmp(a->max,b->max) != 0 )
			return strcmp(a->max,b->max);
	}
	
	if( a->min != NULL )
	{
		if( strcmp(a->min,b->min) != 0 )
			return strcmp(a->min,b->min);
	}
	
	if( a->scale != NULL )
	{
		if( strcmp(a->scale,b->scale) != 0 )
			return strcmp(a->scale,b->scale);
	}
	
	if( a->step != NULL ){
		if( strcmp(a->step,b->step) != 0 )
			return strcmp(a->step,b->step);
	}
	
	if( a->type != NULL ){
		if( strcmp(a->type ,b->type) != 0 )
			return strcmp(a->type,b->type);
	}
	
	if( a->unit != NULL ){
		if( strcmp(a->unit,b->unit) != 0 )
			return strcmp(a->unit,b->unit);
	}
	
	if( a->values != NULL )
	{
		if( strcmp(a->values,b->values) != 0 )
			return strcmp(a->values,b->values);
	}
	return 0;
}
