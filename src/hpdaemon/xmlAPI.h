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

#ifndef XMLAPI_H
#define XMLAPI_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "services.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/encoding.h>
#include <libxml/xpath.h>

#define	ENCODING "UTF-8"
#define	XML_FILE_NAME "services.xml"
#define	DEVICE_LIST_ID "12345"

int init_xml_file(char *name, char *id); // DONE
int device_is_in_xml_file(Device *device); // DONE
int service_is_in_xml_file(Service *service); // DONE
xmlXPathObjectPtr get_node_set(xmlDocPtr doc, xmlChar *xpath); // DONE
int add_device_to_xml(Device *device_to_add); //DONE
int add_service_to_xml(Service *service_to_add); //DONE
xmlChar * get_xml_value(char* value); //DONE
char * timestamp(); //DONE
int remove_service_from_XML(Service *_service); //DONE
int remove_device_from_XML(Device *_device); //DONE
int delete_xml(char* xml_file_path); //DONE
char* get_value_from_xml_value(char* _xml_value); //DONE
xmlChar *extract_service_xml(Service *_service_to_extract);//DONE
void generate_get_and_put_functions_from_xml(char* _xml_file_name); //IN PROGRESS
/*Service *get_service_from_xml_node(char *xml_node);*/ //IN PROGRESS

#endif
