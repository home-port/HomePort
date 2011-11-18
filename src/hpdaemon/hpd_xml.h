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

#ifndef XMLAPI_H
#define XMLAPI_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <mxml.h>
#include "hpd_services.h"

#define	ENCODING "UTF-8"
#define	XML_FILE_NAME "services.xml"
#define	DEVICE_LIST_ID "12345"

typedef struct serviceXmlFile serviceXmlFile;
struct serviceXmlFile
{
    FILE *fp;/**<The actual File "services.xml"*/
    mxml_node_t *xml_tree;/**<The internal XML File*/
    pthread_mutex_t *mutex;/**<The mutex used to access the file*/
};

int init_xml_file(char *name, char *id);
int device_is_in_xml_file(Device *_device);
int service_is_in_xml_file(Service *_service);
int add_device_to_xml(Device *device_to_add); 
int add_service_to_xml(Service *service_to_add); 
char * get_xml_value(char* value); 
char * get_xml_subscription(char* value, char *url);
char * timestamp(); 
int remove_service_from_XML(Service *_service); 
int remove_device_from_XML(Device *_device); 
int delete_xml(char* xml_file_path); 
char* get_value_from_xml_value(char* _xml_value); 
char *extract_service_xml(Service *_service_to_extract);
const char * whitespace_cb(mxml_node_t *node, int where);
void create_service_xml_file();
void destroy_service_xml_file();
void save_xml_tree();
mxml_node_t *get_xml_node_of_device(Device *_device);
mxml_node_t *get_xml_node_of_service(Service *_service);

#endif

