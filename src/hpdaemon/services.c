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

#include <stdio.h>
#include "services.h"
#include "utlist.h"
#include "web_server.h"
#include "hpd_error.h"

/**
 * Creates the structure Service with all its parameters
 *
 * @param _description The Service description
 *
 * @param _ID The Service ID
 *
 * @param _type The Service type
 *
 * @param _unit The unit provided by the Service
 *
 * @param _device The Device that contains the 
 * 			   Service
 *
 * @param _get_function A pointer to the GET function
 * 				   of the Service
 *
 * @param _put_function A pointer to the PUT function
 *				    of the Service
 *
 * @param _parameter The first parameter of the Service
 *
 * @return returns the Service or NULL if failed, note that
 * 		ID, type, device and get_function can not be NULL
 */
Service* create_service_struct(
                               char *_description,
                               char *_ID,
                               char *_type,
                               char *_unit,
                               Device *_device,
                               HPD_GetFunction _get_function,
                               HPD_PutFunction _put_function,
                               Parameter *_parameter,
			       void* _user_data_pointer)
{
    	Service *_service = (Service*)malloc(sizeof(Service));

	if(_ID == NULL)
    	{
        	printf("Service ID cannot be NULL\n");
        	free(_service);
        	return NULL;
    	}
	else
	{
		_service->ID = malloc(sizeof(char)*(strlen(_ID)+1));
		strcpy(_service->ID, _ID);
	}

	if(_type == NULL)
    	{
        	printf("Service type cannot be NULL\n");
		free(_service->ID);
        	free(_service);
        	return NULL;
    	}
	else
	{
		_service->type = malloc(sizeof(char)*(strlen(_type)+1));
		strcpy(_service->type, _type);
	}

		if(_device == NULL)
    	{
        	printf("Service's device cannot be NULL\n");
		free(_service->ID);
		free(_service->type);
        	free(_service);
        	return NULL;
    	}
	else
	{
		_service->device = _device;
	}

	if(_get_function == NULL)
    	{
        	printf("Service's get_function cannot be NULL\n");
		free(_service->ID);
		free(_service->type);
        	free(_service);
        	return NULL;
    	}
	else
	{
		_service->get_function = _get_function;
	}

    	if(_description == NULL)
    	{
        	_service->description = NULL;
    	}
	else
	{
		_service->description = malloc(sizeof(char)*(strlen(_description)+1));
		strcpy(_service->description, _description);
	}

	if(_unit == NULL)
    	{
        	_service->unit = NULL;
    	}
	else
	{
		_service->unit = malloc(sizeof(char)*(strlen(_unit)+1));
		strcpy(_service->unit, _unit);
	}

	if(_put_function == NULL)
    	{
        	_service->put_function = NULL;
    	}
	else
	{
		_service->put_function = _put_function;
	}
	
	if(_user_data_pointer == NULL)
    	{
        	_service->user_data_pointer = NULL;
    	}
	else
	{
		_service->user_data_pointer = _user_data_pointer;
	}

	_service->parameter_head = NULL;

	if(_parameter != NULL)
	{
		ParameterElement *_parameter_to_add = create_parameter_element (_parameter);
		LL_APPEND(_service->parameter_head, _parameter_to_add);
	}

    	/*Creation of the URL*/
    	_service->value_url = malloc(sizeof(char)*(strlen("/")+strlen(_service->device->type)+strlen("/")+strlen(_service->device->ID)+strlen("/")
    	                                  +strlen(_service->type)+strlen("/")+strlen(_service->ID)+1));
    	sprintf(_service->value_url,"/%s/%s/%s/%s",_service->device->type,_service->device->ID,_service->type,_service->ID);

    	/*Creation of the ZeroConf Name*/
    	_service->zeroConfName = malloc(sizeof(char)*(strlen(_service->device->type)+strlen(" ")+strlen(_service->device->ID)+strlen(" ")
    	                                           +strlen(_service->type)+strlen(" ")+strlen(_service->ID)+1));
    	sprintf(_service->zeroConfName,"%s %s %s %s",_service->device->type,_service->device->ID,_service->type,_service->ID);

    	/*Determination of the type*/
    	if(_service->device->secure_device == HPD_SECURE_DEVICE ) _service->DNS_SD_type = "_homeport-secure._tcp";
	else _service->DNS_SD_type = "_homeport._tcp";

    	/*Adding the service to the Device's Service List*/
    	if(add_service_to_device (_service,_service->device) == -1)
	{
    	    free(_service);
    	    return NULL;
    	}

	_service->get_function_buffer = malloc(sizeof(char)*MHD_MAX_BUFFER_SIZE);

    return _service;
}

