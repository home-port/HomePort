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
 * @file hpd_xml.c
 * @brief  Methods for managing XML
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include "hpd_xml.h" 
#include "hpd_error.h"

serviceXmlFile *service_xml_file = NULL;

/**
 * Creates the serviceXmlFile structure
 *
 * @return void
 */
void 
create_service_xml_file()
{
	service_xml_file = (serviceXmlFile*)malloc(sizeof(serviceXmlFile));
	service_xml_file->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(service_xml_file->mutex, NULL);
}

/**
 * Destroys the serviceXmlFile structure
 *
 * @return void
 */
void 
destroy_service_xml_file()
{
	if(service_xml_file)
	{
		mxmlDelete(service_xml_file->xml_tree);
		if(service_xml_file->mutex)
		{
			pthread_mutex_destroy(service_xml_file->mutex);
			free(service_xml_file->mutex);
		}
		free(service_xml_file);
	}
}

/**
 * Saves the XML internal to the actual File
 *
 * @return void
 */
void 
save_xml_tree()
{
	pthread_mutex_lock(service_xml_file->mutex);
	service_xml_file->fp = fopen(XML_FILE_NAME, "w");
	if(service_xml_file->fp == NULL)
	{
		pthread_mutex_unlock(service_xml_file->mutex);
		printf("Impossible to open the XML file\n");
		return HPD_E_FAILED_TO_OPEN_FILE;
	}
	mxmlSaveFile(service_xml_file->xml_tree, service_xml_file->fp, MXML_NO_CALLBACK);
	fclose(service_xml_file->fp);
	pthread_mutex_unlock(service_xml_file->mutex);
}

/**
 * Initialization of the services.xml file that will be used
 * throughout the whole process
 *
 * @param name the name of the devicelist
 * @param id the id of the devicelist
 *
 * @return returns HPD_E_SUCCESS if successful
 */
