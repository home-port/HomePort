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
 * @file hpd_device_configuration.c
 * @brief  Methods for managing the Configuration of Devices at runtime
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include "hpd_device_configuration.h"
#include "hpd_error.h"

/**
 * 
 */
Attribute *
new_attribute(char *name, char *value)
{
	if(!name || !value) return NULL;
	Attribute *attribute = (Attribute*)malloc(sizeof(Attribute));

	attribute->name = malloc(sizeof(char)*(strlen(name)+1));
	strcpy(attribute->name, name);

	attribute->value = malloc(sizeof(char)*(strlen(value)+1));
	strcpy(attribute->value, value);

	attribute->prev = NULL;
	attribute->next=NULL;

	return attribute;
}

/**
 *
 */
int
destroy_attribute(Attribute *attribute_to_destroy)
{
	if(!attribute_to_destroy) return -1;//TO BE CHANGED
	if(attribute_to_destroy-> name) free(attribute_to_destroy-> name);
	if(attribute_to_destroy-> value) free(attribute_to_destroy-> value);
	free(attribute_to_destroy);
	return HPD_E_SUCCESS;
}

/**
 *
 */
configurationXML *
new_configuration_xml()
{
	configurationXML *conf = (configurationXML*)malloc(sizeof(configurationXML));
	conf-> device_selector_head = NULL;
	conf-> device_attribute_head = NULL;
	conf-> service_selector_head = NULL;
	conf-> service_attribute_head = NULL;
	conf-> parameter_selector_head = NULL;
	conf-> parameter_attribute_head = NULL;
	conf-> device_head = NULL;
	conf-> service_head = NULL;
	conf-> parameter_head = NULL;

	return conf;
}

/**
 *
 */
int destroy_configuration_xml(configurationXML *configuration_xml_to_destroy)
{
	if(!configuration_xml_to_destroy) return -1;
	Attribute *iterator;
	Attribute *temp;
	configurationEntity *ce_iterator;
	configurationEntity *ce_temp;

	if(configuration_xml_to_destroy-> device_selector_head)
	{
		DL_FOREACH_SAFE(configuration_xml_to_destroy-> device_selector_head, iterator, temp)
		{
			DL_DELETE(configuration_xml_to_destroy-> device_selector_head, iterator);
			destroy_attribute(iterator);
			iterator = NULL;
		}
	}

	if(configuration_xml_to_destroy-> service_selector_head)
	{
		DL_FOREACH_SAFE(configuration_xml_to_destroy-> service_selector_head, iterator, temp)
		{
			DL_DELETE(configuration_xml_to_destroy-> service_selector_head, iterator);
			destroy_attribute(iterator);
			iterator = NULL;
		}
	}

	if(configuration_xml_to_destroy-> parameter_selector_head)
	{
		DL_FOREACH_SAFE(configuration_xml_to_destroy-> parameter_selector_head, iterator, temp)
		{
			DL_DELETE(configuration_xml_to_destroy-> parameter_selector_head, iterator);
			destroy_attribute(iterator);
			iterator = NULL;
		}
	}

	if(configuration_xml_to_destroy-> device_attribute_head)
	{
		DL_FOREACH_SAFE(configuration_xml_to_destroy-> device_attribute_head, iterator, temp)
		{
			DL_DELETE(configuration_xml_to_destroy-> device_attribute_head, iterator);
			destroy_attribute(iterator);
			iterator = NULL;
		}
	}

	if(configuration_xml_to_destroy-> service_attribute_head)
	{
		DL_FOREACH_SAFE(configuration_xml_to_destroy-> service_attribute_head, iterator, temp)
		{
			DL_DELETE(configuration_xml_to_destroy-> service_attribute_head, iterator);
			destroy_attribute(iterator);
			iterator = NULL;
		}
	}

	if(configuration_xml_to_destroy-> parameter_attribute_head)
	{
		DL_FOREACH_SAFE(configuration_xml_to_destroy-> parameter_attribute_head, iterator, temp)
		{
			DL_DELETE(configuration_xml_to_destroy-> parameter_attribute_head, iterator);
			destroy_attribute(iterator);
			iterator = NULL;
		}
	}

	if(configuration_xml_to_destroy-> device_head)
	{
		DL_FOREACH_SAFE(configuration_xml_to_destroy-> device_head, ce_iterator, ce_temp)
		{
			DL_DELETE(configuration_xml_to_destroy-> device_head, ce_iterator);
			destroy_configuration_entity(ce_iterator);
			ce_iterator = NULL;
			ce_temp = NULL;
		}
	}

	if(configuration_xml_to_destroy-> service_head)
	{
		DL_FOREACH_SAFE(configuration_xml_to_destroy-> service_head, ce_iterator, ce_temp)
		{
			DL_DELETE(configuration_xml_to_destroy-> service_head, ce_iterator);
			destroy_configuration_entity(ce_iterator);
			ce_iterator = NULL;
			ce_temp = NULL;
		}
	}

	if(configuration_xml_to_destroy-> parameter_head)
	{
		DL_FOREACH_SAFE(configuration_xml_to_destroy-> parameter_head, ce_iterator, ce_temp)
		{
			DL_DELETE(configuration_xml_to_destroy-> parameter_head, ce_iterator);
			destroy_configuration_entity(ce_iterator);
			ce_iterator = NULL;
			ce_temp = NULL;
		}
	}

	free(configuration_xml_to_destroy);

	return HPD_E_SUCCESS;
}

