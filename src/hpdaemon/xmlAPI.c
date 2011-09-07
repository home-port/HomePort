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

#include "xmlAPI.h" 

/**
 * Initialization of the services.xml file that will be used
 * throughout the whole process
 *
 * @param name the name of the devicelist
 * @param id the id of the devicelist
 *
 * @return returns 0 if successful -1 if failed
 */
int init_xml_file(char *name, char *id)
{

	int rc;
	xmlTextWriterPtr writer;
	xmlDocPtr doc;

	/*Creation of the Writer*/
	writer = xmlNewTextWriterDoc(&doc, 0);
	if(writer == NULL){
		printf("Error while creating XML writer.\n");
		return -1;
	}

	/*Creation of the document*/
	rc = xmlTextWriterStartDocument(writer, NULL, ENCODING, NULL);
	if(rc < 0){
		printf("Error while creating the document.\n");
		return -1;
	}

	/*Creation of the first(root) element => devicelist*/
	rc = xmlTextWriterStartElement(writer, BAD_CAST "devicelist");
	if(rc < 0){
		printf("Error while creating the root element.\n");
		return -1;
	}

	/*Add attribute to devicelist*/
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "xmlns:xsi",
	                                 BAD_CAST "http://www.w3.org/2001/XMLSchema-instance");
	                                 if(rc < 0){
										 printf("Error while adding attribute to root element.\n");
										 return -1;
									 }

	                                 /*Add attribute to devicelist*/
	                                 rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "xsi:noNamespaceSchemaLocation",
	                                                                  BAD_CAST "http://cs.au.dk/dithus/xml/devicedescription.xsd");
	                                                                  if(rc < 0){
																		  printf("Error while adding attribute to root element.\n");
																		  return -1;
																	  }

	                                                                  /*Add attribute to devicelist*/
	                                                                  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "name",
	                                                                                                   BAD_CAST name);
	                                                                  if(rc < 0){
																		  printf("Error while adding attribute to root element.\n");
																		  return -1;
																	  }

	                                                                  /*Add attribute to devicelist*/
	                                                                  rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "id",
	                                                                                                   BAD_CAST id);
	                                                                  if(rc < 0){
																		  printf("Error while adding attribute to root element.\n");
																		  return -1;
																	  }

	                                                                  /*Finish the writing*/
	                                                                  rc = xmlTextWriterEndDocument(writer);
	                                                                  if (rc < 0){
																		  printf("Error at xmlTextWriterEndDocument.\n");
																		  return -1;
																	  }

	                                                                  /*Saving the document and freeing resources*/
	                                                                  xmlFreeTextWriter(writer);
	                                                                  xmlSaveFormatFileEnc(XML_FILE_NAME, doc, ENCODING,1);
	                                                                  xmlFreeDoc(doc);

	                                                                  return 0;
                                                                  }

/**
 * Checks if the Device is already in the XML file
 *
 * @param device the device that we want to check
 *
 * @return returns 0 if the device is not in the XML file and 1 if it is
 */
int device_is_in_xml_file(Device *device)
{

	xmlDocPtr doc;
	xmlChar *xpath = (xmlChar*) "//device";
		xmlNodeSetPtr nodeset;
	xmlNodePtr cur;
	xmlXPathObjectPtr result;
	xmlChar *id, *type;
	int i;

	doc = xmlParseFile(XML_FILE_NAME);
	if(doc == NULL){
		printf("Document not parsed successfully. \n");
		return -1;
	}

	result = get_node_set (doc,xpath);
	if(result){
		nodeset = result->nodesetval;
		for(i = 0 ; i < nodeset->nodeNr ; i++){
			cur = nodeset->nodeTab[i];
			type = xmlGetProp (cur,(const xmlChar *) "type");
			id = xmlGetProp (cur,(const xmlChar *) "id");
			if(strcmp((char*)type,device->type) == 0 && strcmp((char*)id,device->ID) == 0) {
				xmlFree(id);
				xmlFree(type);
				xmlXPathFreeObject (result);
				xmlFreeDoc(doc);
				return 1; 
			}
			xmlFree(type);
			xmlFree(id);
		}
		xmlXPathFreeObject (result);
	}
	xmlFreeDoc(doc);
	return 0;
}

/**
 * Checks if the Service is already in the XML file
 *
 * @param service the service that we want to check
 *
 * @return returns 0 if the device is not in the XML file and 1 if it is
 */
