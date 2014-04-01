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
 * @file hpd_device.h
 * @brief  Methods for managing the Service structure
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "hpd_service.h"
#include "idgen.h"

#define SERVICE_ID_SIZE 4

/**
 * The structure Device containing all the Attributes that a Device possess
 */
typedef struct Device Device;
typedef struct DeviceElement DeviceElement;

struct Device
{
  char *description;/**<The Device description*/
  char *id;/**<The Device ID*/
  char *vendorId;/**<The ID of the vendor*/
  char *productId;/**<The ID of the product*/
  char *version;/**<The Device version*/
  char *location;/**<The location of the Device*/
  char *type;/**<The Device type*/
  ServiceElement *service_head;/**<The first Service of the Service List*/
};

Device* 	deviceNew( char *description, char *vendorId, char *productId, char *version, char *location, char *type);
void 		deviceFree( Device *device ); 
int 		deviceAddService( Device *device, Service *service );
int 		deviceRemoveService( Device *device, Service *service );
Service* 	findService(Device *device, char *service_id);
mxml_node_t* 	deviceToXml(Device *device, mxml_node_t *parent);
json_t* 	deviceToJson(Device *device);
void 		deviceSetId( Device *device, char *id );

struct DeviceElement
{
  Device *device;
  DeviceElement *next;
  DeviceElement *prev;
};

DeviceElement* 	deviceElementNew( Device *device );
void 		deviceElementFree( DeviceElement *device_element_to_destroy );


#endif
