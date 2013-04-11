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

#include "libWebserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>

static struct libws_instance *libws_http = NULL;

static int on_begin(struct libws_request *req)
{
   printf("New message\n");

   return 0;
}

static int on_method(struct libws_request *req,
      const char *buf, size_t len)
{
   printf("Method: %.*s\n", (int)len, buf);

   return 0;
}

static int on_url(struct libws_request *req,
      const char *buf, size_t len)
{
   printf("Url chunk: %.*s\n", (int)len, buf);

   return 0;
}

static int on_url_complete(struct libws_request *req)
{
   printf("Url complete\n");

   return 0;
}

static int on_header_field(struct libws_request *req,
      const char *buf, size_t len)
{
   printf("Header field chunk: %.*s\n", (int)len, buf);

   return 0;
}

static int on_header_value(struct libws_request *req,
      const char *buf, size_t len)
{
   printf("Header value chunk: %.*s\n", (int)len, buf);

   return 0;
}

static int on_header_complete(struct libws_request *req)
{
   printf("Header complete\n");

   return 0;
}

static int on_body(struct libws_request *req,
      const char *buf, size_t len)
{
   printf("Body chunk: %.*s\n", (int)len, buf);

   return 0;
}

static int on_complete(struct libws_request *req)
{
   printf("Message complete\n");

   return 0;
}

static void exit_cb(int sig)
{
   if (libws_http != NULL) {
      libws_instance_stop(libws_http);
      libws_instance_destroy(libws_http);
   }
   printf("Exiting...\n");
   exit(sig);
}

int main()
{
   struct ev_loop *loop = EV_DEFAULT;

   struct libws_settings settings = LIBWS_SETTINGS_DEFAULT;
   settings.port = LIBWS_PORT_HTTP_ALT;
   settings.on_request_begin = on_begin;
   settings.on_request_method = on_method;
   settings.on_request_url = on_url;
   settings.on_request_url_complete = on_url_complete;
   settings.on_request_header_field = on_header_field;
   settings.on_request_header_value = on_header_value;
   settings.on_request_header_complete = on_header_complete;
   settings.on_request_body = on_body;
   settings.on_request_complete = on_complete;

   signal(SIGINT, exit_cb);
   signal(SIGTERM, exit_cb);

#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Init webserver and start it
   libws_http = libws_instance_create(&settings, loop);
   libws_instance_start(libws_http);

   // Start the loop
   ev_run(loop, 0);

   exit(0);
   return 0;
}