/**
 * 
 */
configurationEntity *new_configuration_entity(void* entity)
{
	configurationEntity *configuration_entity = (configurationEntity *)malloc(sizeof(configurationEntity));
	configuration_entity-> entity = entity;
	configuration_entity-> next = NULL;
	configuration_entity-> prev = NULL;
	return configuration_entity;
}

/**
 * 
 */
int destroy_configuration_entity(configurationEntity *configuration_entity_to_destroy)
{
	configuration_entity_to_destroy-> entity = NULL;
	free(configuration_entity_to_destroy);
	return HPD_E_SUCCESS;
}

/**
 *
 */
int is_legit_device(Device *device, Attribute *attribute_head)
{
	Attribute *attribute;
	DL_FOREACH(attribute_head, attribute)
	{
		if(strcmp(attribute->name, "desc") == 0 && device->description)
			if(strcmp(attribute-> value, device->description) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "id") == 0 && device->ID)
			if(strcmp(attribute-> value, device->ID) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "vendorid") == 0 && device->vendorID)
			if(strcmp(attribute-> value, device->vendorID) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "productid") == 0 && device->productID)
			if(strcmp(attribute-> value, device->productID) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "version") == 0 && device->version)
			if(strcmp(attribute-> value, device->version) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "ip") == 0 && device->IP)
			if(strcmp(attribute-> value, device->IP) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "port") == 0 && device->port)
			if(strcmp(attribute-> value, device->port) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "location") == 0 && device->location)
			if(strcmp(attribute-> value, device->location) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "type") == 0 && device->type)
			if(strcmp(attribute-> value, device->type) != 0) 
				return HPD_NO;
	}

	return HPD_YES;
}

/**
 *
 */
int is_legit_service(Service *service, Attribute *attribute_head)
{
	Attribute *attribute;
	DL_FOREACH(attribute_head, attribute)
	{
		if(strcmp(attribute->name, "desc") == 0 && service->description)
			if(strcmp(attribute-> value, service->description) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "id") == 0 && service->ID)
			if(strcmp(attribute-> value, service->ID) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "value_url") == 0 && service->value_url)
			if(strcmp(attribute-> value, service->value_url) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "type") == 0 && service->type)
			if(strcmp(attribute-> value, service->type) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "unit") == 0 && service->unit)
			if(strcmp(attribute-> value, service->unit) != 0) 
				return HPD_NO;
	}

	return HPD_YES;
}

/**
 *
 */
