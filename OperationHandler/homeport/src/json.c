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

#include "json.h"
#include "hpd_configuration.h"
#include <jansson.h>

char *
jsonGetConfiguration(HomePort *homeport)
{
   char *res;
   json_t * configurationJson = configurationToJson( homeport->configuration );
   res = json_dumps( configurationJson, 0 );
   json_decref(configurationJson);
   return res;
}

char*
jsonGetState(char *state)
{
  json_t *json=NULL;
  json_t *value=NULL;

  if( ( json = json_object() ) == NULL )
  {
    goto error;
  }
  if( ( ( value = json_string(state) ) == NULL ) || ( json_object_set_new(json, "value", value) != 0 ) )
  {
    goto error;
  }

  char *ret = json_dumps( json, 0 );
  json_decref(json);
  return ret;

error:
  if(value) json_decref(value);
  if(json) json_decref(json);
  return NULL;
}

const char*
jsonParseState(char *json_value)
{
  json_t *json = NULL;
  json_error_t *error=NULL;
  json_t *value = NULL;

  if( ( json = json_loads(json_value, 0, error) ) == NULL )
  {
    goto error;
  }

  if( json_is_object(json) == 0 )
  {
    goto error;
  }

  if( ( value = json_object_get(json, "value") ) == NULL )
  {
    goto error;
  }

  const char *ret = json_string_value(value);

  json_decref(json);

  return ret;

error:
  return NULL;
}


