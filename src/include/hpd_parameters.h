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
 * @file hpd_parameters.h
 * @brief  Methods for managing the Parameter structure
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef PARAMETER_H
#define PARAMETER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Parameter Parameter;
struct Parameter
{
    char *ID; /**<The Parameter ID*/
    char *max;/**<The maximum value of the Parameter*/
	char *min;/**<The minimum value of the Parameter*/
	char *scale;/**<The Scale of the Parameter*/
	char *step;/**<The Step of the values of the Parameter*/
	char *type;/**<The Type of values for the Parameter*/
	char *unit;/**<The Unit of the values of the Parameter*/
	char *values;/**<The possible values for the Parameter*/

	Parameter *prev;/**<A pointer to the previous Parameter*/
   	Parameter *next;/**<A pointer to the next Parameter*/
};

Parameter* create_parameter_struct(
                                   char *ID,
                                   char *max,
                                   char *min,
                                   char *scale,
                                   char *step,
                                   char *type,
                                   char *unit,
                                   char *values );

void free_parameter_struct( Parameter *parameter );

int cmp_Parameter( Parameter *a, Parameter *b );

#endif