int is_legit_parameter(Parameter *parameter, Attribute *attribute_head)
{
	Attribute *attribute;
	DL_FOREACH(attribute_head, attribute)
	{
		if(strcmp(attribute->name, "id") == 0 && parameter->ID)
			if(strcmp(attribute-> value, parameter->ID) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "min") == 0 && parameter->min)
			if(strcmp(attribute-> value, parameter->min) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "max") == 0 && parameter->max)
			if(strcmp(attribute-> value, parameter->max) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "scale") == 0 && parameter->scale)
			if(strcmp(attribute-> value, parameter->scale) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "step") == 0 && parameter->step)
			if(strcmp(attribute-> value, parameter->step) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "type") == 0 && parameter->type)
			if(strcmp(attribute-> value, parameter->type) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "unit") == 0 && parameter->unit)
			if(strcmp(attribute-> value, parameter->unit) != 0) 
				return HPD_NO;

		if(strcmp(attribute->name, "values") == 0 && parameter->values)
			if(strcmp(attribute-> value, parameter->values) != 0) 
				return HPD_NO;
	}

	return HPD_YES;
}

int is_device_in_list(configurationXML *configure, Device *device)
{
	configurationEntity *dev_iterator = NULL;
	DL_FOREACH(configure->device_head, dev_iterator)
	{
		if(strcmp(((Device *)dev_iterator->entity)->ID, device-> ID) == 0
			&& strcmp(((Device *)dev_iterator->entity)->type, device-> type) == 0)
		{
			return HPD_YES;
		}
	}
	return HPD_NO;
}
int is_service_in_list(configurationXML *configure, Service *service)
{
	configurationEntity *ser_iterator = NULL;
	DL_FOREACH(configure->service_head, ser_iterator)
	{
		if(strcmp(((Service *)ser_iterator->entity)->ID, service-> ID) == 0
			&& strcmp(((Service *)ser_iterator->entity)->type, service-> type) == 0)
		{
			return HPD_YES;
		}
	}
	return HPD_NO;
}
int is_parameter_in_list(configurationXML *configure, Service *service)
{
	configurationEntity *par_iterator = NULL;
	DL_FOREACH(configure->parameter_head, par_iterator)
	{
		if(strcmp(((Service *)par_iterator->entity)->ID, service-> ID) == 0
			&& strcmp(((Service *)par_iterator->entity)->type, service-> type) == 0)
		{
			return HPD_YES;
		}
	}
	return HPD_NO;
}

/**
 * 
 */