int 
init_xml_file(char *name, char *id)
{

	create_service_xml_file();

	mxml_node_t *devicelist;   

	service_xml_file->xml_tree = mxmlNewXML("1.0");
	devicelist = mxmlNewElement(service_xml_file->xml_tree, "devicelist");
	mxmlElementSetAttr(devicelist, "name", name);
	mxmlElementSetAttr(devicelist, "id", id);
	mxmlElementSetAttr(devicelist, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	                   mxmlElementSetAttr(devicelist, "xsi:noNamespaceSchemaLocation", "http://cs.au.dk/dithus/xml/devicedescription.xsd");

	                                      save_xml_tree();
	                                      return HPD_E_SUCCESS;
                                      }

/**
 * Checks if the Device is already in the XML file
 *
 * @param device the device that we want to check
 *
 * @return returns HPD_YES if the device is in the XML file and HPD_NO if it is not
 */
int 
device_is_in_xml_file(Device *device)
{
	mxml_node_t *xml_device;

	for (xml_device = mxmlFindElement(service_xml_file->xml_tree, 
						service_xml_file->xml_tree,
						"device", 
						NULL, 
						NULL, 
						MXML_DESCEND);
	     xml_device != NULL;
	     xml_device = mxmlFindElement(	xml_device, 
										service_xml_file->xml_tree,
										"device", 
										NULL, 
										NULL, 
										MXML_DESCEND))
	{
		if(strcmp(mxmlElementGetAttr(xml_device,"type") , device->type) == 0
		   && strcmp(mxmlElementGetAttr(xml_device,"id") , device->ID) == 0)
		{
			return HPD_YES;
		}
	}

	return HPD_NO;
}

/**
 * Checks if the Service is already in the XML file
 *
 * @param service the service that we want to check
 *
 * @return returns HPD_NO if the device is not in the XML file and HPD_YES if it is
 */
int 
service_is_in_xml_file(Service *service)
{
	if( device_is_in_xml_file(service->device) == HPD_NO )
		return HPD_NO;

	mxml_node_t *device = get_xml_node_of_device(service->device);
	if(device == NULL)
	{
		printf("Error while retrieving device node\n");
		return HPD_E_IMPOSSIBLE_TO_RETRIEVE_DEVICE_XML_NODE;
	}

	mxml_node_t *xml_service;

	for (device = mxmlFindElement(service_xml_file->xml_tree, 
									service_xml_file->xml_tree,
									"device", 
									NULL, 
									NULL, 
									MXML_DESCEND);
	     device != NULL;
	     device = mxmlFindElement(	device, 
									service_xml_file->xml_tree, 
									"device", 
									NULL, 
									NULL, 
									MXML_DESCEND))
	{
		if(strcmp(mxmlElementGetAttr(device,"type") , service->device->type) == 0
		   && strcmp(mxmlElementGetAttr(device,"id") , service->device->ID) == 0)
		{
			for (xml_service = mxmlFindElement(	device, 
													device,
													"service", 
													NULL, 
													NULL, 
													MXML_DESCEND);
			     xml_service != NULL;
			     xml_service = mxmlFindElement(	xml_service, 
												device, 
												"service", 
												NULL, 
												NULL, 
												MXML_DESCEND))
			{
				if(strcmp(mxmlElementGetAttr(xml_service,"type") , service->type) == 0
				   && strcmp(mxmlElementGetAttr(xml_service,"id") , service->ID) == 0)
				{
					return HPD_YES;
				}
			}
		}
	}

	return HPD_NO;
}

/**
 * Adds a specific device to the XML document
 *
 * @param device_to_add The device to add
 *
 * @return returns HPD_E_SUCCESS if successful and HPD_E_DEVICE_ALREADY_IN_XML or HPD_E_XML_ERROR  if failed
 */
int 
add_device_to_xml(Device *device_to_add)
{

	if(device_is_in_xml_file (device_to_add) == HPD_YES)
		return HPD_E_DEVICE_ALREADY_IN_XML;

	mxml_node_t *devicelist;
	mxml_node_t *new_device;

	devicelist = mxmlFindElement(service_xml_file->xml_tree, service_xml_file->xml_tree, "devicelist", NULL, NULL, MXML_DESCEND);
	if(devicelist == NULL)
	{
		printf("No \"devicelist\" in the XML file\n");
		return HPD_E_XML_ERROR;
	}

	new_device = mxmlNewElement(devicelist, "device");
	if(device_to_add->description != NULL) mxmlElementSetAttr(new_device, "desc", device_to_add->description);
	if(device_to_add->ID != NULL) mxmlElementSetAttr(new_device, "id", device_to_add->ID);
	if(device_to_add->vendorID != NULL) mxmlElementSetAttr(new_device, "vendorID", device_to_add->vendorID);
	if(device_to_add->productID != NULL) mxmlElementSetAttr(new_device, "productID", device_to_add->productID);
	if(device_to_add->version != NULL) mxmlElementSetAttr(new_device, "version", device_to_add->version);
	if(device_to_add->IP != NULL) mxmlElementSetAttr(new_device, "ip", device_to_add->IP);
	if(device_to_add->port != NULL) mxmlElementSetAttr(new_device, "port", device_to_add->port);
	if(device_to_add->location != NULL) mxmlElementSetAttr(new_device, "location", device_to_add->location);
	if(device_to_add->type != NULL) mxmlElementSetAttr(new_device, "type", device_to_add->type);

	save_xml_tree (); 

	return HPD_E_SUCCESS;
}

/**
 * Gets the internal node for a given Device
 *
 * @param _device The device concerned
 *
 * @return returns the mxml_node_t corresponding to the Device of NULL if not in the XML file
 */
mxml_node_t *
get_xml_node_of_device(Device *_device)
{
	mxml_node_t *device;

	for (device = mxmlFindElement(service_xml_file->xml_tree, service_xml_file->xml_tree,"device", NULL, NULL, MXML_DESCEND);
	     device != NULL;
	     device = mxmlFindElement(device, service_xml_file->xml_tree, "device", NULL, NULL, MXML_DESCEND))
	{
		if(strcmp(mxmlElementGetAttr(device,"type") , _device->type) == 0
		   && strcmp(mxmlElementGetAttr(device,"id") , _device->ID) == 0)
		{
			return device;
		}
	}
	return NULL;
}

/**
 * Gets the internal node for a given Service
 *
 * @param _service The device concerned
 *
 * @return returns the mxml_node_t corresponding to the Service of NULL if not in the XML file
 */
mxml_node_t *
get_xml_node_of_service(Service *_service)
{
	if( device_is_in_xml_file(_service->device) == HPD_NO )
		return NULL;

	mxml_node_t *device = get_xml_node_of_device(_service->device);
	if(device == NULL)
	{
		printf("Error while retrieving device node\n");
		return NULL;
	}

	mxml_node_t *service;

	for (device = mxmlFindElement(service_xml_file->xml_tree, service_xml_file->xml_tree,"device", NULL, NULL, MXML_DESCEND);
	     device != NULL;
	     device = mxmlFindElement(device, service_xml_file->xml_tree, "device", NULL, NULL, MXML_DESCEND))
	{
		if(strcmp(mxmlElementGetAttr(device,"type") , _service->device->type) == 0
		   && strcmp(mxmlElementGetAttr(device,"id") , _service->device->ID) == 0)
		{
			for (service = mxmlFindElement(device, device,"service", NULL, NULL, MXML_DESCEND);
			     service != NULL;
			     service = mxmlFindElement(service, device, "service", NULL, NULL, MXML_DESCEND))
			{
				if(strcmp(mxmlElementGetAttr(service,"type") , _service->type) == 0
				   && strcmp(mxmlElementGetAttr(service,"id") , _service->ID) == 0)
				{
					return service;
				}
			}
		}
	}

	return NULL;
}

/**
 * Adds a specific service to the XML document
 *
 * @param service_to_add The service to add
 *
 * @return HPD_E_SERVICE_ALREADY_IN_XML if the Service is already in the XMl file, HPD_E_IMPOSSIBLE_TO_RETRIEVE_DEVICE_XML_NODE if the Service's Device is not in the XML file, HPD_E_SUCCESS if successful
 */
int 
add_service_to_xml(Service *service_to_add)
{
	if(service_is_in_xml_file (service_to_add) == HPD_YES)
		return HPD_E_SERVICE_ALREADY_IN_XML;
	else if(device_is_in_xml_file (service_to_add->device) == HPD_NO)
	{
		add_device_to_xml (service_to_add->device);
	}

	mxml_node_t *device = get_xml_node_of_device(service_to_add->device);
	if(device == NULL)
	{
		printf("Error while retrieving device node\n");
		return HPD_E_IMPOSSIBLE_TO_RETRIEVE_DEVICE_XML_NODE;
	}

	mxml_node_t *new_service;
	mxml_node_t *new_parameter;

	new_service = mxmlNewElement(device, "service");
	if(service_to_add->description != NULL) mxmlElementSetAttr(new_service, "desc", service_to_add->description);
	if(service_to_add->ID != NULL) mxmlElementSetAttr(new_service, "id", service_to_add->ID);
	if(service_to_add->value_url != NULL) mxmlElementSetAttr(new_service, "value_url", service_to_add->value_url);
	if(service_to_add->type != NULL) mxmlElementSetAttr(new_service, "type", service_to_add->type);
	if(service_to_add->unit != NULL) mxmlElementSetAttr(new_service, "unit", service_to_add->unit);

	Parameter *iterator = service_to_add->parameter_head;
	if(iterator != NULL)
	{
		new_parameter = mxmlNewElement(new_service, "parameter");
		if(iterator->ID != NULL) mxmlElementSetAttr(new_parameter, "id", iterator->ID);
		if(iterator->max != NULL) mxmlElementSetAttr(new_parameter, "max", iterator->max);
		if(iterator->min != NULL) mxmlElementSetAttr(new_parameter, "min", iterator->min);
		if(iterator->scale != NULL) mxmlElementSetAttr(new_parameter, "scale", iterator->scale);
		if(iterator->step != NULL) mxmlElementSetAttr(new_parameter, "step", iterator->step);
		if(iterator->type != NULL) mxmlElementSetAttr(new_parameter, "type", iterator->type);
		if(iterator->unit != NULL) mxmlElementSetAttr(new_parameter, "unit", iterator->unit);
		if(iterator->values != NULL) mxmlElementSetAttr(new_parameter, "values", iterator->values);

		while(iterator->next != NULL)
		{
			iterator = iterator->next;
			new_parameter = mxmlNewElement(new_service, "parameter");
			if(iterator->ID != NULL) mxmlElementSetAttr(new_parameter, "id", iterator->ID);
			if(iterator->max != NULL) mxmlElementSetAttr(new_parameter, "max", iterator->max);
			if(iterator->min != NULL) mxmlElementSetAttr(new_parameter, "min", iterator->min);
			if(iterator->scale != NULL) mxmlElementSetAttr(new_parameter, "scale", iterator->scale);
			if(iterator->step != NULL) mxmlElementSetAttr(new_parameter, "step", iterator->step);
			if(iterator->type != NULL) mxmlElementSetAttr(new_parameter, "type", iterator->type);
			if(iterator->unit != NULL) mxmlElementSetAttr(new_parameter, "unit", iterator->unit);
			if(iterator->values != NULL) mxmlElementSetAttr(new_parameter, "values", iterator->values);
		}
	}

	save_xml_tree ();

	return HPD_E_SUCCESS;
}

/**
 * Returns an internal char* of a value under the form of :
 * "<?xml version="1.0" encoding="UTF-8"?><value timestamp = xxxxxx >desired_value</value>"
 *
 * @param value The string of the value desired
 *
 * @return Returns the char* corresponding
 */
char * 
get_xml_value(char* value)
{
	mxml_node_t *xml;
	mxml_node_t *xml_value;

	xml = mxmlNewXML("1.0");
	xml_value = mxmlNewElement(xml, "value");
	mxmlElementSetAttr(xml_value, "timestamp", timestamp());
	mxmlNewText(xml_value, 0, value);

	char* return_value = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);
	mxmlDelete(xml);

	return return_value;
}

