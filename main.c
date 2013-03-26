// main.c

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

#include <stdio.h>
#include "webserver.h"

// TODO: Return true/false to stop parser
struct ws_msg* dummy_receive_header(const char* url, const char* method)
{
   printf("Dummy Header Callback URL: %s METHOD: %s\n",url, method);

   struct ws_msg *msg = ws_msg_create(HTTP_200, "test body\n");
   return msg;
}

struct ws_msg* dummy_receive_body(const char* body)
{
   printf("Dummy body callback:%s\n",body);

   struct ws_msg *msg = ws_msg_create(HTTP_200, NULL);
   return msg;
}

int main()
{
   struct ws_instance *ws_http;
   struct ev_loop *loop = EV_DEFAULT;

#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Init webserver and start it
   ws_http = ws_create_instance("http", &dummy_receive_header, &dummy_receive_body, loop);
   ws_start(ws_http);

   // Start the loop
   ev_run(loop, 0);

   // Clean up the webserver
   ws_stop(ws_http);
   ws_free_instance(ws_http);

   return 0;
}
