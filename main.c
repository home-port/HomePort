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

#include "webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <ev.h>

static struct ws_instance *ws_http = NULL;

// TODO: Return true/false to stop parser
static struct ws_response* dummy_receive_header(struct ws_request *request)
{
   char *url = ws_request_get_url(request);
   const char *method = ws_request_get_method_str(request);

   printf("Dummy Header Callback URL: %s METHOD: %s\n", url, method);

   struct ws_response *response = ws_response_create(request, WS_HTTP_200,
         NULL);
   return response;
}

static struct ws_response* dummy_receive_body(struct ws_request *request)
{
   char *body = ws_request_get_body(request);

   printf("Dummy body callback:%s\n", body);

   struct ws_response *response = ws_response_create(request, WS_HTTP_200,
         NULL);
   return response;
}

static void exit_cb(int sig)
{
   if (ws_http != NULL) {
      ws_stop(ws_http);
      ws_instance_free(ws_http);
   }
   printf("Exiting...\n");
   exit(sig);
}

int main()
{
   struct ev_loop *loop = EV_DEFAULT;

   struct ws_settings settings = WS_SETTINGS_DEFAULT;
   settings.port = WS_PORT_HTTP_ALT;
   settings.on_request_header_complete = &dummy_receive_header;
   settings.on_request_complete = &dummy_receive_body;

   signal(SIGINT, exit_cb);
   signal(SIGTERM, exit_cb);

#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Init webserver and start it
   ws_http = ws_instance_create(&settings, loop);
   ws_start(ws_http);

   // Start the loop
   ev_run(loop, 0);

   exit(0);
   return 0;
}