int service_is_in_xml_file(Service *service)
{

	xmlDocPtr doc;
	xmlChar *xpath = (xmlChar*) "//service";
		xmlNodeSetPtr nodeset;
	xmlNodePtr cur;
	xmlXPathObjectPtr result;
	xmlChar *id, *type;

	int i;

	doc = xmlParseFile(XML_FILE_NAME);
	if(doc == NULL){
		printf("Document not parsed successfully. \n");
		return -1;
	}

	result = get_node_set (doc,xpath);
	if(result){
		nodeset = result->nodesetval;
		for(i = 0 ; i < nodeset->nodeNr ; i++){
			cur = nodeset->nodeTab[i];
			id = xmlGetProp (cur, (const xmlChar *)"id");
			type = xmlGetProp (cur,(const xmlChar *)"type");
			if(strcmp((char*)id,service->ID) == 0 && strcmp((char*)type,service->type)==0 && device_is_in_xml_file (service->device) == 1){
				xmlFree(id);
				xmlFree(type);
				xmlXPathFreeObject (result);
				xmlFreeDoc(doc);
				return 1;
			}
			xmlFree(type);
			xmlFree(id);
		}
		xmlXPathFreeObject (result);
	}
	xmlFreeDoc(doc);
	return 0; 
}

/**
 * Returns the node set of a specified type (ie : Service, Device etc...)
 *
 * @param doc the XML Document
 * @param xpath the desired type
 *
 * @return returns the node set
 */
xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath)
{

	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;

	context = xmlXPathNewContext(doc);
	if(context == NULL){
		printf("Error in xmlPathNewContext. \n");
		return NULL;
	}

	result = xmlXPathEvalExpression (xpath, context);
	xmlXPathFreeContext (context);
	if(result == NULL){
		printf("Error in xmlXPathEvalExpression. \n");
		return NULL;
	}

	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
		xmlXPathFreeObject (result);
		return NULL;
	}

	return result;
}

/**
 * Adds a specific device to the XML document
 *
 * @param device_to_add The device to add
 *
 * @return returns 0 if successful and -1 if failed
 */
int add_device_to_xml(Device *device_to_add)
{


	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr newNode;
	xmlAttrPtr newAttr;

	if( device_is_in_xml_file (device_to_add) == 1 ) return -2;

	doc = xmlParseFile(XML_FILE_NAME);
	if(doc == NULL){
		printf("Error while parsing the file. \n");
		return -1;
	}

	cur = xmlDocGetRootElement (doc);
	if(cur == NULL){
		printf("The Document is empty. \n");
		xmlFreeDoc(doc);
		return -1;
	}

	if(xmlStrcmp(cur->name, (const xmlChar *) "devicelist")){
		printf("Document of the wrong type. \n");
		xmlFreeDoc(doc);
		//xmlFreeNode(cur);
		return -1;
	}
	/*Adds the new device with all its attributes*/
	newNode = xmlNewTextChild (cur, NULL,(const xmlChar *) "device", NULL);	
	if(device_to_add->description != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"desc", (const xmlChar *)device_to_add->description);
	if(device_to_add->ID != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"id", (const xmlChar *)device_to_add->ID);
	if(device_to_add->vendorID != NULL) newAttr = xmlNewProp (newNode,(const xmlChar *) "vendorid", (const xmlChar *)device_to_add->vendorID);
	if(device_to_add->productID != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"productid", (const xmlChar *)device_to_add->productID);
	if(device_to_add->version != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"version", (const xmlChar *)device_to_add->version);
	if(device_to_add->IP != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"ip", (const xmlChar *)device_to_add->IP);
	if(device_to_add->port != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"port", (const xmlChar *)device_to_add->port);
	if(device_to_add->location != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"location", (const xmlChar *)device_to_add->location);
	if(device_to_add->type != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"type", (const xmlChar *)device_to_add->type);

	/*Save the file*/
	xmlSaveFormatFileEnc(XML_FILE_NAME, doc, ENCODING,1);
	xmlFreeDoc(doc);

	return 0;
}

/**
 * Adds a specific service to the XML document
 *
 * @param service_to_add The service to add
 *
 * @return returns 0 if successful and -1 if failed
 */
