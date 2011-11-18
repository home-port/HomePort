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
 * @file hpd_services.h
 * @brief  Methods for managing the Service structure
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef SERVICES_H
#define SERVICES_H

#include "hpd_parameters.h"

#include <pthread.h>


#define HPD_SECURE_DEVICE 1 /**< Defines if the Device is a Secured Device. */  
#define HPD_NON_SECURE_DEVICE 0 /**< Defines if the Device is a Non Secured Device. */  
#define MHD_MAX_BUFFER_SIZE 10

/**
 * The structure Device containing all the Attributes that a Device possess
 */
typedef struct Device Device;
/**
 * The structure Service containing all the Attributes that a Service possess
 */
typedef struct Service Service;

typedef size_t (*HPD_GetFunction) (Service* service, char *buffer, size_t max_buffer_size);

typedef size_t (*HPD_PutFunction) (Service* service, char *buffer, size_t max_buffer_size, char *put_value);

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

	Service *service_head;/**<The first Service of the Service List*/
};

struct Service
{
	char *description;/**<The Service description*/
	char *ID;/**<The Service ID*/
	char *value_url;/**<The URL used to retrieve or set the Value of the Service*/
	char *type;/**<The Service type*/
	char *unit;/**<The unit provided by the Service*/
	char *DNS_SD_type;/**<*/
	char* zeroConfName;/**<The name used to advertise the service using ZeroConf*/
	char* get_function_buffer;
	Device *device;/**<The Device that contains the Service*/
	HPD_GetFunction get_function;/**<A pointer to the GET function of the Service*/
	HPD_PutFunction put_function;/**<A pointer to the PUT function of the Service*/
	void* user_data_pointer;/**<Pointer used for the used to store its data*/

	Parameter *parameter_head;/**<The first Parameter of the Parameter List*/

	Service *prev;/**<A pointer to the previous Service*/
	Service *next;/**<A pointer to the next Service*/
	pthread_mutex_t *mutex; /**<A mutex used to access a Service in the list*/
};

Service* create_service_struct(
                               char *description,
                               char *ID,
                               char *type,
                               char *unit,
                               Device *device,
                               HPD_GetFunction get_function,
                               HPD_PutFunction put_function,
                               Parameter *parameter,
                               void* user_data_pointer);

int destroy_service_struct( Service *service ); 

Device* create_device_struct(
                             char *description,
                             char *ID,
                             char *vendorID,
                             char *productID,
                             char *version,
                             char *IP,
                             char *port,
                             char *location,
                             char *type,
                             int secure_device);

int destroy_device_struct( Device *device ); 

int add_parameter_to_service( Parameter *parameter, Service *service );

int remove_parameter_from_service( Parameter *parameter, Service *service );

int add_service_to_device( Service *service, Device *device );

int remove_service_from_device( Service *service, Device *device );

int cmp_Service( Service *a, Service *b );

Service* matching_service( Service *_service_head, char *url );

#endif