int 
manage_configuration_xml( char *configuration_xml , ServiceElement *service_head)
{

	mxml_node_t *xml = NULL; 
	mxml_node_t *selector_node = NULL;
	mxml_node_t *selected_node = NULL;
	mxml_node_t *node = NULL;
	ServiceElement *iterator = NULL;
	ServiceElement *iterator2 = NULL;
	ServiceElement *iterator3 = NULL;
	Parameter *parameter = NULL;
	Attribute *att_iterator = NULL;
	Service *new_service = NULL;
	Service *new_parameter = NULL;
	configurationEntity *par_iterator = NULL;
	configurationEntity *ser_iterator = NULL;
	configurationEntity *dev_iterator = NULL;
	configurationEntity *newConfEntity = NULL;
	int is_legit = HPD_YES;
	int i=0;

	configurationXML * configure = new_configuration_xml();

	xml = mxmlLoadString(NULL, configuration_xml, MXML_NO_CALLBACK);
	if( !xml ) goto fail;

	selector_node = mxmlFindElement(xml, xml,"selector", NULL, NULL, MXML_DESCEND);
	if( !selector_node ) goto fail;

	selected_node = mxmlFindElement(xml, xml,"selected", NULL, NULL, MXML_DESCEND);
	if( !selected_node ) goto fail;

	/**
	 * The Selectors
	 */
	node = mxmlWalkNext(selector_node, selector_node, MXML_DESCEND);
	if( !node || !node->value.element.name) goto fail;


	if(strcmp(node->value.element.name, "device") == 0)
	{
		if( retrieve_selectors(node, configure, HPD_IS_DEVICE) != HPD_E_SUCCESS) goto fail;

		node = mxmlWalkNext(node, selector_node, MXML_DESCEND);
		if( !node ) goto selected;
		
		if(strcmp(node->value.element.name, "service") == 0)
		{
			if( retrieve_selectors(node, configure, HPD_IS_SERVICE) != HPD_E_SUCCESS) 
				goto fail;
		}
		else goto fail;

		node = mxmlWalkNext(node, selector_node, MXML_DESCEND);
		if( !node ) goto selected;

		if(strcmp(node->value.element.name, "parameter") == 0)
		{
			if( retrieve_selectors(node, configure, HPD_IS_PARAMETER) != HPD_E_SUCCESS) 
				goto fail;
		}
		else goto fail;
		
	}
	else if(strcmp(node->value.element.name, "service") == 0)
	{
		if( retrieve_selectors(node, configure, HPD_IS_SERVICE) != HPD_E_SUCCESS) goto fail;
		
		node = mxmlWalkNext(node, selector_node, MXML_DESCEND);
		if( !node ) goto selected;

		if(strcmp(node->value.element.name, "parameter") == 0)
		{
			if( retrieve_selectors(node, configure, HPD_IS_PARAMETER) != HPD_E_SUCCESS) 
				goto fail;
		}
		else goto fail;
	}
	else if(strcmp(node->value.element.name, "parameter") == 0)
	{
		if( retrieve_selectors(node, configure, HPD_IS_PARAMETER) != HPD_E_SUCCESS) 
			goto fail;
	}
	else goto fail;

	/**
	 * The Selected
	 */
	selected :

	node = mxmlWalkNext(selected_node, selected_node, MXML_DESCEND);
	if( !node ) goto fail;

	if(strcmp(node->value.element.name, "device") == 0)
	{
		if( retrieve_attributes(node, configure, HPD_IS_DEVICE) != HPD_E_SUCCESS) goto fail;

		node = mxmlWalkNext(node, selected_node, MXML_DESCEND);
		if( !node ) goto fill;

		if(strcmp(node->value.element.name, "service") == 0)
		{
			if( retrieve_attributes(node, configure, HPD_IS_SERVICE) != HPD_E_SUCCESS) 
				goto fail;
		}
		else goto fail;

		node = mxmlWalkNext(node, selected_node, MXML_DESCEND);
		if( !node ) goto fill;

		if(strcmp(node->value.element.name, "parameter") == 0)
		{
			if( retrieve_attributes(node, configure, HPD_IS_PARAMETER) != HPD_E_SUCCESS) 
				goto fail;
		}
		else goto fail;
	}
	else if(strcmp(node->value.element.name, "service") == 0)
	{
		if( retrieve_attributes(node, configure, HPD_IS_SERVICE) != HPD_E_SUCCESS) goto fail;
		
		node = mxmlWalkNext(node, selected_node, MXML_DESCEND);
		if( !node ) goto fill;

		if(strcmp(node->value.element.name, "parameter") == 0)
		{
			if( retrieve_attributes(node, configure, HPD_IS_PARAMETER) != HPD_E_SUCCESS) 
				goto fail;
		}
		else goto fail;
	}
	else if(strcmp(node->value.element.name, "parameter") == 0)
	{
		if( retrieve_attributes(node, configure, HPD_IS_PARAMETER) != HPD_E_SUCCESS) 
			goto fail;
	}
	else goto fail;

	fill:

	/**
	 *	Fill the lists
	*/	
	if(configure->device_attribute_head)
	{
		is_legit = HPD_YES;
		DL_FOREACH(service_head, iterator)
		{
			if(is_device_in_list(configure, iterator-> service-> device) == HPD_YES)
				is_legit = HPD_NO;

			/* If we have selectors of device */
			if(configure->device_selector_head && is_legit == HPD_YES)
			{
				is_legit = is_legit_device(iterator-> service-> device, configure->device_selector_head);
			}

			/* If we have selectors of service */
			if(configure->service_selector_head && is_legit == HPD_YES)
			{
				DL_FOREACH(iterator-> service-> device-> service_head, iterator2)
				{
					is_legit = is_legit_service(iterator2-> service, configure->service_selector_head);
					if(is_legit == HPD_YES)
						break;
				}
			}
			/* If we have selectors of parameter */
			if(configure->parameter_selector_head && is_legit == HPD_YES)
			{
				DL_FOREACH(iterator-> service-> device-> service_head, iterator3)
				{
					is_legit = is_legit_parameter(iterator3-> service-> parameter, configure->parameter_selector_head);
					if(is_legit == HPD_YES)
						break;
				}
			}
			/* If the Device is legit we add it to the list */
			if(is_legit == HPD_YES)
			{
				newConfEntity = new_configuration_entity((Device *)iterator->service-> device);
				DL_APPEND(configure->device_head, newConfEntity);
			}
		}
	}

	/**
	 *	Fill the service list
	*/	

	if(configure->service_attribute_head)
	{
		is_legit = HPD_YES;
		DL_FOREACH(service_head, iterator)
		{
			if(is_service_in_list(configure, iterator-> service) == HPD_YES)
				is_legit = HPD_NO;

			/* If we have selectors of device */
			if(configure->device_selector_head && is_legit == HPD_YES)
				is_legit = is_legit_device(iterator-> service-> device, configure->device_selector_head);

			/* If we have selectors of service */
			if(configure->service_selector_head && is_legit == HPD_YES)
				is_legit = is_legit_service(iterator-> service, configure->service_selector_head);

			/* If we have selectors of parameter */
			if(configure->parameter_selector_head && is_legit == HPD_YES)
				is_legit = is_legit_parameter(iterator-> service-> parameter, configure->parameter_selector_head);

			/* If the Service is legit we add it to the list */
			if(is_legit == HPD_YES)
			{
				newConfEntity = new_configuration_entity((Service *)iterator->service);
				DL_APPEND(configure->service_head, newConfEntity);
			}
		}
	}

	/**
	 *	Fill the service list
	*/	

	if(configure->parameter_attribute_head)
	{
		is_legit = HPD_YES;
		DL_FOREACH(service_head, iterator)
		{
			if(is_parameter_in_list(configure, iterator-> service) == HPD_YES)
				is_legit = HPD_NO;

			/* If we have selectors of device */
			if(configure->device_selector_head && is_legit == HPD_YES)
				is_legit = is_legit_device(iterator-> service-> device, configure->device_selector_head);

			/* If we have selectors of service */
			if(configure->service_selector_head && is_legit == HPD_YES)
				is_legit = is_legit_service(iterator-> service, configure->service_selector_head);

			/* If we have selectors of parameter */
			if(configure->parameter_selector_head && is_legit == HPD_YES)
				is_legit = is_legit_parameter(iterator-> service-> parameter, configure->parameter_selector_head);

			/* If the Service is legit we add it to the list */
			if(is_legit == HPD_YES)
			{
				newConfEntity = new_configuration_entity((Service *)iterator->service);
				DL_APPEND(configure->parameter_head, newConfEntity);
			}
		}
	}
	
	if(configure->service_head)
		modify_services(configure);
	if(configure->device_head)
		modify_devices(configure);
	if(configure->parameter_head)
		modify_parameters(configure);


	/**
	 *	Success or Fail
	*/
	success :
		if(xml) mxmlDelete(xml);
		if(configure) destroy_configuration_xml(configure);
		return HPD_E_SUCCESS;

	fail :
		if(xml) mxmlDelete(xml);
		if(configure) destroy_configuration_xml(configure);
		printf("An Error Occured while managing the configuration XML\n");
		return HPD_E_XML_ERROR;
}