int add_service_to_xml(Service *service_to_add)
{

	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr newNode;
	xmlNodePtr paramNode;
	xmlAttrPtr newAttr;
	xmlAttrPtr newParamAttr;
	xmlChar *xpath = (xmlChar*) "//device";
		xmlXPathObjectPtr result;
	xmlChar *id, *type;
	xmlNodeSetPtr nodeset;
	int i;

	/*Check if the device is already in the xml*/
	if( device_is_in_xml_file (service_to_add->device) == 0 ) add_device_to_xml(service_to_add->device);

	/*Check if the service is already in the xml*/
	if( service_is_in_xml_file (service_to_add) == 1 ) return -2;

	doc = xmlParseFile(XML_FILE_NAME);
	if(doc == NULL){
		printf("Error while parsing the file. \n");
		return -1;
	}

	cur = xmlDocGetRootElement (doc);
	if(cur == NULL){
		printf("The Document is empty. \n");
		xmlFreeDoc(doc);
		return -1;
	}

	if(xmlStrcmp(cur->name, (const xmlChar *) "devicelist")){
		printf("Document of the wrong type. \n");
		xmlFreeDoc(doc);
		return -1;
	}

	result = get_node_set (doc,xpath);
	if(result){
		nodeset = result->nodesetval;
		for(i = 0 ; i < nodeset->nodeNr ; i++){
			cur = nodeset->nodeTab[i];
			id = xmlGetProp (cur, (const xmlChar *)"id");
			type = xmlGetProp (cur,(const xmlChar *)"type");
			if(strcmp((char*)id,service_to_add->device->ID) == 0 && strcmp((char*)type,service_to_add->device->type) == 0){

				/*Adds the new device with all its attributes*/
				newNode = xmlNewTextChild (cur, NULL, (const xmlChar *)"service", NULL);
				if(service_to_add->description != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"desc", (const xmlChar *)service_to_add->description);
				if(service_to_add->ID != NULL) newAttr = xmlNewProp (newNode,(const xmlChar *) "id",(const xmlChar *) service_to_add->ID);
				if(service_to_add->value_url != NULL) newAttr = xmlNewProp (newNode,(const xmlChar *) "value_url", (const xmlChar *)service_to_add->value_url);
				if(service_to_add->type != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"type", (const xmlChar *)service_to_add->type);
				if(service_to_add->unit != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"unit", (const xmlChar *)service_to_add->unit);

				ParameterElement *iterator = service_to_add->parameter_head;

				if(iterator != NULL)
				{    
					paramNode = xmlNewTextChild (newNode, NULL, (const xmlChar *)"parameter", NULL);
					if(iterator->parameter->ID != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"id", (const xmlChar *)iterator->parameter->ID);
					if(iterator->parameter->max != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"max",(const xmlChar *) iterator->parameter->max);
					if(iterator->parameter->min != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"min",(const xmlChar *) iterator->parameter->min);
					if(iterator->parameter->scale != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"scale", (const xmlChar *)iterator->parameter->scale);
					if(iterator->parameter->step != NULL) newParamAttr = xmlNewProp (paramNode,(const xmlChar *) "step",(const xmlChar *) iterator->parameter->step);
					if(iterator->parameter->type != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"type",(const xmlChar *) iterator->parameter->type);
					if(iterator->parameter->unit != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"unit", (const xmlChar *)iterator->parameter->unit);
					if(iterator->parameter->values != NULL) newParamAttr = xmlNewProp (paramNode,(const xmlChar *) "values",(const xmlChar *) iterator->parameter->values);

					while(iterator->next != NULL)
					{
						iterator = iterator->next;
						paramNode = xmlNewTextChild (newNode, NULL, (const xmlChar *)"parameter", NULL);
						if(iterator->parameter->ID != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"id",(const xmlChar *) iterator->parameter->ID);
						if(iterator->parameter->max != NULL) newParamAttr = xmlNewProp (paramNode,(const xmlChar *) "max",(const xmlChar *) iterator->parameter->max);
						if(iterator->parameter->min != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"min",(const xmlChar *) iterator->parameter->min);
						if(iterator->parameter->scale != NULL) newParamAttr = xmlNewProp (paramNode,(const xmlChar *) "scale",(const xmlChar *) iterator->parameter->scale);
						if(iterator->parameter->step != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"step",(const xmlChar *) iterator->parameter->step);
						if(iterator->parameter->type != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"type", (const xmlChar *)iterator->parameter->type);
						if(iterator->parameter->unit != NULL) newParamAttr = xmlNewProp (paramNode, (const xmlChar *)"unit",(const xmlChar *) iterator->parameter->unit);
						if(iterator->parameter->values != NULL) newParamAttr = xmlNewProp (paramNode,(const xmlChar *) "values",(const xmlChar *) iterator->parameter->values);

					}
				}


				/*Save the file*/
				xmlSaveFormatFileEnc(XML_FILE_NAME, doc, ENCODING,1);
				xmlFreeDoc(doc);
				xmlXPathFreeObject (result);
				xmlFree(id);
				xmlFree(type);

				return 0;

			}
			xmlFree(id);
			xmlFree(type);
		}
		xmlXPathFreeObject (result);
	}
	xmlFreeDoc(doc);
	return -1;	
}

/**
 * Returns an internal xmlChar of a value under the form of :
 * <?xml version="1.0" encoding="UTF-8"?>
 * <value timestamp = xxxxxx >
 *       desired_value
 * </value>
 *
 * @param value The string of the value desired
 *
 * @return Returns the xmlChar* corresponding
 */
