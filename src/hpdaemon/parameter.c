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

#include "parameter.h"

/**
 * Creates the structure Parameter with all its parameters
 *
 * @param _ID The Parameter ID
 *
 * @param _max The maximum value of the Parameter
 *
 * @param _min The minimum value of the Parameter
 *
 * @param _scale The Scale of the Parameter
 *
 * @param _step The Step of the values of the Parameter
 *
 * @param _type The Type of values for the Parameter
 *
 * @param _unit The Unit of the values of the Parameter
 *
 * @param _values The possible values for the Parameter
 *
 * @return returns the Parameter or NULL if failed, note that the ID can not be NULL
 */
Parameter* create_parameter_struct(
                                   char *_ID,
                                   char *_max,
                                   char *_min,
                                   char *_scale,
                                   char *_step,
                                   char *_type,
                                   char *_unit,
                                   char *_values){
	Parameter *_parameter = (Parameter*)malloc(sizeof(Parameter));
	_parameter->ID = _ID;
	if(_parameter->ID==NULL)
    {
        printf("Parameter ID cannot be NULL\n");
        return NULL;
    }
	_parameter->max=_max;
	_parameter->min=_min;
	_parameter->scale=_scale;
	_parameter->step=_step;
	_parameter->type=_type;
	_parameter->unit=_unit;
	_parameter->values=_values;

	return _parameter;
}


/**
 * Frees all the memory allocated for the Parameter. Note
 * that it only frees the memory used by the API, if the
 * user allocates memory for Parameter attributes, he needs
 * to free it before/after calling this function.
 *
 * @param _parameter The parameter to free
 *
 * @return 
 */
void free_parameter_struct(Parameter *_parameter){

	if(_parameter)
        	free(_parameter);
}


/**
 * Creates a Parameter Element for the Parameter List
 *
 * @param _parameter The Parameter 
 *
 * @return returns the Parameter Element if successfull NULL if failed
 */
ParameterElement* create_parameter_element(Parameter *_parameter)
{
    ParameterElement *_parameter_list_element = (ParameterElement*)malloc(sizeof(ParameterElement));
    _parameter_list_element->parameter = _parameter;
    _parameter_list_element->next = NULL;
    
    return _parameter_list_element;
}

/**
 * Method that compares all the Parameter's Attributes
 *
 * @param a The Parameter to compare
 *
 * @param b The other Parameter to compare
 *
 * @return 0 if the same -1 or 1 if not
 */
int cmp_ParameterElement(ParameterElement *a, ParameterElement *b)
{
    if(a->parameter->ID != NULL){
        if(strcmp(a->parameter->ID,b->parameter->ID) != 0)
	   return strcmp(a->parameter->ID,b->parameter->ID);
    }
    if(a->parameter->max != NULL){
        if(strcmp(a->parameter->max,b->parameter->max) != 0)
	   return strcmp(a->parameter->max,b->parameter->max);
    }
    if(a->parameter->min != NULL){
        if(strcmp(a->parameter->min,b->parameter->min) != 0)
	   return strcmp(a->parameter->min,b->parameter->min);
    }
    if(a->parameter->scale != NULL){
        if(strcmp(a->parameter->scale,b->parameter->scale) != 0)
	   return strcmp(a->parameter->scale,b->parameter->scale);
    }
    if(a->parameter->step != NULL){
        if(strcmp(a->parameter->step,b->parameter->step) != 0)
	   return strcmp(a->parameter->step,b->parameter->step);
    }
    if(a->parameter->type != NULL){
        if(strcmp(a->parameter->type ,b->parameter->type) != 0)
	   return strcmp(a->parameter->type,b->parameter->type);
    }
    if(a->parameter->unit != NULL){
        if(strcmp(a->parameter->unit,b->parameter->unit) != 0)
	   return strcmp(a->parameter->unit,b->parameter->unit);
    }
    if(a->parameter->values != NULL){
        if(strcmp(a->parameter->values,b->parameter->values) != 0)
	   return strcmp(a->parameter->values,b->parameter->values);
    }
    return 0;
}