/**
 *
 */
int modify_services(configurationXML *configure)
{
	configurationEntity *iterator = NULL;
	Attribute *att_iterator = NULL;
	LL_FOREACH(configure->service_head, iterator)
	{
		LL_FOREACH(configure->service_attribute_head, att_iterator)
		{
			if(strcmp(att_iterator->name, "desc") == 0)
			{
				((Service *)iterator->entity)->description = realloc(((Service *)iterator->entity)->description,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Device *)iterator->entity)->description, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "unit") == 0)
			{
				((Service *)iterator->entity)->unit = realloc(((Service *)iterator->entity)->unit,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Service *)iterator->entity)->unit, att_iterator->value);
			}
		}
		update_service_xml((Service *)iterator->entity);
	}
}

/**
 *
 */
int modify_devices(configurationXML *configure)
{
	configurationEntity *iterator = NULL;
	Attribute *att_iterator = NULL;
	LL_FOREACH(configure->device_head, iterator)
	{
		LL_FOREACH(configure->device_attribute_head, att_iterator)
		{
			if(strcmp(att_iterator->name, "desc") == 0)
			{
				((Device *)iterator->entity)->description = realloc(((Device *)iterator->entity)->description,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Device *)iterator->entity)->description, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "vendorid") == 0)
			{
				((Device *)iterator->entity)->vendorID = realloc(((Device *)iterator->entity)->vendorID,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Device *)iterator->entity)->vendorID, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "productid") == 0)
			{
				((Device *)iterator->entity)->productID = realloc(((Device *)iterator->entity)->productID,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Device *)iterator->entity)->productID, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "version") == 0)
			{
				((Device *)iterator->entity)->version = realloc(((Device *)iterator->entity)->version,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Device *)iterator->entity)->version, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "ip") == 0)
			{
				((Device *)iterator->entity)->IP = realloc(((Device *)iterator->entity)->IP,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Device *)iterator->entity)->IP, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "port") == 0)
			{
				((Device *)iterator->entity)->port = realloc(((Device *)iterator->entity)->port,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Device *)iterator->entity)->port, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "location") == 0)
			{
				((Device *)iterator->entity)->location = realloc(((Device *)iterator->entity)->location,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Device *)iterator->entity)->location, att_iterator->value);
			}
		}
		update_device_xml((Device *)iterator->entity);
	}
}

