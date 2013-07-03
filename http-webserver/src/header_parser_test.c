// header_parser_test.c

/*  Copyright 2013 Aalborg University. All rights reserved.
*   
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*  
*  2. Redistributions in binary form must reproduce the above copyright
*  notice, this list of conditions and the following disclaimer in the
*  documentation and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
*  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
*  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*  
*  The views and conclusions contained in the software and
*  documentation are those of the authors and should not be interpreted
*  as representing official policies, either expressed.
*/

#include "unit_test.h"
#include "header_parser.c"
#include <stdio.h>

struct data {
   char *field;
   char *value;
   int count;
   int errors;
};

void on_field_value(void * _data,
                    const char* field, size_t field_length,
                    const char* value, size_t value_length)
{
   int _errors = 0;
   struct data *data = _data;
   int i;
   char *expect_field = data->field;
   char *expect_value = data->value;
   for (i = 0; i < data->count; i++) {
      expect_field = &expect_field[strlen(expect_field)+1];
      expect_value = &expect_value[strlen(expect_value)+1];
   }
	ASSERT_EQUAL(strncmp(field, expect_field, field_length), 0);
	ASSERT_EQUAL(strncmp(value, expect_value, value_length), 0);
   data->count++;
   data->errors += _errors;
}

TEST_START("header_parser.c")

   struct header_parser_settings settings = HEADER_PARSER_SETTINGS_DEFAULT;
   settings.on_field_value_pair = &on_field_value;

TEST(Header parser test)

   struct data data;
   data.field = "cat\0dog";
   data.value = "yes\0yes, this is dog";
   data.count = 0;
   data.errors = 0;
   settings.data = &data;
   
   struct header_parser_instance *hp = hp_create(&settings);

	hp_on_header_field(hp, "cat",3);
	hp_on_header_value(hp, "yes",3);

	hp_on_header_field(hp, "dog",3);
	hp_on_header_value(hp, "yes, thi",8);
	hp_on_header_value(hp, "s is dog",8);

	hp_on_header_complete(hp);

	hp_destroy(hp);

   _errors += data.errors;

   ASSERT_EQUAL(data.count, 2);

TSET()

TEST_END()
