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
#include "libREST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>

// Server instance
static struct ws *ws = NULL;
static struct lr *lr = NULL;

// Handle requests
static int request_begin_cb(void *_req)
{
   struct ws_request *req = _req;
   void *lr_req = lr_request_create(lr);

   ws_request_set_data(req, lr_req);

   return 0;
}

static void my_get(const char *url, size_t len)
{
   printf("Get on: %.*s\n", (int)len, url);
}

// Clean up requests
static int request_cmpl_cb(void *_lr_req)
{
   struct lr_request *lr_req = _lr_req;

   lr_request_cmpl(lr_req);
   lr_request_destroy(lr_req);

   return 0;
}

// Shutdown webserver and exit
static void exit_cb(int sig)
{
   if (ws != NULL) {
      ws_stop(ws);
      ws_destroy(ws);
   }
   if (lr != NULL) {
      lr_destroy(lr);
   }
   printf("Exiting...\n");
   exit(sig);
}

// Main function
int main(int argc, char *argv[])
{
   struct ev_loop *loop = EV_DEFAULT;
   struct ws_settings ws_settings = WS_SETTINGS_DEFAULT;

   // Set settings for webserver
   ws_settings.port = WS_PORT_HTTP_ALT;

   // Set up url parser for url parsing
   ws_settings.on_request_begin = request_begin_cb;
   ws_settings.on_request_method = lr_request_method;
   ws_settings.on_request_url = lr_request_url;
   ws_settings.on_request_url_complete = lr_request_url_cmpl;
   ws_settings.on_request_header_field = lr_request_hdr_field;
   ws_settings.on_request_header_value = lr_request_hdr_value;
   ws_settings.on_request_header_complete = lr_request_hdr_cmpl;
   ws_settings.on_request_body = lr_request_body;
   ws_settings.on_request_complete = request_cmpl_cb;

   // Connect signals for handling exiting correctly
   signal(SIGINT, exit_cb);
   signal(SIGTERM, exit_cb);

#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Init libREST
   lr = lr_create();

   // Create services
   lr_register_service(lr, "/homeport/devices", my_get, NULL, NULL, NULL);

   // Create webserver
   ws = ws_create(&ws_settings, loop);
   ws_start(ws);

   // Start the event loop
   ev_run(loop, 0);

   // Exit
   exit(0);
   return 0;
}
