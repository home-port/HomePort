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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mxml.h>

#include "hpd_error.h"
#include "utlist.h"

#define MHD_MAX_BUFFER_SIZE 10

/**
 * The structure Service containing all the Attributes that a Service possess
 */
typedef struct Service Service;
/**
 * The structure Service containing all the Attributes that a Parameter possess
 */
typedef struct Parameter Parameter;
/**
 * The structure serviceElement used to store Services in lists
 */
typedef struct ServiceElement ServiceElement;

typedef size_t (*serviceGetFunction) (Service* service, char *buffer, size_t max_buffer_size);

typedef size_t (*servicePutFunction) (Service* service, char *buffer, size_t max_buffer_size, char *put_value);

struct Service
{
  char *description;/**<The Service description*/
  int isActuator; /**<Determine if the service is an actuator or a sensro */
  char *type;/**<The Service type*/
  char *unit;/**<The unit provided by the Service*/
  serviceGetFunction getFunction;/**<A pointer to the GET function of the Service*/
  servicePutFunction putFunction;/**<A pointer to the PUT function of the Service*/
  Parameter *parameter;/**<The first Parameter of the Parameter List*/
  void* data;/**<Pointer used for the used to store its data*/
  char *id;/**<The Service ID*/
  char *uri;/**<The Service URI*/
};

Service* 	serviceNew( char *description, int isActuator, char *type, char *unit, serviceGetFunction getFunction, servicePutFunction putFunction, Parameter *parameter, void* data); 
void 		serviceFree( Service *service ); 
void 		serviceSetId( Service *service, char *id );
void 		serviceSetUri( Service *service, char *uri );
mxml_node_t* 	serviceToXml(Service *service, mxml_node_t *parent);

struct ServiceElement
{
  Service *service;
  ServiceElement *next;
  ServiceElement *prev;
};

ServiceElement* serviceElementNew( Service *service );
void 		serviceElementFree( ServiceElement *serviceElement );

struct Parameter
{
  char *max;/**<The maximum value of the Parameter*/
  char *min;/**<The minimum value of the Parameter*/
  char *scale;/**<The Scale of the Parameter*/
  char *step;/**<The Step of the values of the Parameter*/
  char *type;/**<The Type of values for the Parameter*/
  char *unit;/**<The Unit of the values of the Parameter*/
  char *values;/**<The possible values for the Parameter*/
};

Parameter* 	parameterNew( char *max, char *min, char *scale, char *step, char *type, char *unit, char *values );
void 		parameterFree( Parameter *parameter );


#endif