/**
 * Frees all the memory allocated for the Service. Note
 * that it only frees the memory used by the API, if the
 * user allocates memory for Services attributes, he needs
 * to free it before/after calling this function. Also note
 * that the user can't destroy a Service that is
 * registered on the server.
 *
 * @param _service The service to destroy
 *
 * @return returns A HPD error code
 */
int destroy_service_struct(Service *_service)
{

	if(_service)
	{

		if( is_service_registered( _service ) )
		{
			printf("Cannot destroy a registered service.\n Make a call to HPD_unregister_service before destroying service\n");
			return HPD_E_SERVICE_IN_USE;
		}

		if(_service->description)
			free(_service->description);

		if(_service->ID)
			free(_service->ID);

		if(_service->type)
			free(_service->type);

		if(_service->unit)
			free(_service->unit);

		if(_service->value_url)
			free(_service->value_url);

		if(_service->zeroConfName)
			free(_service->zeroConfName);
		
		if(_service->get_function_buffer)
			free(_service->get_function_buffer);

		if(_service->parameter_head)
		{
			ParameterElement *_tmp, *_iterator;
			LL_FOREACH_SAFE(_service->parameter_head, _iterator, _tmp)
			{
				LL_DELETE(_service->parameter_head, _iterator);
				free_parameter_struct (_iterator->parameter);
				free(_iterator);
			}
		}

		if( _service->device )
		{
			if( _service->device->service_head )
			{
				
				remove_service_from_device(_service, _service->device);

				if( _service->device->service_head == NULL )
				{
					destroy_device_struct( _service->device );
					_service->device = NULL;
				}
			}
		}

		free(_service);
	}
    return HPD_E_SUCCESS;
}


/**
 * Create the structure Device with all its parameters
 *
 * @param _description The Device description
 *
 * @param _ID The Device ID
 *
 * @param _vendorID The ID of the vendor
 *
 * @param _productID The ID of the product
 *
 * @param _version The Device version
 *
 * @param _IP The IP address of the Device
 *
 * @param _port The port that the Device uses
 *
 * @param _location The location of the Device
 *
 * @param _type The Device type
 *
 * @param _secure_device A variable that specifies
 *				     if the Device is a secured
 *				     or not secured service
 *				     (HPD_SECURE_DEVICE or
 *				       HPD_NON_SECURE_DEVICE)
 *
 * @return returns the Device or NULL if failed, note that
 * 		ID and type can not be NULL
 */
Device* create_device_struct(
                             char *_description,
                             char *_ID,
                             char *_vendorID,
                             char *_productID,
                             char *_version,
                             char *_IP,
                             char *_port,
                             char *_location,
                             char *_type,
                             int _secure_device)
{
    	Device *_device = (Device*)malloc(sizeof(Device));
	
	if(_ID == NULL)
    	{
        	printf("Device ID cannot be NULL\n");
        	free(_device);
        	return NULL;
    	}
	else
	{
		_device->ID = malloc(sizeof(char)*(strlen(_ID)+1));
		strcpy(_device->ID, _ID);
	}

	if(_type == NULL)
    	{
        	printf("Device type cannot be NULL\n");
		free(_device->ID);
        	free(_device);
        	return NULL;
    	}
	else
	{
		_device->type = malloc(sizeof(char)*(strlen(_type)+1));
		strcpy(_device->type, _type);
	}
	
	if(_description == NULL)
    	{
        	_device->description = NULL;
    	}
	else
	{
		_device->description = malloc(sizeof(char)*(strlen(_description)+1));
		strcpy(_device->description, _description);
	}
	
	if(_vendorID == NULL)
    	{
        	_device->vendorID = NULL;
    	}
	else
	{
		_device->vendorID = malloc(sizeof(char)*(strlen(_vendorID)+1));
		strcpy(_device->vendorID, _vendorID);
	}
	
	if(_productID == NULL)
    	{
        	_device->productID = NULL;
    	}
	else
	{
		_device->productID = malloc(sizeof(char)*(strlen(_productID)+1));
		strcpy(_device->productID, _productID);
	}

	if(_version == NULL)
    	{
        	_device->version = NULL;
    	}
	else
	{
		_device->version = malloc(sizeof(char)*(strlen(_version)+1));
		strcpy(_device->version, _version);
	}

	if(_IP == NULL)
    	{
        	_device->IP = NULL;
    	}
	else
	{
		_device->IP = malloc(sizeof(char)*(strlen(_IP)+1));
		strcpy(_device->IP, _IP);
	}

	if(_port == NULL)
    	{
        	_device->port = NULL;
    	}
	else
	{
		_device->port = malloc(sizeof(char)*(strlen(_port)+1));
		strcpy(_device->port, _port);
	}

	if(_location == NULL)
    	{
        	_device->location = NULL;
    	}
	else
	{
		_device->location = malloc(sizeof(char)*(strlen(_location)+1));
		strcpy(_device->location, _location);
	}

	if(_secure_device == HPD_SECURE_DEVICE || _secure_device == HPD_NON_SECURE_DEVICE)
        	_device->secure_device = _secure_device;
    	else
        	_device->secure_device = HPD_NON_SECURE_DEVICE;
    
    	_device->service_head = NULL;
    
    return _device;
}