/**
 *
 */
int modify_parameters(configurationXML *configure)
{
	configurationEntity *iterator = NULL;
	Attribute *att_iterator = NULL;
	LL_FOREACH(configure->parameter_head, iterator)
	{
		LL_FOREACH(configure->parameter_attribute_head, att_iterator)
		{
			if(strcmp(att_iterator->name, "max") == 0)
			{
				((Service *)iterator->entity)->parameter->max = realloc(((Service *)iterator->entity)->parameter->max,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Service *)iterator->entity)->parameter->max, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "min") == 0)
			{
				((Service *)iterator->entity)->parameter->min = realloc(((Service *)iterator->entity)->parameter->min,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Service *)iterator->entity)->parameter->min, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "scale") == 0)
			{
				((Service *)iterator->entity)->parameter->scale = realloc(((Service *)iterator->entity)->parameter->scale,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Service *)iterator->entity)->parameter->scale, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "step") == 0)
			{
				((Service *)iterator->entity)->parameter->step = realloc(((Service *)iterator->entity)->parameter->step,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Service *)iterator->entity)->parameter->step, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "unit") == 0)
			{
				((Service *)iterator->entity)->parameter->unit = realloc(((Service *)iterator->entity)->parameter->unit,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Service *)iterator->entity)->parameter->unit, att_iterator->value);
			}
			if(strcmp(att_iterator->name, "values") == 0)
			{
				((Service *)iterator->entity)->parameter->values = realloc(((Service *)iterator->entity)->parameter->values,sizeof(char)*(strlen(att_iterator->value) + 1));
				strcpy(((Service *)iterator->entity)->parameter->values, att_iterator->value);
			}
		}
		update_parameter_xml((Service *)iterator->entity);
	}
}