xmlChar * get_xml_value(char* value)
{

	xmlDocPtr doc;
	xmlChar* xmlbuff;
	int buffersize;

	int rc;
	xmlTextWriterPtr writer;

	writer = xmlNewTextWriterDoc(&doc, 0);
	if(writer == NULL){
		printf("Error while creating XML writer.\n");
		return NULL;
	}

	rc = xmlTextWriterStartDocument(writer, NULL, ENCODING, NULL);
	if(rc < 0){
		printf("Error while creating the document.\n");
		return NULL;
	}

	rc = xmlTextWriterStartElement (writer, BAD_CAST "value");
	if(rc < 0){
		printf("Error while creating the root element.\n");
		return NULL;
	}

	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "timestamp",
	                                 BAD_CAST timestamp ());
	if(rc < 0){
		printf("Error while adding attribute to root element.\n");
		return NULL;
	}

	rc = xmlTextWriterWriteString (writer,BAD_CAST value);

	rc = xmlTextWriterEndDocument(writer);
	if (rc < 0){
		printf("Error at xmlTextWriterEndDocument.\n");
		return NULL;
	}

	xmlFreeTextWriter(writer);

	xmlDocDumpFormatMemory (doc, &xmlbuff, &buffersize, 1);

	xmlFreeDoc (doc);

	return xmlbuff;
}

/**
 * Simple timestamp function
 *
 * @return Returns the timestamp
 */
char * timestamp()
{
	time_t ltime;
	ltime = time(NULL);
	return asctime(localtime(&ltime));
}

/**
 * Removes a given service from the XML file
 *
 * @param _service The service to remove
 *
 * @return Returns 0 if successful and -1 if failed
 */
int remove_service_from_XML(Service *_service)
{

	xmlDocPtr doc;
	xmlChar *xpath = (xmlChar*) "//device";
		xmlNodeSetPtr nodeset;
	xmlNodePtr cur;
	xmlXPathObjectPtr result;
	xmlChar *id, *type;
	int i;

	if(service_is_in_xml_file (_service) == 0) return -1;

	doc = xmlParseFile(XML_FILE_NAME);
	if(doc == NULL){
		printf("Document not parsed successfully. \n");
		return -1;
	}

	result = get_node_set (doc,xpath);
	if(result){
		nodeset = result->nodesetval;
		for(i = 0 ; i < nodeset->nodeNr ; i++){
			cur = nodeset->nodeTab[i];
			id = xmlGetProp (cur, (const xmlChar *)"id");
			type = xmlGetProp (cur,(const xmlChar *) "type");
			if(strcmp((char*)id,_service->device->ID)== 0 && strcmp((char*)type, _service->device->type) == 0){
				cur = cur->xmlChildrenNode;
				while (cur != NULL) {
					id = xmlGetProp (cur,(const xmlChar *) "id");
					type = xmlGetProp (cur, (const xmlChar *)"type");
					if (strcmp((char*)id,_service->ID) == 0 && strcmp((char*)type,_service->type) == 0){
						/*Removing the Node*/
						xmlUnlinkNode(cur);
						xmlFreeNode(cur);
						xmlFree(type);
						xmlFree(id);
						xmlXPathFreeObject (result);

						/*Save the file*/
						xmlSaveFormatFileEnc(XML_FILE_NAME, doc, ENCODING,1);
						xmlFreeDoc(doc);
						return 0;
					}
					cur = cur->next;
				}
			}
			xmlFree(type);
			xmlFree(id);
		}
		xmlXPathFreeObject (result);
	}
	xmlFreeDoc(doc);

	return -1;
}

/**
 * Removes a given device from the XML file
 *
 * @param _device The device to remove
 *
 * @return Returns 0 if successful and -1 if failed
 */
int remove_device_from_XML(Device *_device)
{

	xmlDocPtr doc;
	xmlChar *xpath = (xmlChar*) "//device";
		xmlNodeSetPtr nodeset;
	xmlNodePtr cur;
	xmlXPathObjectPtr result;
	xmlChar *id, *type;
	int i;

	if(device_is_in_xml_file (_device) == 0) return -1;

	doc = xmlParseFile(XML_FILE_NAME);
	if(doc == NULL){
		printf("Document not parsed successfully. \n");
		return -1;
	}

	result = get_node_set (doc,xpath);
	if(result){
		nodeset = result->nodesetval;
		for(i = 0 ; i < nodeset->nodeNr ; i++){
			cur = nodeset->nodeTab[i];
			id = xmlGetProp (cur,(const xmlChar *) "id");
			type = xmlGetProp (cur,(const xmlChar *) "type");
			if(strcmp((char*)id,_device->ID)== 0 && strcmp((char*)type, _device->type) == 0){

				/*Removing the Node*/
				xmlUnlinkNode(cur);
				xmlFreeNode(cur);
				xmlFree(type);
				xmlFree(id);
				xmlXPathFreeObject (result);

				/*Save the file*/
				xmlSaveFormatFileEnc(XML_FILE_NAME, doc, ENCODING,1);
				xmlFreeDoc(doc);
				return 0;

			}
			xmlFree(type);
			xmlFree(id);
		}
		xmlXPathFreeObject (result);
	}
	xmlFreeDoc(doc);

	return -1;
}
/**
 * Deletes the XML file
 *
 * @param xml_file_path The path to the XML file to delete
 *
 * @return Returns 0 if successful
 */
