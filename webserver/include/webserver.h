// webserver.h

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

/** \defgroup webserver Webserver
 *
 *  The webserver is a single threaded event based webserver, built on
 *  top of the event loop implemented in LibEV.
 *
 *  \{
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <stddef.h>
#include "ws_types.h"

// Structs
struct ev_loop;
struct ws;
struct ws_client;

/**********************************************************************
 *  Callbacks                                                         *
 **********************************************************************/

typedef int (*ws_client_cb)(struct ws_client *client);
typedef int (*ws_data_cb)(struct ws_client *client, void *data);
typedef int (*ws_chunk_cb)(struct ws_client *client, void *data,
                           const char *buf, size_t len);

/// Settings struct for webserver
/**
 *  Please initialise this struct as following, to ensure that all
 *  settings have acceptable default values:
 *  \code
 *  struct ws_settings *settings = WS_SETTINGS_DEFAULT;
 *  \endcode
 *
 *  The settings hold a series of callbacks of type either data_cb or
 *  nodata_cb. Do not expect the string parameter in data callbacks to
 *  be null terminated (only on_request_method may be). All data
 *  callbacks, except on_request_method, are called on chunks and
 *  therefore may be called multiple times. That is
 *  on_request_header_field may be called first with 'hos' and then with
 *  't'. It is up to the implementer of these callbacks to concatenate
 *  the results if needed.
 *
 *  The callbacks are called in the following order:
 *  \dot
 *  digraph callback_order {
 *  on_request_begin -> on_request_method;
 *  on_request_method -> on_request_url;
 *  on_request_url -> on_request_url;
 *  on_request_url -> on_request_url_complete;
 *  on_request_url_complete -> on_request_header_field;
 *  on_request_url_complete -> on_request_header_complete;
 *  on_request_header_field -> on_request_header_field;
 *  on_request_header_field -> on_request_header_value;
 *  on_request_header_value -> on_request_header_value;
 *  on_request_header_value -> on_request_header_field;
 *  on_request_header_value -> on_request_header_complete;
 *  on_request_header_complete -> on_request_body;
 *  on_request_header_complete -> on_request_complete;
 *  on_request_body -> on_request_body;
 *  on_request_body -> on_request_complete;
 *  }
 *  \enddot
 */
struct ws_settings {
   enum ws_port port; ///< Port number
   ws_client_cb on_connect;
   ws_chunk_cb on_receive;
};

/// Default settings for webserver
/**
 *  Use this as:
 *  \code
 *  struct ws_settings *settings = WS_SETTINGS_DEFAULT;
 *  \endcode
 */
#define WS_SETTINGS_DEFAULT { \
   .port = WS_PORT_HTTP, \
   .on_connect = NULL, \
   .on_receive = NULL }

// Webserver functions
struct ws *ws_create(struct ws_settings *settings, struct ev_loop *loop);
void ws_destroy(struct ws *instance);
int ws_start(struct ws *instance);
void ws_stop(struct ws *instance);

// Client functions
void ws_client_set_data(struct ws_client *client, void *data);
void ws_client_kill(struct ws_client *client);
void ws_client_sendf(struct ws_client *client, char *fmt, ...);

#endif

/** } */
