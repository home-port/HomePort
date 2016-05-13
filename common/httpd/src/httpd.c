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

#include "httpd.h"
#include "tcpd.h"
#include "request.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/// httpd instance struct
struct hpd_httpd {
    hpd_httpd_settings_t settings; ///< Settings
    hpd_tcpd_t *webserver;            ///< Webserver instance
};

/**
 * Callback for tcpd library.
 *
 *  Handles new connections, by creating a request for them.
 *
 *  \param  instance  Webserver instance
 *  \param  conn      Connection
 *  \param  tcpd_ctx  The http webserver instance
 *  \param  req       The http request
 *
 *  \return 0 on success, 1 on error
 */
static hpd_error_t on_connect(hpd_tcpd_t *instance, hpd_tcpd_conn_t *conn, void *ws_ctx, void **conn_ctx)
{
    hpd_httpd_t *parent = ws_ctx;
    *conn_ctx = http_request_create(parent, &parent->settings, conn);
    if (!conn_ctx) {
        fprintf(stderr, "Not enough memory for request\n");
        return 1;
    }

    return 0;
}

/**
 * Callback for tcpd library.
 *
 *  Handles reception of data, by supplying it to the http_parser
 *  associated with the request.
 *
 *  \param  instance  Webserver instance
 *  \param  conn      Connection
 *  \param  tcpd_ctx  The http webserver instance
 *  \param  req       The http request
 *
 *  \return 0 on success, 1 on error
 */
static hpd_error_t on_receive(hpd_tcpd_t *instance, hpd_tcpd_conn_t *conn, void *ws_ctx, void **conn_ctx, const char *buf, size_t len)
{
    http_request_parse(*conn_ctx, buf, len);

    return 0;
}

/**
 * Callback for tcpd library.
 *
 *  Handles closure of connections, by creating a destroying the
 *  request.
 *
 *  \param  instance  Webserver instance
 *  \param  conn      Connection
 *  \param  tcpd_ctx  The http webserver instance
 *  \param  req       The http request
 *
 *  \return 0 on success, 1 on error
 */
static hpd_error_t on_disconnect(hpd_tcpd_t *instance, hpd_tcpd_conn_t *conn, void *ws_ctx, void **conn_ctx)
{
    http_request_destroy(*conn_ctx);
    *conn_ctx = NULL;

    return 0;
}

/**
 * Create a new httpd instance.
 *
 *  Allocates a new httpd instance, that should be freed with
 *  hpd_httpd_destroy()
 *
 *  The settings is copied to the instance and a webserver instance is
 *  created for it.
 *
 *  \param  settings  The settings for the httpd.
 *  \param  loop      The event loop to start the server on.
 *
 *  \returns  The newly created instance.
 */
hpd_httpd_t *hpd_httpd_create(hpd_httpd_settings_t *settings, hpd_module_t *context, hpd_ev_loop_t *loop)
{
    // Allocate instance
    hpd_httpd_t *instance = malloc(sizeof(hpd_httpd_t));
    if (instance == NULL) {
        fprintf(stderr, "ERROR: Cannot allocate memory for a " \
                      "webserver instance\n");
        return NULL;
    }

    // Copy settings
    memcpy(&instance->settings, settings,
           sizeof(hpd_httpd_settings_t));

    // Construct settings for tcpd
    hpd_tcpd_settings_t ws_settings = HPD_TCPD_SETTINGS_DEFAULT;
    ws_settings.port          = settings->port;
    ws_settings.timeout       = settings->timeout;
    ws_settings.on_connect    = on_connect;
    ws_settings.on_receive    = on_receive;
    ws_settings.on_disconnect = on_disconnect;
    ws_settings.tcpd_ctx        = instance;

    // Create tcpd
    hpd_tcpd_create(&instance->webserver, &ws_settings, context, loop); // TODO Ignoring error

    return instance;
}

/**
 * Destroy a httpd instance.
 *
 *  Destroy a httpd instance created with hpd_httpd_create(). If
 *  the server is started it should be stopped first by hpd_httpd_stop().
 *
 *  \param  instance  The httpd instance to destroy.
 */
void hpd_httpd_destroy(hpd_httpd_t *httpd)
{
    hpd_tcpd_destroy(httpd->webserver); // TODO Ignoring error
    free(httpd);
}

/**
 * Start a httpd instance.
 *
 *  Starts a created httpd.
 *
 *  \param  instance  The instance to start.
 *
 *  \return The error code of hpd_tcpd_start()
 */
int hpd_httpd_start(hpd_httpd_t *httpd)
{
    return hpd_tcpd_start(httpd->webserver);
}

/**
 * Stop a httpd instance.
 *
 *  Stops an already started httpd instance. All open connections will
 *  be killed without notifying clients, nor sending remaining data.
 *
 *  \param  instance  Instance to stop.
 */
void hpd_httpd_stop(hpd_httpd_t *httpd)
{
    hpd_tcpd_stop(httpd->webserver); // TODO Ignoring error
}

