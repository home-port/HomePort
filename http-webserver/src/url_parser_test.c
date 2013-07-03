// url_parser_test.c

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

#include "url_parser.c"
#include "unit_test.h"
#include <string.h>

struct data {
   char *protocol;
   char *host;
   char *port;
   char *path;
   int cur_path;
   char *key;
   int cur_key_value;
   char *value;
   int call_order;
   int errors;
   char *url;
};

void on_begin(void *data)
{
   int _errors = 0;
   struct data *dat = data;
   ASSERT_EQUAL(dat->call_order, 0);
	dat->call_order = dat->call_order | 1;
   dat->errors += _errors;
}

void on_protocol(void *data, const char* protocol, size_t length)
{
   int _errors = 0;
   struct data *dat = data;
   ASSERT_EQUAL(dat->call_order, 1);
   dat->call_order = dat->call_order | 2;
	ASSERT_EQUAL(strncmp(protocol, dat->protocol, length), 0);
   dat->errors += _errors;
   strncat(dat->url, protocol, length);
   strcat(dat->url, "://");
}

void on_host(void *data, const char* host, size_t length)
{
   int _errors = 0;
   struct data *dat = data;
   ASSERT_EQUAL((dat->call_order & 61), 1);
   dat->call_order = dat->call_order | 4;
	ASSERT_EQUAL(strncmp(host, dat->host, length), 0);
   dat->errors += _errors;
   strncat(dat->url, host, length);
}

void on_port(void *data, const char* port, size_t length)
{
   int _errors = 0;
   struct data *dat = data;
   ASSERT_EQUAL((dat->call_order & 61), 5);
   dat->call_order = dat->call_order | 8;
	ASSERT_EQUAL(strncmp(port, dat->port, length), 0);
   dat->errors += _errors;
   strcat(dat->url, ":");
   strncat(dat->url, port, length);
}

void on_path_segment(void *data, const char* seg, size_t length)
{
   int i;
   int _errors = 0;
   struct data *dat = data;
   char *expect = dat->path;
   ASSERT_EQUAL((dat->call_order & 33), 1);
   dat->call_order = dat->call_order | 16;
   for (i = 0; i < dat->cur_path; i++) {
      expect = &expect[strlen(expect)+1];
   }
	ASSERT_EQUAL(strncmp(seg, expect, length), 0);
   dat->cur_path++;
   dat->errors += _errors;
   strcat(dat->url, "/");
   strncat(dat->url, seg, length);
}

void on_key_value(void * data,
                  const char* key, size_t key_length,
                  const char* value, size_t value_length)
{
   int i;
   int _errors = 0;
   struct data *dat = data;
   char *expect_key = dat->key;
   char *expect_val = dat->value;
   ASSERT_EQUAL((dat->call_order & 17), 17);
   dat->call_order = dat->call_order | 32;
   for (i = 0; i < dat->cur_key_value; i++) {
      expect_key = &expect_key[strlen(expect_key)+1];
      expect_val = &expect_val[strlen(expect_val)+1];
   }
	ASSERT_EQUAL(strncmp(key, expect_key, key_length), 0);
	ASSERT_EQUAL(strncmp(value, expect_val, value_length), 0);
   dat->cur_key_value++;
   dat->errors += _errors;
   if (dat->cur_key_value == 1) strcat(dat->url, "?");
   else strcat(dat->url, "&");
   strncat(dat->url, key, key_length);
   strcat(dat->url, "=");
   strncat(dat->url, value, value_length);
}

TEST_START("url_parser.c")

	struct up_settings settings = UP_SETTINGS_DEFAULT;
 	settings.on_begin = &on_begin;
 	settings.on_protocol = &on_protocol;
 	settings.on_host = &on_host;
 	settings.on_port = &on_port;
 	settings.on_path_segment = &on_path_segment;
 	settings.on_key_value = &on_key_value;

