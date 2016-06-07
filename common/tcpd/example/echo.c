/*
 * Copyright 2011 Aalborg University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVidED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 */

#include "hpd_tcpd.h"
#include <stdio.h>
#include <stdlib.h>
#include <ev.h>

// A tcpd instance
static hpd_tcpd_t *ws = NULL;

// Receive messages
static int on_receive(hpd_tcpd_t *instance, hpd_tcpd_conn_t *conn, void *ctx, void **data,
                      const char *buf, size_t len)
{
   hpd_tcpd_conn_sendf(conn, "%.*s", (int) len, buf);
   return 0;
}

// Handle correct exiting
static void exit_handler(int sig)
{
   // Stop tcpd
   if (hpd_ws != NULL) {
      hpd_tcpd_stop(hpd_ws);
      hpd_tcpd_destroy(hpd_ws);
   }

   // Exit
   printf("Exiting....\n");
   exit(sig);
}

// Main function
int main(int argc, char *argv[])
{
   // The event loop for the tcpd to run on
   hpd_ev_loop_t *loop = EV_DEFAULT;

   // Settings for the tcpd
   hpd_tcpd_settings_t settings = HPD_TCPD_SETTINGS_DEFAULT;
   settings.port = HPD_TCPD_P_HTTP_ALT;
   settings.on_receive = on_receive;

   // Inform if we have been built with debug flag
#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Register signals for correctly exiting
   signal(SIGINT, exit_handler);
   signal(SIGTERM, exit_handler);

   // Create tcpd
   hpd_ws = hpd_tcpd_create(NULL, &settings, NULL, NULL);
   hpd_tcpd_start(hpd_ws);

   // Start the event loop and tcpd
   ev_run(loop, 0);

   // Exit
   exit_handler(0);
   return 0;
}
