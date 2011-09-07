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

#ifndef SERVICES_H
#define SERVICES_H

#include "parameter.h"

#include <pthread.h>


#define HPD_SECURE_DEVICE 1 /**< Defines if the Device is a Secured Device. */  
#define HPD_NON_SECURE_DEVICE 0 /**< Defines if the Device is a Non Secured Device. */  

/**
 * The structure ServiceElement used in the Service List, it contains a Service and a pointer on the next ServiceElement
 */
typedef struct ServiceElement ServiceElement;
/**
 * The structure Device containing all the Attributes that a Device possess
 */
typedef struct Device Device;
/**
 * The structure Service containing all the Attributes that a Service possess
 */
typedef struct Service Service;

struct ServiceElement
{
    Service *service;/**<The Service*/
    ServiceElement *next;/**<A pointer to the next ServiceElement*/
    pthread_mutex_t mutex; /**<A mutex used to access a Service in the list*/
};

struct Device
{
    char *description;/**<The Device description*/
    char *ID;/**<The Device ID*/
    char *vendorID;/**<The ID of the vendor*/
    char *productID;/**<The ID of the product*/
    char *version;/**<The Device version*/
	char *IP;/**<The IP address of the Device*/
	char *port;/**<The port that the Device uses*/
	char *location;/**<The location of the Device*/
	char *type;/**<The Device type*/
    int secure_device;/**<A variable that states if the Device is Secure or not (HPD_SECURE_DEVICE or HPD_NON_SECURE_DEVICE)*/
    ServiceElement *service_head;/**<The first ServiceElement of the Service List*/
};

struct Service
{
    char *description;/**<The Service description*/
    char *ID;/**<The Service ID*/
    char *value_url;/**<The URL used to retrieve or set the Value of the Service*/
    char *type;/**<The Service type*/
    char *unit;/**<The unit provided by the Service*/
    char *DNS_SD_type;/**<*/
    Device *device;/**<The Device that contains the Service*/
    char* (*get_function)(Service*);/**<A pointer to the GET function of the Service*/
    char* (*put_function)(Service*, char*);/**<A pointer to the PUT function of the Service*/
    ParameterElement *parameter_head;/**<The first ParameterElement of the Parameter List*/
    char* zeroConfName;/**<The name used to advertise the service using ZeroConf*/
};

Service* create_service_struct(
                               char *_description,
                               char *_ID,
                               char *_type,
                               char *_unit,
                               Device *_device,
                               char* (*_get_function)(Service*),
                               char* (*_put_function)(Service*, char*),
                               Parameter *_parameter);

int destroy_service_struct(Service *_service); 

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
                             int _secure_device);

int destroy_device_struct(Device *_device); 

int add_parameter_to_service(Parameter *_parameter, Service *_service);

int remove_parameter_from_service(Parameter *_parameter, Service *_service);

int add_service_to_device(Service *_service, Device *_device);

int remove_service_from_device(Service *_service, Device *_device);

ServiceElement* create_service_element(Service *_service);

int cmp_ServiceElement( ServiceElement *a, ServiceElement *b);

ServiceElement* matching_service(ServiceElement *_service_head, char *url);

#endif