TEST(non chunked url parsing)

	char* url = "http://localhost:8080/device/tv?id=1&brand=Apple";

   struct data data;
   data.protocol = "http";
   data.host = "localhost";
   data.port = "8080";
   data.path = "device\0tv";
   data.cur_path = 0;
   data.key = "id\0brand";
   data.cur_key_value = 0;
   data.value = "1\0Apple";
   data.call_order = 0;
   data.errors = 0;
   data.url = malloc((strlen(url)+1)*sizeof(char));
   data.url[0] = '\0';

	struct up *instance = up_create(&settings, &data);

	up_add_chunk(instance, url, strlen(url));
	up_complete(instance);
	up_destroy(instance);
   
   ASSERT_STR_EQUAL(data.url, url);

   _errors += data.errors;

   free(data.url);

TSET()

TEST(chunked url parsing)

	char* url = "http://localhost:8080/device/tv?id=1&brand=Apple";
	char* url1 = "http://localhost:8080/device/";
	char* url2 = "tv?id=1&brand=Apple";

   struct data data;
   data.protocol = "http";
   data.host = "localhost";
   data.port = "8080";
   data.path = "device\0tv";
   data.cur_path = 0;
   data.key = "id\0brand";
   data.cur_key_value = 0;
   data.value = "1\0Apple";
   data.call_order = 0;
   data.errors = 0;
   data.url = malloc((strlen(url1)+strlen(url2)+1)*sizeof(char));
   data.url[0] = '\0';

	struct up *instance = up_create(&settings, &data);

	up_add_chunk(instance, url1, strlen(url1));
	up_add_chunk(instance, url2, strlen(url2));
	up_complete(instance);
	up_destroy(instance);
   
   ASSERT_STR_EQUAL(data.url, url);

   _errors += data.errors;

   free(data.url);

TSET()

TEST(empty path test)

   char *url = "http://localhost:8080";

   struct data data;
   data.protocol = "http";
   data.host = "localhost";
   data.port = "8080";
   data.path = "";
   data.cur_path = 0;
   data.key = "";
   data.cur_key_value = 0;
   data.value = "";
   data.call_order = 0;
   data.errors = 0;
   data.url = malloc((strlen(url)+1)*sizeof(char));
   data.url[0] = '\0';

	struct up *instance = up_create(&settings, &data);

	up_add_chunk(instance, url, strlen(url));
	up_complete(instance);
	up_destroy(instance);

   ASSERT_STR_EQUAL(data.url, url);

   _errors += data.errors;

   free(data.url);

TSET()

TEST(only host test)

   char *url = "http://localhost";

   struct data data;
   data.protocol = "http";
   data.host = "localhost";
   data.port = "";
   data.path = "";
   data.cur_path = 0;
   data.key = "";
   data.cur_key_value = 0;
   data.value = "";
   data.call_order = 0;
   data.errors = 0;
   data.url = malloc((strlen(url)+1)*sizeof(char));
   data.url[0] = '\0';

	struct up *instance = up_create(&settings, &data);

	up_add_chunk(instance, url, strlen(url));
	up_complete(instance);
	up_destroy(instance);

   ASSERT_STR_EQUAL(data.url, url);

   _errors += data.errors;

   free(data.url);

TSET()

TEST(path test)

   char *url = "/devices/";

   struct data data;
   data.protocol = "";
   data.host = "";
   data.port = "";
   data.path = "devices";
   data.cur_path = 0;
   data.key = "";
   data.cur_key_value = 0;
   data.value = "";
   data.call_order = 0;
   data.errors = 0;
   data.url = malloc((strlen(url)+1)*sizeof(char));
   data.url[0] = '\0';

	struct up *instance = up_create(&settings, &data);

	up_add_chunk(instance, url, strlen(url));
	up_complete(instance);
	up_destroy(instance);

   ASSERT_STR_EQUAL(data.url, url);

   _errors += data.errors;

   free(data.url);

TSET()

TEST(slash test)

   char url = '/';

   struct data data;
   data.protocol = "";
   data.host = "";
   data.port = "";
   data.path = "";
   data.cur_path = 0;
   data.key = "";
   data.cur_key_value = 0;
   data.value = "";
   data.call_order = 0;
   data.errors = 0;
   data.url = malloc((1+1)*sizeof(char));
   data.url[0] = '\0';

	struct up *instance = up_create(&settings, &data);

	up_add_chunk(instance, &url, 1);
	up_complete(instance);
	up_destroy(instance);

   ASSERT_STR_EQUAL(data.url, "/");

   _errors += data.errors;

   free(data.url);

TSET()

TEST_END()
