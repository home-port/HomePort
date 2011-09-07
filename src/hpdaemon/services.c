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
#include "web_server_api.h"

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
                               char* (*_get_function)(Service*),
                               char* (*_put_function)(Service*, char*),
                               Parameter *_parameter)
{
    Service *_service = (Service*)malloc(sizeof(Service));
    _service->description = _description;
    _service->ID = _ID;
    if(_service->ID==NULL)
    {
        printf("Service ID cannot be NULL\n");
        free(_service);
        return NULL;
    }

    _service->type = _type;
    if(_service->type==NULL)
    {
        printf("Service type cannot be NULL\n");
        free(_service);
        return NULL;
    }

    _service->unit = _unit;
    _service->device = _device;
    if(_service->device==NULL)
    {
        printf("Service device cannot be NULL\n");
        free(_service);
        return NULL;
    }

    _service->get_function = _get_function;
    if(_service->get_function==NULL)
    {
        printf("Service get_function cannot be NULL\n");
        free(_service);
        return NULL;
    }

    _service->put_function = _put_function;
    _service->parameter_head = NULL;

    if(_parameter!=NULL){
        ParameterElement *_parameter_to_add = create_parameter_element (_parameter);
        LL_APPEND(_service->parameter_head, _parameter_to_add);
    }

    /*Creation of the URL*/
    char * URL = malloc(sizeof(char)*(strlen("/")+strlen(_service->device->type)+strlen("/")+strlen(_service->device->ID)+strlen("/")
                                      +strlen(_service->type)+strlen("/")+strlen(_service->ID)+1));
    sprintf(URL,"/%s/%s/%s/%s",_service->device->type,_service->device->ID,_service->type,_service->ID);
    _service->value_url = URL;

    /*Creation of the ZeroConf Name*/
    char * zeroConfName = malloc(sizeof(char)*(strlen(_service->device->type)+strlen(" ")+strlen(_service->device->ID)+strlen(" ")
                                               +strlen(_service->type)+strlen(" ")+strlen(_service->ID)+1));
    sprintf(zeroConfName,"%s %s %s %s",_service->device->type,_service->device->ID,_service->type,_service->ID);
    _service->zeroConfName = zeroConfName;

    /*Determination of the type*/
    if( _service->device->secure_device == HPD_NON_SECURE_DEVICE ) _service->DNS_SD_type = "_homeport._tcp";
    else if(_service->device->secure_device == HPD_SECURE_DEVICE ) _service->DNS_SD_type = "_homeport-secure._tcp";
    else {
        free(URL);
        free(zeroConfName);
        free(_service);
        return NULL;
    }

    /*Adding the service to the Device's Service List*/
    if(add_service_to_device (_service,_service->device) == -1){
        free(URL);
        free(zeroConfName);
        free(_service);
        return NULL;
    }

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
 * @return returns 0 if successful -1 if failed
 */
int destroy_service_struct(Service *_service)
{

	if(_service)
	{

		if( is_service_registered( _service ) )
		{
			printf("Cannot destroy a registered service.\n Make a call to HPD_unregister_service before destroying service\n");
			return -1;
		}

		if(_service->value_url)
			free(_service->value_url);

		if(_service->zeroConfName)
			free(_service->zeroConfName);

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
				ServiceElement *_tmp, *_iterator = NULL;
				
				LL_FOREACH_SAFE( _service->device->service_head, _iterator, _tmp )
				{
					if( strcmp( _service->type, _iterator->service->type ) == 0
					   && strcmp ( _service->ID, _iterator->service->ID ) == 0 )
					{
						LL_DELETE( _service->device->service_head, _iterator );
						break;
					}
				}

				if( _service->device->service_head == NULL )
				{
					destroy_device_struct( _service->device );
					_service->device = NULL;
				}
			}
		}

		free(_service);
	}
    return 0;
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
    _device->description = _description;
    _device->ID = _ID;
    if(_device->ID==NULL)
    {
        printf("Device ID cannot be NULL\n");
        free(_device);
        return NULL;
    }
    _device->vendorID = _vendorID;
    _device->productID = _productID;
    _device->version = _version;
    _device->IP = _IP;
    _device->port = _port;
    _device->location = _location;
    _device->type = _type;
    if(_device->type==NULL)
    {
        printf("Device type cannot be NULL\n");
        free(_device);
        return NULL;
    }

    if(_secure_device==HPD_SECURE_DEVICE || _secure_device==HPD_NON_SECURE_DEVICE)
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
 * @return returns 0 if successful -1 if failed
 */