/**
 * Returns an internal char* of a value under the form of :
 * "<?xml version="1.0" encoding="UTF-8"?><subscription timestamp = xxxxxx url = yyyyy>yes/no</subscription>"
 *
 * @param value The value
 *
 * @param url The URL of the Service
 *
 * @return Returns the char* corresponding
 */
char * 
get_xml_subscription(char* value, char *url)
{
	mxml_node_t *xml;
	mxml_node_t *xml_value;

	xml = mxmlNewXML("1.0");
	xml_value = mxmlNewElement(xml, "subscription");
	mxmlElementSetAttr(xml_value, "timestamp", timestamp());
	mxmlElementSetAttr(xml_value, "url", url);
	mxmlNewText(xml_value, 0, value);

	char* return_value = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);

	mxmlDelete(xml);

	return return_value;
}

/**
 * Simple timestamp function
 *
 * @return Returns the timestamp
 */
char *
timestamp ( void )
{
	time_t ltime;
	ltime = time(NULL);
	const struct tm *timeptr = localtime(&ltime);

	static char wday_name[7][3] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static char mon_name[12][3] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	static char result[25];

	sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d",
	        wday_name[timeptr->tm_wday],
	        mon_name[timeptr->tm_mon],
	        timeptr->tm_mday, timeptr->tm_hour,
	        timeptr->tm_min, timeptr->tm_sec,
	        1900 + timeptr->tm_year);
	return result;
}