int delete_xml(char* xml_file_path)
{
	remove(xml_file_path);
	return 0;
}

/**
 * Returns the string of the value extracted from the XML value
 *
 * @param _xml_value The XML formatted value
 *
 * @return The string of the value
 */
char* get_value_from_xml_value(char* _xml_value)
{

	FILE* temporary_xml_file;
	temporary_xml_file = fopen("temp.xml","w+t");
	fputs(_xml_value,temporary_xml_file);
	fclose(temporary_xml_file);

	xmlTextReaderPtr reader;
	int ret;
	char* return_value;

	reader = xmlReaderForFile("temp.xml", NULL, 0);
	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);                             
		while(ret == 1)
		{
			if(strcmp((char*)xmlTextReaderName(reader),"value")==0)
			{
				ret = xmlTextReaderRead(reader);
				if(strcmp((char*)xmlTextReaderName(reader),"#text")==0){
					return_value = malloc(sizeof(char)*strlen((char*)xmlTextReaderValue (reader)));
					strcpy(return_value,(char*)xmlTextReaderValue (reader));
					xmlFreeTextReader(reader);
					delete_xml("temp.xml");
					return return_value;
				}
				else{
					delete_xml("temp.xml");
					xmlFreeTextReader(reader);
					return NULL;
				}
			}
			ret = xmlTextReaderRead(reader);
		}
	}
	delete_xml("temp.xml");
	xmlFreeTextReader(reader);

	return NULL;
}

/**
 * Extracts the service XML description given its internal structure
 *
 * @param _service_to_extract The Service that we want to extract
 *
 * @return The XML description of the service
 */ 
xmlChar *extract_service_xml(Service *_service_to_extract)
{

	xmlDocPtr doc;
	xmlChar *xpath = (xmlChar*) "//device";
		xmlNodeSetPtr nodeset;
	xmlNodePtr cur;
	xmlNodePtr newNode;
	xmlXPathObjectPtr result;
	xmlAttrPtr newAttr;
	xmlChar *id, *type, *return_string ,*xml_result;
	xmlBufferPtr buffer = xmlBufferCreate();

	int i;

	doc = xmlParseFile(XML_FILE_NAME);
	if(doc == NULL){
		printf("Document not parsed successfully. \n");
		return NULL;
	}

	result = get_node_set (doc,xpath);
	if(result){
		nodeset = result->nodesetval;
		for(i = 0 ; i < nodeset->nodeNr ; i++){
			cur = nodeset->nodeTab[i];
			id = xmlGetProp (cur, (const xmlChar *)"id");
			type = xmlGetProp (cur, (const xmlChar *)"type");
			if(strcmp((char*)id,_service_to_extract->device->ID)== 0 && strcmp((char*)type, _service_to_extract->device->type) == 0){
				cur = cur->xmlChildrenNode;
				while (cur != NULL) {
					id = xmlGetProp (cur, (const xmlChar *)"id");
					type = xmlGetProp (cur, (const xmlChar *)"type");
					if (strcmp((char*)id,_service_to_extract->ID) == 0 && strcmp((char*)type,_service_to_extract->type) == 0){
						newNode = xmlNewTextChild (cur, NULL,(const xmlChar *) "device", NULL);
						if(_service_to_extract->device->description != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"desc", (const xmlChar *)_service_to_extract->device->description);
						if(_service_to_extract->device->type != NULL) newAttr = xmlNewProp (newNode,(const xmlChar *) "type",(const xmlChar *) _service_to_extract->device->type);
						if(_service_to_extract->device->port != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"port", (const xmlChar *)_service_to_extract->device->port);
						if(_service_to_extract->device->location != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"location", (const xmlChar *)_service_to_extract->device->location);
						if(_service_to_extract->device->vendorID != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"vendorid", (const xmlChar *)_service_to_extract->device->vendorID);
						if(_service_to_extract->device->productID != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"productid", (const xmlChar *)_service_to_extract->device->productID);
						if(_service_to_extract->device->version != NULL) newAttr = xmlNewProp (newNode,(const xmlChar *) "version", (const xmlChar *)_service_to_extract->device->version);
						if(_service_to_extract->device->IP != NULL) newAttr = xmlNewProp (newNode, (const xmlChar *)"ip", (const xmlChar *)_service_to_extract->device->IP);
						if(_service_to_extract->device->ID != NULL) newAttr = xmlNewProp (newNode,(const xmlChar *) "id",(const xmlChar *) _service_to_extract->device->ID);

						xmlNodeDump(buffer,doc,cur,0,1);

						xml_result = (xmlChar*)xmlBufferContent(buffer);
						return_string = (xmlChar *)malloc((strlen((char *)xml_result) + 1)*sizeof(xmlChar));
						strcpy((char*)return_string,(char*)xml_result);
						xmlBufferFree(buffer);
						xmlFree(id);
						xmlFree(type);
						xmlXPathFreeObject (result);
						xmlFreeDoc(doc);

						return return_string;
					}
					cur = cur->next;
				}
			}
			xmlFree(id);
			xmlFree(type);
		}
		xmlXPathFreeObject (result);
	}
	xmlBufferFree(buffer);
	xmlFreeDoc(doc);

	return NULL;
}






