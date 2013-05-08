// http-webserver.c

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

#include "http-webserver.h"
#include "webserver.h"
#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/// http-webserver instance struct
struct httpws {
   struct httpws_settings settings;
   struct ws *webserver;
};

/// http-webserver client struct
struct httpws_client {
   struct httpws *instance;
   struct httpws_parser *parser;
   struct ws_client *client;
};

/// Callback for webserver library
static int on_connect(struct ws *instance, struct ws_client *client)
{
   // Alloc client struct
   struct httpws_client *ctx = malloc(sizeof(struct httpws_client));
   if (!ctx) {
      fprintf(stderr, "Not enough memory for client\n");
      return 1;
   }
   
   // Initialise struct
   ctx->instance = ws_get_ctx(instance);
   ctx->parser = httpws_parser_create(ctx, &ctx->instance->settings);
   ctx->client = client;

   // Set it as client context
   ws_client_set_ctx(client, ctx);

   return 0;
}

/// Callback for webserver library
static int on_receive(struct ws *instance, struct ws_client *client,
                      const char *buf, size_t len)
{
   struct httpws_parser *parser = ws_client_get_ctx(client);
   httpws_parser_parse(parser, buf, len);

   return 0;
}

/// Callback for webserver library
static int on_disconnect(struct ws *instance, struct ws_client *client)
{
   struct httpws_client *ctx = ws_client_get_ctx(client);

   httpws_parser_destroy(ctx->parser);
   free(ctx);

   return 0;
}

/// Create a new http-server instance
struct httpws *httpws_create(struct httpws_settings *settings,
                             struct ev_loop *loop)
{
   // Allocate instance
   struct httpws *instance = malloc(sizeof(struct httpws));
   if (instance == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory for a " \
                      "webserver instance\n");
      return NULL;
   }

   // Copy settings
   memcpy(&instance->settings, settings,
          sizeof(struct httpws_settings));

   // Construct settings for webserver
   struct ws_settings ws_settings;
   ws_settings.port          = settings->port;
   ws_settings.on_connect    = on_connect;
   ws_settings.on_receive    = on_receive;
   ws_settings.on_disconnect = on_disconnect;
   ws_settings.ws_ctx        = instance;

   // Create webserver
   instance->webserver = ws_create(&ws_settings, loop);
}

/// Destroy a http-server instance (remember to stop first)
void httpws_destroy(struct httpws *instance)
{
   ws_destroy(instance->webserver);
   free(instance);
}

/// Start a http-server instance
int httpws_start(struct httpws *instance)
{
   return ws_start(instance->webserver);
}

/// Stop a http-server instance
void httpws_stop(struct httpws *instance)
{
   ws_stop(instance->webserver);
}