/**
 * Removes a given service from the XML file
 *
 * @param _service The service to remove
 *
 * @return HPD_E_SERVICE_NOT_IN_LIST if the Service is not in the XML file, HPD_E_IMPOSSIBLE_TO_RETRIEVE_SERVICE_XML_NODE if the retrieving of the XML node failed, HPD_YES if successful
 */
int 
remove_service_from_XML(Service *_service)
{
	if(service_is_in_xml_file (_service) == HPD_NO)
		return HPD_E_SERVICE_NOT_IN_LIST;

	mxml_node_t *service = get_xml_node_of_service (_service);
	if(service == NULL)
	{
		printf("Impossible to retrieve Service XML node\n");
		return HPD_E_IMPOSSIBLE_TO_RETRIEVE_SERVICE_XML_NODE;
	}
	mxmlRemove(service);
	mxmlDelete(service);
	save_xml_tree ();
	return HPD_YES;
}

/**
 * Removes a given device from the XML file
 *
 * @param _device The device to remove
 *
 * @return HPD_E_SERVICE_NOT_IN_LIST if the Device is not in the XML file, HPD_E_IMPOSSIBLE_TO_RETRIEVE_SERVICE_XML_NODE if the retrieving of the XML node failed, HPD_YES if successful
 */
int 
remove_device_from_XML(Device *_device)
{
	if(device_is_in_xml_file (_device) == HPD_NO)
		return HPD_E_SERVICE_NOT_IN_LIST;

	mxml_node_t *device = get_xml_node_of_device(_device);
	if(device == NULL)
	{
		printf("Error while retrieving device node\n");
		return HPD_E_IMPOSSIBLE_TO_RETRIEVE_DEVICE_XML_NODE;
	}
	mxmlRemove(device);
	mxmlDelete(device);
	save_xml_tree ();
	return HPD_YES;
}

