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

#include "http-webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>

// The server instance
static struct httpws *server = NULL;

// Handle correct exiting
static void exit_handler(int sig)
{
    // Stop server
    if (server) {
        httpws_stop(server);
        httpws_destroy(server);
    }

    // Exit
    printf("Exiting....\n");
    exit(sig);
}

void header_printer(void *data, const char* key, const char* value)
{
    printf("\tHeader key/value = {%s : %s}\n",key,value);
}

int handle_request(struct httpws *ins, struct http_request *req, void* ws_ctx, void** req_data)
{
    const char *method = http_method_str(http_request_get_method(req));
    const char *url = http_request_get_url(req);

    struct lm *headers = http_request_get_headers(req);

    printf("Got %s request on %s\n", method, url);

    // print headers
    lm_map(headers, header_printer, NULL);

    char *body1 = "<html><body><h1>Hello</h1>Your language: ";
    char *body2 = lm_find(headers, "Accept-Language");
    if (body2 == NULL) body2 = "n/a";
    char *body3 = "</body></html>";
    char *body = malloc((strlen(body1)+strlen(body2)+strlen(body3)+1)*sizeof(char));
    sprintf(body, "%s%s%s",body1, body2, body3);

    struct http_response *res = http_response_create(req, WS_HTTP_200);
    http_response_sendf(res, body);

    free(body);

    return 0;
}

// Main function
int main(int argc, char *argv[])
{
    // The event loop for the webserver to run on
    hpd_ev_loop_t *loop = EV_DEFAULT;

    // Set settings for the webserver
    struct httpws_settings settings = HTTPWS_SETTINGS_DEFAULT;
    settings.on_req_cmpl = handle_request;
    settings.port = HPD_P_HTTP_ALT;

    // Inform if we have been built with debug flag
#ifdef DEBUG
    printf("Debugging is set\n");
#endif

    // Register signals for correctly exiting
    signal(SIGINT, exit_handler);
    signal(SIGTERM, exit_handler);

    // Create server
    // TODO Parameter cannot be null !
    server = httpws_create(&settings, NULL, loop);

    // Start the event loop and webserver
    if (!httpws_start(server))
        ev_run(loop, 0);

    // Exit
    exit_handler(0);
    return 0;
}
