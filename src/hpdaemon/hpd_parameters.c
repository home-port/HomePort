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
 * @file hpd_parameters.c
 * @brief  Methods for managing the Parameter structure
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include "hpd_parameters.h"

/**
 * Creates the structure Parameter with all its parameters
 *
 * @param ID The Parameter ID
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
 * @return returns the Parameter or NULL if failed, note that the ID can not be NULL
 */
Parameter* 
create_parameter_struct( char *ID,
                         char *max,
                         char *min,
                         char *scale,
                         char *step,
                         char *type,
                         char *unit,
                         char *values )
{
	Parameter *parameter = (Parameter*)malloc(sizeof(Parameter));
	parameter->ID = ID;
	if( parameter->ID == NULL )
	{
		printf("Parameter ID cannot be NULL\n");
		return NULL;
	}
	parameter->max=max;
	parameter->min=min;
	parameter->scale=scale;
	parameter->step=step;
	parameter->type=type;
	parameter->unit=unit;
	parameter->values=values;

	parameter->prev = NULL;
	parameter->next = NULL;

	return parameter;
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
free_parameter_struct(Parameter *parameter){

	if( parameter )
		free(parameter);
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
int 
cmp_Parameter( Parameter *a, Parameter *b )
{
	if( a->ID != NULL )
	{
		if( strcmp(a->ID,b->ID) != 0 )
			return strcmp(a->ID,b->ID);
	}
	
	if( a->max != NULL )
	{
		if( strcmp(a->max,b->max) != 0 )
			return strcmp(a->max,b->max);
	}
	
	if( a->min != NULL )
	{
		if( strcmp(a->min,b->min) != 0 )
			return strcmp(a->min,b->min);
	}
	
	if( a->scale != NULL )
	{
		if( strcmp(a->scale,b->scale) != 0 )
			return strcmp(a->scale,b->scale);
	}
	
	if( a->step != NULL ){
		if( strcmp(a->step,b->step) != 0 )
			return strcmp(a->step,b->step);
	}
	
	if( a->type != NULL ){
		if( strcmp(a->type ,b->type) != 0 )
			return strcmp(a->type,b->type);
	}
	
	if( a->unit != NULL ){
		if( strcmp(a->unit,b->unit) != 0 )
			return strcmp(a->unit,b->unit);
	}
	
	if( a->values != NULL )
	{
		if( strcmp(a->values,b->values) != 0 )
			return strcmp(a->values,b->values);
	}
	return 0;
}