/**
 * Deletes the XML file
 *
 * @param xml_file_path The path to the XML file to delete
 *
 * @return Returns HPD_E_SUCCESS if successful
 */
int 
delete_xml(char* xml_file_path)
{
	destroy_service_xml_file();
	remove(xml_file_path);
	return HPD_E_SUCCESS;
}

/**
 * Returns the string of the value extracted from the XML value
 *
 * @param _xml_value The XML formatted value
 *
 * @return The string of the value or NULL if failed
 */
char* 
get_value_from_xml_value(char* _xml_value)
{
	mxml_node_t *xml;
	mxml_node_t *node;

	xml = mxmlLoadString(NULL, _xml_value, MXML_TEXT_CALLBACK);
	if(xml == NULL)
	{
		printf("XML value format uncompatible with HomePort\n");
		return NULL;
	}

	node = mxmlFindElement(xml, xml, "value", NULL, NULL, MXML_DESCEND);
	if(node == NULL || node->child == NULL)
	{
		mxmlDelete(xml);
		printf("No \"value\" in the XML file\n");
		return NULL;
	}

	char *return_value = malloc(sizeof(char)*(strlen(node->child->value.text.string)+1));
	strcpy(return_value, node->child->value.text.string);

	mxmlDelete(xml);

	return return_value;
}

/**
 * Extracts the service XML description given its internal structure
 *
 * @param _service_to_extract The Service that we want to extract
 *
 * @return The XML description of the service or NULL if failed
 */ 