/**
 * Frees all the memory allocated for the Device. Note
 * that it only frees the memory used by the API, if the
 * user allocates memory for Devices attributes, he needs
 * to free it before/after calling this function. Also note
 * that the user can't destroy a Device that has Services
 * registered on the server.
 *
 * @param _device The Device to destroy
 *
 * @return A HPD error code
 */
int destroy_device_struct(Device *_device)
{

	if(_device)
	{

		ServiceElement *_tmp, *_iterator = NULL;

		LL_FOREACH_SAFE( _device->service_head, _iterator, _tmp )
		{
			if( is_service_registered( _iterator->service ) )
			{
				printf("Device has registered service(s). Call HPD_unregister_device before destroying\n");
				return HPD_E_SERVICE_IN_USE;
			}
			LL_DELETE( _device->service_head, _iterator );
			destroy_service_struct (_iterator->service);
			destroy_service_element(_iterator);
			_iterator = NULL;
		}		
		
		if(_device->description)
			free(_device->description);

		if(_device->ID)
			free(_device->ID);

		if(_device->vendorID)
			free(_device->vendorID);

		if(_device->productID)
			free(_device->productID);

		if(_device->version)
			free(_device->version);

		if(_device->IP)
			free(_device->IP);

		if(_device->port)
			free(_device->port);

		if(_device->location)
			free(_device->location);

		if(_device->type)
			free(_device->type);

		free(_device);

		return HPD_E_SUCCESS;
	}

	return HPD_E_NULL_POINTER;
}

/**
 * Adds a Parameter to the Service
 *
 * @param _parameter The Parameter to add
 *
 * @param _service The Service 
 *
 * @return A HPD error code
 */
int add_parameter_to_service(Parameter *_parameter, Service *_service)
{
    if(_parameter == NULL || _service == NULL) return HPD_E_NULL_POINTER;

    ParameterElement *elt;
    ParameterElement *_parameter_to_add = create_parameter_element (_parameter);

    LL_SEARCH(_service->parameter_head,elt,_parameter_to_add,cmp_ParameterElement);
    if(elt) return HPD_E_PARAMETER_ALREADY_IN_LIST;
    
    LL_APPEND(_service->parameter_head, _parameter_to_add);
    
    return HPD_E_SUCCESS;
}


/**
 * Removes a Parameter from the Service
 *
 * @param _parameter The Parameter to remove
 *
 * @param _service The Service 
 *
 * @return return A HPD error code 
 */
int remove_parameter_from_service(Parameter *_parameter, Service *_service)
{

	if(_parameter == NULL || _service == NULL) return -1;

    ParameterElement *elt;
    ParameterElement *_parameter_to_remove = create_parameter_element (_parameter);

    LL_SEARCH(_service->parameter_head,elt,_parameter_to_remove,cmp_ParameterElement);
    if(!elt) return HPD_E_PARAMETER_NOT_IN_LIST;
    
    LL_DELETE(_service->parameter_head,_parameter_to_remove);
    return HPD_E_SUCCESS;
}



/**
 * Adds a Service to the Device
 *
 * @param _service The Service to add
 *
 * @param _device The Device 
 *
 * @return returns A HPD error code
 */