/**
 * 
 */
int
retrieve_attributes(mxml_node_t *node, configurationXML * configure, int type)
{
	switch(type)
	{
		case HPD_IS_DEVICE :
			if(mxmlElementGetAttr(node,"desc"))
				DL_APPEND(configure-> device_attribute_head, new_attribute("desc", mxmlElementGetAttr( node,"desc" ) ) );
			if(mxmlElementGetAttr(node,"vendorid"))
				DL_APPEND(configure-> device_attribute_head, new_attribute("vendorid", mxmlElementGetAttr( node,"vendorid" ) ) );
			if(mxmlElementGetAttr(node,"productid"))
				DL_APPEND(configure-> device_attribute_head, new_attribute("productid", mxmlElementGetAttr( node,"productid" ) ) );
			if(mxmlElementGetAttr(node,"version"))
				DL_APPEND(configure-> device_attribute_head, new_attribute("version", mxmlElementGetAttr( node,"version" ) ) );
			if(mxmlElementGetAttr(node,"ip"))
				DL_APPEND(configure-> device_attribute_head, new_attribute("ip", mxmlElementGetAttr( node,"ip" ) ) );
			if(mxmlElementGetAttr(node,"port"))
				DL_APPEND(configure-> device_attribute_head, new_attribute("port", mxmlElementGetAttr( node,"port" ) ) );
			if(mxmlElementGetAttr(node,"location"))
				DL_APPEND(configure-> device_attribute_head, new_attribute("location", mxmlElementGetAttr( node,"location" ) ) );
			break;
		case HPD_IS_SERVICE:
			if(mxmlElementGetAttr(node,"desc"))
				DL_APPEND(configure-> service_attribute_head, new_attribute("desc", mxmlElementGetAttr( node,"desc" ) ) );
			if(mxmlElementGetAttr(node,"unit"))
				DL_APPEND(configure-> service_attribute_head, new_attribute("unit", mxmlElementGetAttr( node,"unit" ) ) );
			break;
		case HPD_IS_PARAMETER:
			if(mxmlElementGetAttr(node,"max"))
				DL_APPEND(configure-> parameter_attribute_head, new_attribute("max", mxmlElementGetAttr( node,"max" ) ) );
			if(mxmlElementGetAttr(node,"min"))
				DL_APPEND(configure-> parameter_attribute_head, new_attribute("min", mxmlElementGetAttr( node,"min" ) ) );
			if(mxmlElementGetAttr(node,"scale"))
				DL_APPEND(configure-> parameter_attribute_head, new_attribute("scale", mxmlElementGetAttr( node,"scale" ) ) );
			if(mxmlElementGetAttr(node,"step"))
				DL_APPEND(configure-> parameter_attribute_head, new_attribute("step", mxmlElementGetAttr( node,"step" ) ) );
			if(mxmlElementGetAttr(node,"unit"))
				DL_APPEND(configure-> parameter_attribute_head, new_attribute("unit", mxmlElementGetAttr( node,"unit" ) ) );
			if(mxmlElementGetAttr(node,"values"))
				DL_APPEND(configure-> parameter_attribute_head, new_attribute("values", mxmlElementGetAttr( node,"values" ) ) );
			break;
		default:
			return -1;
			break;
	}
	return HPD_E_SUCCESS;
}

/**
 * 
 */