char *
extract_service_xml(Service *_service_to_extract)
{
	if(service_is_in_xml_file (_service_to_extract) == HPD_NO)
		return NULL;

	mxml_node_t *xml;
	xml = mxmlNewXML("1.0");

	mxml_node_t *new_service;
	mxml_node_t *new_device;
	mxml_node_t *new_parameter;

	new_service = mxmlNewElement(xml, "service");
	if(_service_to_extract->description != NULL) mxmlElementSetAttr(new_service, "desc", _service_to_extract->description);
	if(_service_to_extract->ID != NULL) mxmlElementSetAttr(new_service, "id", _service_to_extract->ID);
	if(_service_to_extract->value_url != NULL) mxmlElementSetAttr(new_service, "value_url", _service_to_extract->value_url);
	if(_service_to_extract->type != NULL) mxmlElementSetAttr(new_service, "type", _service_to_extract->type);
	if(_service_to_extract->unit != NULL) mxmlElementSetAttr(new_service, "unit", _service_to_extract->unit);

	new_device = mxmlNewElement(new_service, "device");
	if(_service_to_extract->device->description != NULL) mxmlElementSetAttr(new_device, "desc", _service_to_extract->device->description);
	if(_service_to_extract->device->ID != NULL) mxmlElementSetAttr(new_device, "id", _service_to_extract->device->ID);
	if(_service_to_extract->device->vendorID != NULL) mxmlElementSetAttr(new_device, "vendorID", _service_to_extract->device->vendorID);
	if(_service_to_extract->device->productID != NULL) mxmlElementSetAttr(new_device, "productID", _service_to_extract->device->productID);
	if(_service_to_extract->device->version != NULL) mxmlElementSetAttr(new_device, "version", _service_to_extract->device->version);
	if(_service_to_extract->device->IP != NULL) mxmlElementSetAttr(new_device, "ip", _service_to_extract->device->IP);
	if(_service_to_extract->device->port != NULL) mxmlElementSetAttr(new_device, "port", _service_to_extract->device->port);
	if(_service_to_extract->device->location != NULL) mxmlElementSetAttr(new_device, "location", _service_to_extract->device->location);
	if(_service_to_extract->device->type != NULL) mxmlElementSetAttr(new_device, "type", _service_to_extract->device->type);

	Parameter *iterator = _service_to_extract->parameter_head;
	if(iterator != NULL)
	{
		new_parameter = mxmlNewElement(new_service, "parameter");
		if(iterator->ID != NULL) mxmlElementSetAttr(new_parameter, "id", iterator->ID);
		if(iterator->max != NULL) mxmlElementSetAttr(new_parameter, "max", iterator->max);
		if(iterator->min != NULL) mxmlElementSetAttr(new_parameter, "min", iterator->min);
		if(iterator->scale != NULL) mxmlElementSetAttr(new_parameter, "scale", iterator->scale);
		if(iterator->step != NULL) mxmlElementSetAttr(new_parameter, "step", iterator->step);
		if(iterator->type != NULL) mxmlElementSetAttr(new_parameter, "type", iterator->type);
		if(iterator->unit != NULL) mxmlElementSetAttr(new_parameter, "unit", iterator->unit);
		if(iterator->values != NULL) mxmlElementSetAttr(new_parameter, "values", iterator->values);

		while(iterator->next != NULL)
		{
			iterator = iterator->next;
			new_parameter = mxmlNewElement(new_service, "parameter");
			if(iterator->ID != NULL) mxmlElementSetAttr(new_parameter, "id", iterator->ID);
			if(iterator->max != NULL) mxmlElementSetAttr(new_parameter, "max", iterator->max);
			if(iterator->min != NULL) mxmlElementSetAttr(new_parameter, "min", iterator->min);
			if(iterator->scale != NULL) mxmlElementSetAttr(new_parameter, "scale", iterator->scale);
			if(iterator->step != NULL) mxmlElementSetAttr(new_parameter, "step", iterator->step);
			if(iterator->type != NULL) mxmlElementSetAttr(new_parameter, "type", iterator->type);
			if(iterator->unit != NULL) mxmlElementSetAttr(new_parameter, "unit", iterator->unit);
			if(iterator->values != NULL) mxmlElementSetAttr(new_parameter, "values", iterator->values);
		}
	}

	char* return_string = mxmlSaveAllocString(xml, MXML_NO_CALLBACK);

	mxmlDelete(xml);

	return return_string;
}

/**
 * Returns the entire xml device list
 *
 * @return The XML device list
 */ 
char *
get_xml_device_list()
{

	char *return_value = mxmlSaveAllocString(service_xml_file->xml_tree, MXML_NO_CALLBACK);

	return return_value;

}

