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
#include "webserver.h"
#include "request.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/// http-webserver instance struct
struct httpws {
    struct httpws_settings settings; ///< Settings
    struct ws *webserver;            ///< Webserver instance
};

/**
 * Callback for webserver library.
 *
 *  Handles new connections, by creating a request for them.
 *
 *  \param  instance  Webserver instance
 *  \param  conn      Connection
 *  \param  http_ins  The http webserver instance
 *  \param  req       The http request
 *
 *  \return 0 on success, 1 on error
 */
static int on_connect(struct ws *instance, struct ws_conn *conn,
                      void *http_ins, void **req)
{
    struct httpws *parent = http_ins;
    *req = http_request_create(parent, &parent->settings, conn);
    if (!req) {
        fprintf(stderr, "Not enough memory for request\n");
        return 1;
    }

    return 0;
}

/**
 * Callback for webserver library.
 *
 *  Handles reception of data, by supplying it to the http_parser
 *  associated with the request.
 *
 *  \param  instance  Webserver instance
 *  \param  conn      Connection
 *  \param  http_ins  The http webserver instance
 *  \param  req       The http request
 *
 *  \return 0 on success, 1 on error
 */
static int on_receive(struct ws *instance, struct ws_conn *conn,
                      void *http_ins, void **req,
                      const char *buf, size_t len)
{
    http_request_parse(*req, buf, len);

    return 0;
}

/**
 * Callback for webserver library.
 *
 *  Handles closure of connections, by creating a destroying the
 *  request.
 *
 *  \param  instance  Webserver instance
 *  \param  conn      Connection
 *  \param  http_ins  The http webserver instance
 *  \param  req       The http request
 *
 *  \return 0 on success, 1 on error
 */
static int on_disconnect(struct ws *instance, struct ws_conn *conn,
                         void *http_ins, void **req)
{
    http_request_destroy(*req);
    *req = NULL;

    return 0;
}

/**
 * Create a new http-server instance.
 *
 *  Allocates a new http-webserver instance, that should be freed with
 *  httpws_destroy()
 *
 *  The settings is copied to the instance and a webserver instance is
 *  created for it.
 *
 *  \param  settings  The settings for the http-webserver.
 *  \param  loop      The event loop to start the server on.
 *
 *  \returns  The newly created instance.
 */
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
    struct ws_settings ws_settings = WS_SETTINGS_DEFAULT;
    ws_settings.port          = settings->port;
    ws_settings.timeout       = settings->timeout;
    ws_settings.on_connect    = on_connect;
    ws_settings.on_receive    = on_receive;
    ws_settings.on_disconnect = on_disconnect;
    ws_settings.ws_ctx        = instance;

    // Create webserver
    instance->webserver = ws_create(&ws_settings, loop);

    return instance;
}

/**
 * Destroy a http-server instance.
 *
 *  Destroy a http-webserver instance created with httpws_create(). If
 *  the server is started it should be stopped first by httpws_stop().
 *
 *  \param  instance  The http-webserver instance to destroy.
 */
void httpws_destroy(struct httpws *instance)
{
    ws_destroy(instance->webserver);
    free(instance);
}

/**
 * Start a http-server instance.
 *
 *  Starts a created http-webserver.
 *
 *  \param  instance  The instance to start.
 *
 *  \return The error code of ws_start()
 */
int httpws_start(struct httpws *instance)
{
    return ws_start(instance->webserver);
}

/**
 * Stop a http-server instance.
 *
 *  Stops an already started http-webserver instance. All open connections will
 *  be killed without notifying clients, nor sending remaining data.
 *
 *  \param  instance  Instance to stop.
 */
void httpws_stop(struct httpws *instance)
{
    ws_stop(instance->webserver);
}