int 
retrieve_selectors(mxml_node_t *node, configurationXML * configure, int type)
{
	switch(type)
	{
		case HPD_IS_DEVICE :
			if(mxmlElementGetAttr(node,"desc"))
				DL_APPEND(configure-> device_selector_head, new_attribute("desc", mxmlElementGetAttr( node,"desc" ) ) );
			if(mxmlElementGetAttr(node,"id"))
				DL_APPEND(configure-> device_selector_head, new_attribute("id", mxmlElementGetAttr( node,"id" ) ) );
			if(mxmlElementGetAttr(node,"vendorid"))
				DL_APPEND(configure-> device_selector_head, new_attribute("vendorid", mxmlElementGetAttr( node,"vendorid" ) ) );
			if(mxmlElementGetAttr(node,"productid"))
				DL_APPEND(configure-> device_selector_head, new_attribute("productid", mxmlElementGetAttr( node,"productid" ) ) );
			if(mxmlElementGetAttr(node,"version"))
				DL_APPEND(configure-> device_selector_head, new_attribute("version", mxmlElementGetAttr( node,"version" ) ) );
			if(mxmlElementGetAttr(node,"ip"))
				DL_APPEND(configure-> device_selector_head, new_attribute("ip", mxmlElementGetAttr( node,"ip" ) ) );
			if(mxmlElementGetAttr(node,"port"))
				DL_APPEND(configure-> device_selector_head, new_attribute("port", mxmlElementGetAttr( node,"port" ) ) );
			if(mxmlElementGetAttr(node,"location"))
				DL_APPEND(configure-> device_selector_head, new_attribute("location", mxmlElementGetAttr( node,"location" ) ) );
			if(mxmlElementGetAttr(node,"type"))
				DL_APPEND(configure-> device_selector_head, new_attribute("type", mxmlElementGetAttr( node,"type" ) ) );
			break;
		case HPD_IS_SERVICE:
			if(mxmlElementGetAttr(node,"desc"))
				DL_APPEND(configure-> service_selector_head, new_attribute("desc", mxmlElementGetAttr( node,"desc" ) ) );
			if(mxmlElementGetAttr(node,"id"))
				DL_APPEND(configure-> service_selector_head, new_attribute("id", mxmlElementGetAttr( node,"id" ) ) );
			if(mxmlElementGetAttr(node,"value_url"))
				DL_APPEND(configure-> service_selector_head, new_attribute("value_url", mxmlElementGetAttr( node,"value_url" ) ) );
			if(mxmlElementGetAttr(node,"type"))
				DL_APPEND(configure-> service_selector_head, new_attribute("type", mxmlElementGetAttr( node,"type" ) ) );
			if(mxmlElementGetAttr(node,"unit"))
				DL_APPEND(configure-> service_selector_head, new_attribute("unit", mxmlElementGetAttr( node,"unit" ) ) );
			break;
		case HPD_IS_PARAMETER:
			if(mxmlElementGetAttr(node,"id"))
				DL_APPEND(configure-> parameter_selector_head, new_attribute("id", mxmlElementGetAttr( node,"id" ) ) );
			if(mxmlElementGetAttr(node,"max"))
				DL_APPEND(configure-> parameter_selector_head, new_attribute("max", mxmlElementGetAttr( node,"max" ) ) );
			if(mxmlElementGetAttr(node,"min"))
				DL_APPEND(configure-> parameter_selector_head, new_attribute("min", mxmlElementGetAttr( node,"min" ) ) );
			if(mxmlElementGetAttr(node,"scale"))
				DL_APPEND(configure-> parameter_selector_head, new_attribute("scale", mxmlElementGetAttr( node,"scale" ) ) );
			if(mxmlElementGetAttr(node,"step"))
				DL_APPEND(configure-> parameter_selector_head, new_attribute("step", mxmlElementGetAttr( node,"step" ) ) );
			if(mxmlElementGetAttr(node,"type"))
				DL_APPEND(configure-> parameter_selector_head, new_attribute("type", mxmlElementGetAttr( node,"type" ) ) );
			if(mxmlElementGetAttr(node,"unit"))
				DL_APPEND(configure-> parameter_selector_head, new_attribute("unit", mxmlElementGetAttr( node,"unit" ) ) );
			if(mxmlElementGetAttr(node,"values"))
				DL_APPEND(configure-> parameter_selector_head, new_attribute("values", mxmlElementGetAttr( node,"values" ) ) );
			break;
		default:
			return -1;
			break;
	}
	return HPD_E_SUCCESS;
}