int destroy_device_struct(Device *_device)
{
	ServiceElement *_tmp, *_iterator = NULL;

	LL_FOREACH_SAFE( _device->service_head, _iterator, _tmp )
	{
		if( is_service_registered( _iterator->service ) )
		{
			printf("Device has registered service(s). Call HPD_unregister_device before destroying\n");
			return -1;
		}
		LL_DELETE( _device->service_head, _iterator );
		destroy_service_struct (_iterator->service);
		free(_iterator);
		_iterator = NULL;
	}
    
    if(_device)
        free(_device);

	return 0;
}

/**
 * Adds a Parameter to the Service
 *
 * @param _parameter The Parameter to add
 *
 * @param _service The Service 
 *
 * @return returns 0 if successful -1 if failed -2 if already exist
 */
int add_parameter_to_service(Parameter *_parameter, Service *_service)
{
    if(_parameter == NULL || _service == NULL) return -1;

    ParameterElement *elt;
    ParameterElement *_parameter_to_add = create_parameter_element (_parameter);

    LL_SEARCH(_service->parameter_head,elt,_parameter_to_add,cmp_ParameterElement);
    if(elt) return -2;
    
    LL_APPEND(_service->parameter_head, _parameter_to_add);
    
    return 0;
}


/**
 * Removes a Parameter from the Service
 *
 * @param _parameter The Parameter to remove
 *
 * @param _service The Service 
 *
 * @return returns 0 if successful -1 if failed -2 if not in the list
 */
int remove_parameter_from_service(Parameter *_parameter, Service *_service)
{

	if(_parameter == NULL || _service == NULL) return -1;

    ParameterElement *elt;
    ParameterElement *_parameter_to_remove = create_parameter_element (_parameter);

    LL_SEARCH(_service->parameter_head,elt,_parameter_to_remove,cmp_ParameterElement);
    if(!elt) return -2;
    
    LL_DELETE(_service->parameter_head,_parameter_to_remove);
    return 0;
}



/**
 * Adds a Service to the Device
 *
 * @param _service The Service to add
 *
 * @param _device The Device 
 *
 * @return returns 0 if successful -1 if failed -2 if already in the list
 */
int add_service_to_device(Service *_service, Device *_device)
{
    
    if(_service == NULL || _device == NULL) return -1;
    
    ServiceElement *_service_to_add = create_service_element (_service);
    ServiceElement *elt;

    LL_SEARCH(_device->service_head,elt,_service_to_add,cmp_ServiceElement);
    if(elt) return -2;
    
    LL_APPEND(_device->service_head, _service_to_add);

    return 0;
}


/**
 * Removes a Service from the Device
 *
 * @param _service The Service to remove
 *
 * @param _device The Device 
 *
 * @return returns 0 if successful -1 if failed -2 if not in the list
 */
int remove_service_from_device(Service *_service, Device *_device)
{

    if(_service == NULL || _device == NULL) return -1;
    
    ServiceElement *_service_to_remove = create_service_element (_service);
    ServiceElement *elt;

    LL_SEARCH(_device->service_head,elt,_service_to_remove,cmp_ServiceElement);
    if(!elt) return -2;
    
    LL_DELETE(_device->service_head, _service_to_remove);

    return 0;
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
    pthread_mutex_init(&_service_list_element->mutex, NULL);
    
    return _service_list_element;
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