int add_service_to_device(Service *_service, Device *_device)
{
    
    if(_service == NULL || _device == NULL) return HPD_E_NULL_POINTER;
    
    ServiceElement *_service_to_add = create_service_element (_service);
    ServiceElement *elt;

    LL_SEARCH(_device->service_head,elt,_service_to_add,cmp_ServiceElement);
    if(elt) 
	{
		destroy_service_element(_service_to_add);
		return HPD_E_SERVICE_ALREADY_IN_LIST;
	}
    LL_APPEND(_device->service_head, _service_to_add);

    return HPD_E_SUCCESS;
}


/**
 * Removes a Service from the Device
 *
 * @param _service The Service to remove
 *
 * @param _device The Device 
 *
 * @return returns A HPD error code
 */
int remove_service_from_device(Service *_service, Device *_device)
{

    if(_service == NULL || _device == NULL) return HPD_E_NULL_POINTER;
    
    ServiceElement *_iterator, *_tmp;

	LL_FOREACH_SAFE(_device->service_head, _iterator, _tmp)
	{
		if( strcmp( _service->type, _iterator->service->type ) == 0
		   && strcmp ( _service->ID, _iterator->service->ID ) == 0 )
		{
			LL_DELETE( _service->device->service_head, _iterator );
			destroy_service_element(_iterator);
			_iterator = NULL;
			break;
		}			
	}

    return HPD_E_SUCCESS;
}

/**
 * Creates a Service Element for the Service List
 *
 * @param _service The Service 
 *
 * @return returns the Service Element if successfull NULL if failed
 */
ServiceElement* create_service_element(Service *_service)
{
    ServiceElement *_service_list_element = (ServiceElement*)malloc(sizeof(ServiceElement));
    _service_list_element->service = _service;
    _service_list_element->next = NULL;
    _service_list_element->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(_service_list_element->mutex, NULL);
    
    return _service_list_element;
}

/**
 * Creates a Service Element for the Service List derived from another service element
 *
 * @param _service_element_to_subscribe The Service Element 
 *
 * @return returns the Service Element if successfull NULL if failed
 */
ServiceElement* create_subscribed_service_element(ServiceElement *_service_element_to_subscribe)
{
    ServiceElement *_service_list_element = (ServiceElement*)malloc(sizeof(ServiceElement));
    _service_list_element->service = _service_element_to_subscribe->service;
    _service_list_element->next = NULL;
    _service_list_element->mutex = _service_element_to_subscribe->mutex;

    return _service_list_element;
}

/**
 * Destroys a Service Element
 *
 * @param _service_element_to_destroy The Service Element 
 *
 * @return void
 */
void destroy_service_element(ServiceElement *_service_element_to_destroy)
{
	if(_service_element_to_destroy)
	{
		if(_service_element_to_destroy->mutex)
		{
			pthread_mutex_destroy(_service_element_to_destroy->mutex);
		}
		if(_service_element_to_destroy->mutex)
		{
			free(_service_element_to_destroy->mutex);
		}
		free(_service_element_to_destroy);
	}
}

/**
 * Destroys a Service Element used for subscription
 *
 * @param _subscribed_service_element_to_destroy The Service Element 
 *
 * @return void
 */
void destroy_subscribed_service_element(ServiceElement *_subscribed_service_element_to_destroy)
{
     if(_subscribed_service_element_to_destroy)
    {
        free(_subscribed_service_element_to_destroy);
    }
}


/**
 * Compares two Service Elements
 *
 * @param a The first Service Element
 *
 * @param b The second Service Element
 *
 * @return returns 0 if the same -1 or 1 if not
 */
int cmp_ServiceElement( ServiceElement *a, ServiceElement *b)
{
	return strcmp( a->service->value_url, b->service->value_url );
}

/**
 * Returns a ServiceElement that matches the URL in the Service List
 *
 * @param _service_head The head of the service list
 *
 * @param _url The URL to check
 *
 * @return returns the Service if it exists NULL if not
 */
ServiceElement* matching_service(ServiceElement *_service_head, char *_url)
{
    ServiceElement *_iterator;
    LL_FOREACH(_service_head, _iterator)
    {
        if( strcmp( _iterator->service->value_url, _url) == 0 )
        {
	   return _iterator;
        }
    }
    return  NULL;
}
