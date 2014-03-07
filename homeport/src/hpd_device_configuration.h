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
 * @file hpd_device_configuration.h
 * @brief  Methods for managing the Configuration of Devices at runtime
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef DEVICE_CONFIGURATION_H
#define DEVICE_CONFIGURATION_H

#include "hpd_services.h"
#include "utlist.h"
#include "hpd_xml.h"

/**
 * 
 */
typedef struct configurationXML configurationXML;

/**
 * 
 */
typedef struct Attribute Attribute;

/**
 * 
 */
typedef struct configurationEntity configurationEntity;

struct configurationXML
{
	Attribute *device_selector_head;/**<*/
	Attribute *device_attribute_head;/**<*/

	Attribute *service_selector_head;/**<*/
	Attribute *service_attribute_head;/**<*/

	Attribute *parameter_selector_head;/**<*/
	Attribute *parameter_attribute_head;/**<*/

	configurationEntity *device_head;/**<*/
	configurationEntity *service_head;/**<*/
	configurationEntity *parameter_head;/**<*/

};

struct configurationEntity
{
	void *entity;/**<*/
	configurationEntity *prev;/**<*/
	configurationEntity *next;/**<*/
};

struct Attribute
{
	char *name;/**<*/
	char *value;/**<*/
	Attribute *prev;/**<*/
	Attribute *next;/**<*/
};

Attribute *new_attribute(const char *name, const char *value);
int destroy_attribute(Attribute *attribute_to_destroy);

configurationEntity *new_configuration_entity(void* entity);
int destroy_configuration_entity(configurationEntity *configuration_entity_to_destroy);

configurationXML *new_configuration_xml();
int destroy_configuration_xml(configurationXML *configuration_xml_to_destroy);

int retrieve_selectors(mxml_node_t *node, configurationXML * configure, int type);
int retrieve_attributes(mxml_node_t *node, configurationXML * configure, int type);

int manage_configuration_xml( char *configuration_xml , ServiceElement *service_head);

int modify_services(configurationXML *configure);
int modify_devices(configurationXML *configure);
int modify_parameters(configurationXML *configure);

int is_legit_device(Device *device, Attribute *attribute_head);
int is_legit_service(Service *service, Attribute *attribute_head);
int is_legit_parameter(Parameter *parameter, Attribute *attribute_head);

int is_device_in_list(configurationXML *configure, Device *device);
int is_service_in_list(configurationXML *configure, Service *service);
int is_parameter_in_list(configurationXML *configure, Service *service);


#endif