/*******************IN PROGRESS******************/

void generate_get_and_put_functions_from_xml(char* _xml_file_name){

	//Create or open C file
	FILE* get_and_put_functions_c_source;
	get_and_put_functions_c_source = fopen("get_and_put_functions.c","w+t");

	//Create or open H file
	FILE* get_and_put_functions_h_source;
	get_and_put_functions_h_source = fopen("get_and_put_functions.h","w+t");


	fputs("/*Auto Generated File from HomePortDaemon*\\n",get_and_put_functions_c_source);
	fputs("/*Auto Generated File from HomePortDaemon*\\n",get_and_put_functions_h_source);

	fputs("#include \"get_and_put_functions.h\"\n\n",get_and_put_functions_c_source);

	fputs("#ifndef _GET_AND_PUT_FUNCTIONS_H\n",get_and_put_functions_h_source);
	fputs("#define _GET_AND_PUT_FUNCTIONS_H\n\n",get_and_put_functions_h_source);

	fputs("#include <stdio.h>\n",get_and_put_functions_h_source);
	fputs("#include <stdlib.h>\n",get_and_put_functions_h_source);
	fputs("#include <string.h>\n",get_and_put_functions_h_source);
	fputs("#include \"service_list.h\"\n\n",get_and_put_functions_h_source);


	//Scanning for Services

	xmlDocPtr doc;
	xmlChar *xpath = (xmlChar*) "//service";
		xmlNodeSetPtr nodeset;
	xmlXPathObjectPtr result;
	int i;
	xmlChar *id;
	xmlNodePtr cur;
	char* put_begin = "char * put_";
	char* get_begin = "char * get_";
	char* put_end_c = "(char * _put_value){\n\nreturn NULL;\n";
	char* put_end_h = "(char * _put_value);\n";
	char* get_end_c = "(){\n\nreturn NULL;\n";
	char* get_end_h = "();\n";
	char* return_service_list_name_h = "ServiceList return_service_list(ServiceList list);\n";
	char* return_service_list_name_c = "ServiceList return_service_list(ServiceList list){\n";

	doc = xmlParseFile(_xml_file_name);
	if(doc == NULL){
		printf("Document not parsed successfully. \n");
		return;
	}

	result = get_node_set (doc,xpath);
	if(result){
		nodeset = result->nodesetval;
		for(i = 0 ; i < nodeset->nodeNr ; i++){
			cur = nodeset->nodeTab[i];
			id = xmlGetProp (cur, (const xmlChar *)"id");
			if(id){

				/*****Create the PUT function*****/

				//Add the PUT function to the C file
				fputs(put_begin,get_and_put_functions_c_source);
				fputs((char*)id,get_and_put_functions_c_source);
				fputs(put_end_c,get_and_put_functions_c_source);
				fputs("}\n",get_and_put_functions_c_source);

				//Add the PUT function to the H file
				fputs(put_begin,get_and_put_functions_h_source);
				fputs((char*)id,get_and_put_functions_h_source);
				fputs(put_end_h,get_and_put_functions_h_source);

				/*****Create the GET function*****/

				//Add the GET function to the C file
				fputs(get_begin,get_and_put_functions_c_source);
				fputs((char*)id,get_and_put_functions_c_source);
				fputs(get_end_c,get_and_put_functions_c_source);
				fputs("}\n",get_and_put_functions_c_source);

				//Add the GET function to the H file
				fputs(get_begin,get_and_put_functions_h_source);
				fputs((char*)id,get_and_put_functions_h_source);
				fputs(get_end_h,get_and_put_functions_h_source);


				fputs("\n",get_and_put_functions_c_source);
				fputs("\n",get_and_put_functions_h_source);
			}
		}
	}
	xmlFree(id);
	xmlXPathFreeObject (result);
	xmlFreeDoc(doc);

	/*Creating the return_service_list function*/

	fputs(return_service_list_name_c, get_and_put_functions_c_source);
	fputs(return_service_list_name_h, get_and_put_functions_h_source);

	/*****Stream the file*****/

	xmlTextReaderPtr reader;
	int ret;
	const xmlChar *name;
	char* inDevice = malloc(100);
	char* inService = malloc(100);
	int firstParameter = 0;
	char* inParameter = malloc(100);

	inDevice = "";
	inService = "";
	inParameter = "";

	reader = xmlReaderForFile(_xml_file_name, NULL, 0);
	if (reader != NULL) {
		ret = xmlTextReaderRead(reader);
		while (ret == 1) {
			name = xmlTextReaderConstName(reader);
			if (strcmp((char*)name,"device")==0 && strcmp(inDevice,"") == 0){
				inDevice = (char *)xmlTextReaderGetAttribute(reader,(const xmlChar *)"id");
				/*Create the device :
				 Device* create_device_struct(
				                              char *_description,
				                              char *_ID,
				                              char *_UID,
				                              char *_IP,
				                              char *_port,
				                              char *_location,
				                              char *_type);
											  */
				fputs(" Device *device_", get_and_put_functions_c_source);
				fputs(inDevice, get_and_put_functions_c_source);
				fputs("= create_device_struct (", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"desc")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"desc"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"id")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"id"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"uid")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"uid"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"ip")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"ip"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"port")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"port"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"location")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"location"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"type")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"type"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs(");\n\n", get_and_put_functions_c_source);

			}	
			else if (strcmp((char*)name,"device")==0 && strcmp(inDevice,"") != 0){
				inDevice = "";
			}
			else if (strcmp((char*)name,"service")==0 && strcmp(inService,"") == 0){
				inService = (char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"id");

				/*Create the service :
				 Service* create_service_struct(
				                                char *_description,
				                                char *_ID,
				                                char *_value_url,
				                                char *_type,
				                                char *_unit,
				                                Device *_device,
				                                char* (*_get_function)(),
												char* (*put_function)(char*),
												Parameter *_parameter);
												*/
				fputs(" Service *service_", get_and_put_functions_c_source);
				fputs(inService, get_and_put_functions_c_source);
				fputs("= create_service_struct (", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"desc")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"desc"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"id")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"id"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"value_url")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"value_url"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"type")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"type"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"unit")){
					fputs("\"", get_and_put_functions_c_source);
					fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"unit"), get_and_put_functions_c_source);
					fputs("\"", get_and_put_functions_c_source);
				}
				else fputs("NULL", get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				fputs("device_", get_and_put_functions_c_source);
				fputs(inDevice, get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				fputs("get_", get_and_put_functions_c_source);
				fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"id"), get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

				fputs("put_", get_and_put_functions_c_source);
				fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"id"), get_and_put_functions_c_source);
				fputs("\n\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

			}
			else if (strcmp((char*)name,"service")==0 && strcmp(inService,"") != 0){

				fputs("list = add_service_to_list(service_", get_and_put_functions_c_source);
				fputs(inService , get_and_put_functions_c_source);
				fputs(", list);\n\n", get_and_put_functions_c_source);
				inService = "";
				firstParameter = 0;

			}
			else if (strcmp((char*)name,"parameter")==0){
				inParameter = (char *)xmlTextReaderGetAttribute(reader,(const xmlChar *)"id");
				if(firstParameter == 0){
					firstParameter = 1;
					/*Create the parameter :
					 Parameter* create_parameter_struct(
					                                    char *_ID,
					                                    char *_max,
					                                    char *_min,
					                                    char *_scale,
					                                    char *_step,
					                                    char *_type,
					                                    char *_unit,
					                                    char *_values);
														*/
					fputs( "create_parameter_struct(", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"id")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"id"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"max")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"max"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"min")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"min"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"scale")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"scale"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"step")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"step"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"type")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"type"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"unit")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"unit"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"values")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"values"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("));\n\n", get_and_put_functions_c_source);

				}
				else{
					/*Create the parameter :
					 Parameter* create_parameter_struct(
					                                    char *_ID,
					                                    char *_max,
					                                    char *_min,
					                                    char *_scale,
					                                    char *_step,
					                                    char *_type,
					                                    char *_unit,
					                                    char *_values);
														*/
					fputs(" Parameter *parameter_", get_and_put_functions_c_source);
					fputs(inParameter, get_and_put_functions_c_source);
					fputs("= create_parameter_struct (", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"id")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"id"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"max")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"max"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"min")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"min"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"scale")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"scale"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"step")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"step"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"type")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"type"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"unit")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"unit"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs("\n\t\t\t\t\t\t\t\t\t\t, ", get_and_put_functions_c_source);

					if(xmlTextReaderGetAttribute(reader,(const xmlChar *)"values")){
						fputs("\"", get_and_put_functions_c_source);
						fputs((char*)xmlTextReaderGetAttribute(reader,(const xmlChar *)"values"), get_and_put_functions_c_source);
						fputs("\"", get_and_put_functions_c_source);
					}
					else fputs("NULL", get_and_put_functions_c_source);
					fputs(");\n", get_and_put_functions_c_source);

					/*Add the parameter to the list of parameter of service */

					fputs("add_parameter_to_service (parameter_", get_and_put_functions_c_source);
					fputs(inParameter, get_and_put_functions_c_source);
					fputs(",service_", get_and_put_functions_c_source);
					fputs(inService, get_and_put_functions_c_source);
					fputs(");\n\n", get_and_put_functions_c_source);
				}
			}

			ret = xmlTextReaderRead(reader);
		}
		xmlFreeTextReader(reader);
		if (ret != 0) {
			fprintf(stderr, "%s : failed to parse\n", _xml_file_name);
		}
	} else {
		fprintf(stderr, "Unable to open %s\n", _xml_file_name);
	}



	fputs("return list;\n",get_and_put_functions_c_source);
	fputs("}\n",get_and_put_functions_c_source);

	fclose(get_and_put_functions_c_source);

	fputs("#endif /* GET_AND_PUT_FUNCTIONS_FCT_H *\n",get_and_put_functions_h_source);
	fclose(get_and_put_functions_h_source);



}
/*
Service *get_service_from_xml_node(char *xml_node){

	EN ATTENTE DE LA NOUVELLE STRUCTURE SERVICE
		static int service1_value = 0;
	static int service2_value = 0;
	static int service3_value = 0;

	Device *device1 = create_device_struct ("Legrand Switch",
	                                        "10000",
	                                        "12345",
	                                        "192.168.1.1",
	                                        "_xulhttp._tcp",
	                                        "Kitchen",
	                                        NULL);


											xmlTextReaderPtr reader;
											xmlChar *name;
											char *temp_desc,*temp_id,*temp_value_url,*temp_type,*temp_unit;
											int ret;
											int firstIteration = 0;
											Service *new_service;
											Parameter *new_parameter;

											reader= xmlReaderForMemory (xml_node,
											                            strlen(xml_node),
																		"test",
																		ENCODING,
																		NULL);



																		if (reader != NULL) {
																			ret = xmlTextReaderRead(reader);
																			while (ret == 1) {
																				name = xmlTextReaderName(reader);

																				if(strcmp((char *)name,"service") == 0 && firstIteration == 0) {
																					temp_desc = (char *)xmlTextReaderGetAttribute (reader,"desc");
																					temp_id = (char *)xmlTextReaderGetAttribute (reader,"id");
																					temp_value_url = (char *)xmlTextReaderGetAttribute (reader,"value_url");
																					temp_type = (char *)xmlTextReaderGetAttribute (reader,"type");
																					temp_unit = (char *)xmlTextReaderGetAttribute (reader,"unit");
																					}

																					if(strcmp((char *)name,"parameter") == 0 && firstIteration == 1){
																						new_parameter = create_parameter_struct(
																						                                        (char *)xmlTextReaderGetAttribute (reader,"id"),
																																(char *)xmlTextReaderGetAttribute (reader,"max"),
																																(char *)xmlTextReaderGetAttribute (reader,"min"),
																																(char *)xmlTextReaderGetAttribute (reader,"scale"),
																																(char *)xmlTextReaderGetAttribute (reader,"step"),
																																(char *)xmlTextReaderGetAttribute (reader,"type"),
																																(char *)xmlTextReaderGetAttribute (reader,"unit"),
																																(char *)xmlTextReaderGetAttribute (reader,"values"));
																																new_service->parameter_list = add_parameter_to_list(new_parameter, new_service->parameter_list);
																																}

																																if(strcmp((char *)name,"parameter") == 0 && firstIteration == 0){
																																	firstIteration = 1;
																																	new_parameter = create_parameter_struct(
																																	                                        (char *)xmlTextReaderGetAttribute (reader,"id"),
																																											(char *)xmlTextReaderGetAttribute (reader,"max"),
																																											(char *)xmlTextReaderGetAttribute (reader,"min"),
																																											(char *)xmlTextReaderGetAttribute (reader,"scale"),
																																											(char *)xmlTextReaderGetAttribute (reader,"step"),
																																											(char *)xmlTextReaderGetAttribute (reader,"type"),
																																											(char *)xmlTextReaderGetAttribute (reader,"unit"),
																																											(char *)xmlTextReaderGetAttribute (reader,"values"));

																																											new_service = create_service_struct(temp_desc,
																																											                                    temp_id,
																																											                                    temp_value_url,
																																											                                    temp_type,
																																											                                    temp_unit,
																																											                                    device1, // A changer
																																											                                    get_function_service1, // A changer
																																											                                    put_function_service1, // A changer
																																											                                    new_parameter);
																																																				}

																																																				ret = xmlTextReaderRead(reader);
																																																				}

																																																				xmlFreeTextReader(reader);

																																																				return new_service;
																																																				if (ret != 0) {
																																																					printf("failed to parse\n");
																																																					return NULL;
																																																					}
																																																					} else {
																																																						printf("Fail");
																																																						return NULL;
																																																						}
																																																						}*/

