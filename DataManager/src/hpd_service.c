/*Copyright 2011 Aalborg University. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of
  conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list
  of conditions and the following disclaimer in the documentation and/or other materials
  provided with the distribution.

  THIS SOFTWARE IS PROVidED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCidENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed*/

/**
 * @file hpd_services.c
 * @brief  Methods for managing the Service structure
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include <stdio.h>
#include "hpd_service.h"
#include "utlist.h"
#include "hpd_error.h"
#include "hp_macros.h"

/**
 * Creates the structure Service with all its parameters
 *
 * @param description The Service description
 *
 * @param id The Service id
 *
 * @param type The Service type
 *
 * @param unit The unit provided by the Service
 *
 * @param device The Device that contains the 
 * 			   Service
 *
 * @param get_function A pointer to the GET function
 * 				   of the Service
 *
 * @param put_function A pointer to the PUT function
 *				    of the Service
 *
 * @param parameter The first parameter of the Service
 *
 * @param user_data_pointer A generic pointer used by the user to store his structures
 *
 * @return returns the Service or NULL if failed, note that
 * 		id, type, device and get_function can not be NULL
 */
Service* 
serviceNew(
    char *description,
    int isActuator,
    char *type,
    char *unit,
    serviceGetFunction getFunction,
    servicePutFunction putFunction,
    Parameter *parameter,
    void* data)
{
  Service *service;

  alloc_struct(service);

  service->id = NULL;
  service->uri = NULL;

  null_ok_string_copy(service->description, description);

  service->isActuator = isActuator;

  null_nok_string_copy(service->type, type);

  null_ok_string_copy(service->unit, unit);

  null_nok_pointer_ass(service->parameter, parameter);

  service->getFunction = getFunction;

  service->putFunction = putFunction;

  service->data = data;

  return service;

cleanup:
  serviceFree(service);
  return NULL;
}

/**
 * Frees all the memory allocated for the Service. Note
 * that it only frees the memory used by the API, if the
 * user allocates memory for Services attributes, he needs
 * to free it before/after calling this function. Also note
 * that the user can't destroy a Service that is
 * registered on the server.
 *
 * @param service_to_destroy The service to destroy
 *
 * @return returns A HPD error code
 */
void
serviceFree( Service *service )
{

  if( service != NULL )
  {
    free_pointer(service->description);
    free_pointer(service->type);
    free_pointer(service->unit);
    free_pointer(service->id);
    free_pointer(service->uri);
    parameterFree(service->parameter);
    free(service);
  }
}

mxml_node_t *
serviceToXml(Service *service, mxml_node_t *parent)
{
  mxml_node_t *serviceXml;

  serviceXml = mxmlNewElement(parent, "service");
  if(service->description != NULL) mxmlElementSetAttr(serviceXml, "desc", service->description);
  if(service->id != NULL) mxmlElementSetAttr(serviceXml, "id", service->id);
  if(service->uri != NULL) mxmlElementSetAttr(serviceXml, "uri", service->uri);
  mxmlElementSetAttr(serviceXml, "isActuator", service->isActuator ? "1" : "0");
  if(service->type != NULL) mxmlElementSetAttr(serviceXml, "type", service->type);
  if(service->unit != NULL) mxmlElementSetAttr(serviceXml, "unit", service->unit);


  if(service->parameter != NULL)
  {
    mxml_node_t *parameterXml = mxmlNewElement(serviceXml, "parameter");
    if(service->parameter->max != NULL) mxmlElementSetAttr(parameterXml, "max", service->parameter->max);
    if(service->parameter->min != NULL) mxmlElementSetAttr(parameterXml, "min", service->parameter->min);
    if(service->parameter->scale != NULL) mxmlElementSetAttr(parameterXml, "scale", service->parameter->scale);
    if(service->parameter->step != NULL) mxmlElementSetAttr(parameterXml, "step", service->parameter->step);
    if(service->parameter->type != NULL) mxmlElementSetAttr(parameterXml, "type", service->parameter->type);
    if(service->parameter->unit != NULL) mxmlElementSetAttr(parameterXml, "unit", service->parameter->unit);
    if(service->parameter->values != NULL) mxmlElementSetAttr(parameterXml, "values", service->parameter->values);
  }


  return serviceXml;
}

void
serviceSetId( Service *service, char *id )
{
  service->id = id;
}

void
serviceSetUri( Service *service, char *uri )
{
  service->uri = uri;
}

ServiceElement* 
serviceElementNew( Service *service )
{
  ServiceElement *serviceElement;
  alloc_struct(serviceElement);

  null_nok_pointer_ass(serviceElement->service, service);

  return serviceElement;

cleanup:
  serviceElementFree(serviceElement);
  return NULL;
}


void 
serviceElementFree( ServiceElement *serviceElement )
{
  free_pointer(serviceElement);
}

/**
 * Creates the structure Parameter with all its parameters
 *
 * @param id The Parameter id
 *
 * @param max The maximum value of the Parameter
 *
 * @param min The minimum value of the Parameter
 *
 * @param scale The Scale of the Parameter
 *
 * @param step The Step of the values of the Parameter
 *
 * @param type The Type of values for the Parameter
 *
 * @param unit The Unit of the values of the Parameter
 *
 * @param values The possible values for the Parameter
 *
 * @return returns the Parameter or NULL if failed, note that the id can not be NULL
 */
Parameter* 
parameterNew( char *max,
    char *min,
    char *scale,
    char *step,
    char *type,
    char *unit,
    char *values )
{
  Parameter *parameter;
  alloc_struct(parameter);

  null_ok_string_copy(parameter->max, max);
  null_ok_string_copy(parameter->min, min);
  null_ok_string_copy(parameter->scale, scale);
  null_ok_string_copy(parameter->step, step);
  null_ok_string_copy(parameter->type, type);
  null_ok_string_copy(parameter->unit, unit);
  null_ok_string_copy(parameter->values, values);

  return parameter;

cleanup:
  parameterFree(parameter);
  return NULL;
}


/**
 * Frees all the memory allocated for the Parameter. Note
 * that it only frees the memory used by the API, if the
 * user allocates memory for Parameter attributes, he needs
 * to free it before/after calling this function.
 *
 * @param parameter The parameter to free
 *
 * @return 
 */
void 
parameterFree(Parameter *parameter)
{
  if( parameter != NULL )
  {
    free_pointer(parameter->max);
    free_pointer(parameter->min);
    free_pointer(parameter->scale);
    free_pointer(parameter->step);
    free_pointer(parameter->type);
    free_pointer(parameter->unit);
    free_pointer(parameter->values);
    free(parameter);
  }
}

