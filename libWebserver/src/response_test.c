// response_test.c

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

#include "response.c"
#include "../../unit_test.h"

TEST_START("response.c");

TEST(public interface)
   struct ws_settings settings;
   struct ws_request *req = libws_request_create(NULL, &settings);
   struct ws_response *res = ws_response_create(req, WS_HTTP_200);

   ASSERT_NULL(res->client);
   ASSERT_EQUAL(res->state, S_START);
   ASSERT_STR_EQUAL(res->msg, "HTTP/1.1 200 OK\r\n");

   ws_response_add_header_field(res, "hd", 2);
   ASSERT_EQUAL(res->state, S_HEADER_FIELD);
   ASSERT_STR_EQUAL(res->msg, "HTTP/1.1 200 OK\r\nhd");

   ws_response_add_header_field(res, "r1", 2);
   ASSERT_EQUAL(res->state, S_HEADER_FIELD);
   ASSERT_STR_EQUAL(res->msg, "HTTP/1.1 200 OK\r\nhdr1");
   
   ws_response_add_header_value(res, "valu", 4);
   ASSERT_EQUAL(res->state, S_HEADER_VALUE);
   ASSERT_STR_EQUAL(res->msg, "HTTP/1.1 200 OK\r\nhdr1:valu");

   ws_response_add_header_value(res, "e1", 2);
   ASSERT_EQUAL(res->state, S_HEADER_VALUE);
   ASSERT_STR_EQUAL(res->msg, "HTTP/1.1 200 OK\r\nhdr1:value1");

   ws_response_add_header_field(res, "hdr2", 4);
   ASSERT_EQUAL(res->state, S_HEADER_FIELD);
   ASSERT_STR_EQUAL(res->msg, "HTTP/1.1 200 OK\r\nhdr1:value1\r\nhdr2");
   
   ws_response_add_header_value(res, "value2", 6);
   ASSERT_EQUAL(res->state, S_HEADER_VALUE);
   ASSERT_STR_EQUAL(res->msg, "HTTP/1.1 200 OK\r\nhdr1:value1\r\nhdr2:value2");

   ws_response_add_body(res, "A looo", 6);
   ASSERT_EQUAL(res->state, S_BODY);
   ASSERT_STR_EQUAL(res->msg, "HTTP/1.1 200 OK\r\nhdr1:value1\r\nhdr2:value2\r\n\r\nA looo");

   ws_response_add_body(res, "oong body", 9);
   ASSERT_EQUAL(res->state, S_BODY);
   ASSERT_STR_EQUAL(res->msg, "HTTP/1.1 200 OK\r\nhdr1:value1\r\nhdr2:value2\r\n\r\nA looooong body");

   ws_response_destroy(res);
   libws_request_destroy(req);
TSET()

TEST_END()

