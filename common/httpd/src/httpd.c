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
 * THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
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

#include "hpd-0.6/common/hpd_httpd.h"
#include "hpd-0.6/common/hpd_tcpd.h"
#include "httpd_request.h"
#include "hpd-0.6/hpd_shared_api.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/// httpd instance struct
struct hpd_httpd {
    hpd_httpd_settings_t settings; ///< Settings
    hpd_tcpd_t *webserver; ///< Webserver instance
    const hpd_module_t *context;
};

/**
 * Callback for tcpd library.
 *
 *  Handles new connections, by creating a request for them.
 *
 *  \param  instance  Webserver instance
 *  \param  conn      Connection
 *  \param  ws_ctx    The http webserver context
 *  \param  conn_ctx  Can be set to a connection context
 *
 *  \return 0 on success, 1 on error
 */
static hpd_tcpd_return_t httpd_on_connect(hpd_tcpd_t *instance, hpd_tcpd_conn_t *conn, void *ws_ctx, void **conn_ctx)
{
    hpd_error_t rc;
    hpd_httpd_t *httpd = ws_ctx;

    if ((rc = http_request_create((hpd_httpd_request_t **) conn_ctx, httpd, &httpd->settings, conn, httpd->context))) {
        HPD_LOG_ERROR(httpd->context, "Failed to create request (code: %d).", rc);
        return HPD_TCPD_R_STOP;
    }

    return HPD_TCPD_R_CONTINUE;
}

/**
 * Callback for tcpd library.
 *
 *  Handles reception of data, by supplying it to the http_parser
 *  associated with the request.
 *
 *  \param  instance  Webserver instance
 *  \param  conn      Connection
 *  \param  ws_ctx    The http webserver instance
 *  \param  conn_ctx  The connection context, if set in previous callbacks
 *  \param  buf       Data received.
 *  \param  len       Length of data received.
 *
 *  \return 0 on success, 1 on error
 */
static hpd_tcpd_return_t httpd_on_receive(hpd_tcpd_t *instance, hpd_tcpd_conn_t *conn, void *ws_ctx, void **conn_ctx, const char *buf, size_t len)
{
    hpd_error_t rc;
    hpd_httpd_t *httpd = ws_ctx;

    if ((rc = http_request_parse(*conn_ctx, buf, len))) {
        HPD_LOG_ERROR(httpd->context, "Failed to parse request (code: %d).", rc);
        return HPD_TCPD_R_STOP;
    }
    return HPD_TCPD_R_CONTINUE;
}

/**
 * Callback for tcpd library.
 *
 *  Handles closure of connections, by creating a destroying the
 *  request.
 *
 *  \param  instance  Webserver instance
 *  \param  conn      Connection
 *  \param  ws_ctx    The http webserver instance
 *  \param  conn_ctx  The connection context, if set in previous callbacks
 *
 *  \return 0 on success, 1 on error
 */
static hpd_tcpd_return_t httpd_on_disconnect(hpd_tcpd_t *instance, hpd_tcpd_conn_t *conn, void *ws_ctx, void **conn_ctx)
{
    hpd_error_t rc;
    hpd_httpd_t *httpd = ws_ctx;

    if ((rc = http_request_destroy(*conn_ctx))) {
        HPD_LOG_ERROR(httpd->context, "Failed to destroy request (code: %d).", rc);
        return HPD_TCPD_R_STOP;
    }
    *conn_ctx = NULL;

    return HPD_TCPD_R_CONTINUE;
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
 *  \param  httpd     The newly created instance will be stored here.
 *  \param  settings  The settings for the httpd.
 *  \param  context   HPD module context.
 *  \param  loop      The event loop to start the server on.
 */
hpd_error_t hpd_httpd_create(hpd_httpd_t **httpd, hpd_httpd_settings_t *settings, const hpd_module_t *context,
                             hpd_ev_loop_t *loop)
{
    if (!context) return HPD_E_NULL;
    if (!httpd || !settings || !loop) HPD_LOG_RETURN_E_NULL(context);

    // Allocate instance
    (*httpd) = malloc(sizeof(hpd_httpd_t));
    if (!(*httpd)) HPD_LOG_RETURN_E_ALLOC(context);

    // Set context
    (*httpd)->context = context;

    // Copy settings
    memcpy(&(*httpd)->settings, settings, sizeof(hpd_httpd_settings_t));

    // Construct settings for tcpd
    hpd_tcpd_settings_t ws_settings = HPD_TCPD_SETTINGS_DEFAULT;
    ws_settings.port          = settings->port;
    ws_settings.timeout       = settings->timeout;
    ws_settings.on_connect    = httpd_on_connect;
    ws_settings.on_receive    = httpd_on_receive;
    ws_settings.on_disconnect = httpd_on_disconnect;
    ws_settings.tcpd_ctx      = (*httpd);

    // Create tcpd
    hpd_error_t rc;
    if ((rc = hpd_tcpd_create(&(*httpd)->webserver, &ws_settings, context, loop))) {
        hpd_httpd_destroy(*httpd);
        return rc;
    }

    return HPD_E_SUCCESS;
}

/**
 * Destroy a httpd instance.
 *
 *  Destroy a httpd instance created with hpd_httpd_create(). If
 *  the server is started it should be stopped first by hpd_httpd_stop().
 *
 *  \param  httpd  The httpd instance to destroy.
 */
hpd_error_t hpd_httpd_destroy(hpd_httpd_t *httpd)
{
    if (!httpd) return HPD_E_NULL;

    hpd_error_t rc = hpd_tcpd_destroy(httpd->webserver);
    free(httpd);
    return rc;
}

/**
 * Start a httpd instance.
 *
 *  Starts a created httpd.
 *
 *  \param  httpd  The instance to start.
 *
 *  \return The error code of hpd_tcpd_start()
 */
hpd_error_t hpd_httpd_start(hpd_httpd_t *httpd)
{
    if (!httpd) return HPD_E_NULL;

    return hpd_tcpd_start(httpd->webserver);
}

/**
 * Stop a httpd instance.
 *
 *  Stops an already started httpd instance. All open connections will
 *  be killed without notifying clients, nor sending remaining data.
 *
 *  \param  httpd  Instance to stop.
 */
hpd_error_t hpd_httpd_stop(hpd_httpd_t *httpd)
{
    if (!httpd) return HPD_E_NULL;

    return hpd_tcpd_stop(httpd->webserver);
}

