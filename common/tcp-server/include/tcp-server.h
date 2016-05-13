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

#ifndef HOMEPORT_TCP_SERVER_H
#define HOMEPORT_TCP_SERVER_H

#include <stddef.h>
#include <stdarg.h>
#include "hpd_types.h"

// Structs
typedef struct hpd_tcpd hpd_tcpd_t;
typedef struct hpd_tcpd_settings hpd_tcpd_settings_t;
typedef struct hpd_tcpd_conn hpd_tcpd_conn_t;

/**********************************************************************
 *  Callbacks                                                         *
 **********************************************************************/

typedef hpd_error_t (*hpd_tcpd_nodata_f)(hpd_tcpd_t *instance, hpd_tcpd_conn_t *conn, void *ws_ctx, void **conn_ctx);
typedef hpd_error_t (*hpd_tcpd_data_f)(hpd_tcpd_t *instance, hpd_tcpd_conn_t *conn, void *ws_ctx, void **conn_ctx, const char *buf, size_t len);

/**
 * Settings struct for tcp-server.
 *
 *  Please initialise this struct as following, to ensure that all
 *  settings have acceptable default values:
 *  \code
 *  hpd_tcpd_settings_t *settings = HPD_TCPD_SETTINGS_DEFAULT;
 *  \endcode
 *
 *  The settings hold a series of callbacks of type either data_cb or
 *  nodata_cb. Do not expect the string parameter in data callbacks to
 *  be null terminated. All data callbacks are called on chunks and
 *  therefore may be called multiple times. It is up to the implementer
 *  of these callbacks to concatenate the results if needed.
 *
 *  The callbacks are called in the following order:
 *  \dot
 *  digraph callback_order {
 *  on_connect -> on_receive;
 *  on_connect -> on_disconnect;
 *  on_receive -> on_receive;
 *  on_receive -> on_disconnect;
 *  }
 *  \enddot
 *
 *  Notes on the return values of callbacks:
 *  - on_connect:
 *    - zero: Accept client and start listening for data.
 *    - non-zero: Reject client, e.g. kill connection.
 *  - on_receive:
 *    - zero: Data accepted, continue to listen for further data.
 *    - non-zero: Data rejected, kill connection.
 *  - on_disconnect:
 *    - any: Ignored as client is to be killed anyways.
 */
struct hpd_tcpd_settings {
    hpd_port_t port; ///< Port number
    int timeout;
    int retry_delay;
    size_t max_data_size;
    hpd_tcpd_nodata_f on_connect;
    hpd_tcpd_data_f on_receive;
    hpd_tcpd_nodata_f on_disconnect;
    void *tcpd_ctx;
};

/**
 * Default settings for tcp-server.
 *
 *  Use this as:
 *  \code
 *  hpd_tcpd_settings_t settings = HPD_TCPD_SETTINGS_DEFAULT;
 *  \endcode
 */
#define HPD_TCPD_SETTINGS_DEFAULT { \
   .port = HPD_P_HTTP, \
   .timeout = 15, \
   .retry_delay = 5, \
   .max_data_size = 1024, \
   .on_connect = NULL, \
   .on_receive = NULL, \
   .on_disconnect = NULL, \
   .tcpd_ctx = NULL }

// Webserver functions
hpd_error_t hpd_tcpd_create(hpd_tcpd_t **tcpd, hpd_tcpd_settings_t *settings, hpd_module_t *context, hpd_ev_loop_t *loop);
hpd_error_t hpd_tcpd_destroy(hpd_tcpd_t *tcpd);
hpd_error_t hpd_tcpd_start(hpd_tcpd_t *tcpd);
hpd_error_t hpd_tcpd_stop(hpd_tcpd_t *tcpd);

// Client functions
hpd_error_t hpd_tcpd_conn_get_ip(hpd_tcpd_conn_t *conn, const char **ip);
hpd_error_t hpd_tcpd_conn_keep_open(hpd_tcpd_conn_t *conn);
hpd_error_t hpd_tcpd_conn_sendf(hpd_tcpd_conn_t *conn, const char *fmt, ...);
hpd_error_t hpd_tcpd_conn_vsendf(hpd_tcpd_conn_t *conn, const char *fmt, va_list vp);
hpd_error_t hpd_tcpd_conn_close(hpd_tcpd_conn_t *conn);
hpd_error_t hpd_tcpd_conn_kill(hpd_tcpd_conn_t *conn);

#endif // HOMEPORT_TCP_SERVER_H

