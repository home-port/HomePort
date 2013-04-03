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
#include <string.h>
#include <ev.h>

static struct ws_instance *ws_http = NULL;

static struct ws_response *on_begin(struct ws_request *req)
{
   const char *ip = ws_request_get_client_ip(req);

   printf("Message from %s\n", ip);

   return NULL;
}

static struct ws_response *on_url(struct ws_request *req,
      const char *url)
{
   const char *method = ws_request_get_method_str(req);
   const char *ip = ws_request_get_client_ip(req);

   printf("[%s] %s %s\n", ip, method, url);

   return NULL;
}

static struct ws_response *on_header(struct ws_request *req,
      const char *field, const char *value)
{
   const char *ip = ws_request_get_client_ip(req);

   printf("[%s] Header <%s,%s>\n", ip, field, value);

   return NULL;
}

static struct ws_response *on_header_complete(struct ws_request *req)
{
   const char *ip = ws_request_get_client_ip(req);

   printf("[%s] Headers complete\n", ip);

   return NULL;
}

static struct ws_response *on_body(struct ws_request *req,
      const char *chunk, size_t len)
{
   const char *ip = ws_request_get_client_ip(req);
   
   printf("[%s] Body chunk: '%.*s'", ip, (int)len, chunk);

   return NULL;
}

static struct ws_response *on_complete(struct ws_request *req)
{
   char *msg;
   const char *ip = ws_request_get_client_ip(req);

   printf("[%s] Message complete\n", ip);

   msg = malloc((6+strlen(ip)+1)*sizeof(char));
   sprintf(msg, "Hello %s", ip);

   return ws_response_create(req, WS_HTTP_200, msg);
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
   settings.on_request_begin = on_begin;
   settings.on_request_url = on_url;
   settings.on_request_header = on_header;
   settings.on_request_header_complete = on_header_complete;
   settings.on_request_body = on_body;
   settings.on_request_complete = on_complete;

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
