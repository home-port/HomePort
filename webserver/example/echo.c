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

// A webserver instance
static struct ws *ws = NULL;

// Receive messages
static int on_receive(struct ws *instance, struct ws_conn *conn, void *ctx, void **data,
                      const char *buf, size_t len)
{
   ws_conn_sendf(conn, "%.*s", (int)len, buf);
   return 0;
}

// Handle correct exiting
static void exit_handler(int sig)
{
   // Stop webserver
   if (ws != NULL) {
      ws_stop(ws);
      ws_destroy(ws);
   }

   // Exit
   printf("Exiting....\n");
   exit(sig);
}

// Main function
int main(int argc, char *argv[])
{
   // The event loop for the webserver to run on
   struct ev_loop *loop = EV_DEFAULT;

   // Settings for the webserver
   struct ws_settings settings = WS_SETTINGS_DEFAULT;
   settings.port = WS_PORT_HTTP_ALT;
   settings.on_receive = on_receive;

   // Inform if we have been built with debug flag
#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Register signals for correctly exiting
   signal(SIGINT, exit_handler);
   signal(SIGTERM, exit_handler);

   // Create webserver
   ws = ws_create(&settings, loop);
   ws_start(ws);

   // Start the event loop and webserver
   ev_run(loop, 0);

   // Exit
   exit_handler(0);
   return 0;
}
