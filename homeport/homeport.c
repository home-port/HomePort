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
#include "url_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>

struct request_data
{
   struct ws_request *req;
   struct url_parser_instance *up;
};

// Server instance
static struct ws *ws = NULL;
static struct url_parser_settings up_settings = URL_PARSER_SETTINGS_DEFAULT;

static int request_begin_cb(void *_req)
{
   struct ws_request *req = _req;
   struct request_data *req_data = malloc(sizeof(struct request_data));
   
   req_data->up=up_create(&up_settings);
   req_data->req = req;

   ws_request_set_data(req,req_data);

   return 0;
}

static int request_url_cb(void *_req_data, const char *chunk, size_t chunk_length)
{
   struct request_data *req_data = _req_data;
   return up_add_chunk(req_data->up, chunk, chunk_length);
}

static int request_url_complete_cb(void *_req_data)
{
   struct request_data *req_data = _req_data;
   return up_complete(req_data->up);
}

static int request_complete_cb(void *_req_data)
{
   struct request_data *req_data = _req_data;
   up_destroy(req_data->up);

   ws_request_set_data(req_data->req, NULL);

   free(req_data);

   return 0;
}

static void url_p_complete(const char* parsed, size_t seg_len)
{
   printf("%.*s\n", (int)seg_len, parsed); 
}

// Shutdown webserver and exit
static void exit_cb(int sig)
{
   if (ws != NULL) {
      ws_stop(ws);
      ws_destroy(ws);
   }
   printf("Exiting...\n");
   exit(sig);
}

// Main function
int main()
{
   struct ev_loop *loop = EV_DEFAULT;
   struct ws_settings ws_settings = WS_SETTINGS_DEFAULT;

   // Set settings for webserver
   ws_settings.port = WS_PORT_HTTP_ALT;

   // Set up url parser for url parsing
   ws_settings.on_request_url = request_url_cb;
   ws_settings.on_request_url_complete = request_url_complete_cb;
   ws_settings.on_request_begin = request_begin_cb;
   ws_settings.on_request_complete = request_complete_cb;

   up_settings.on_complete = url_p_complete;

   // Connect signals for handling exiting correctly
   signal(SIGINT, exit_cb);
   signal(SIGTERM, exit_cb);

#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Init webserver and start it
   ws = ws_create(&ws_settings, loop);
   ws_start(ws);

   // Start the event loop
   ev_run(loop, 0);

   // Exit
   exit(0);
   return 0;
}
