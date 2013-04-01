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
 *  The webserver is a single threaded event based webserver, built on
 *  top of the event loop implemented in LibEV.
 *  
 *  The overall webserver consists of two sub-modules:
 *
 *  \dot
 *  graph example {
 *     node [shape=record, fontname=Helvetica, fontsize=10];
 *     webserver [ label="Webserver" URL="\ref webserver.h"];
 *     client [ label="Client" URL="\ref client.h"];
 *     webserver -- accept;
 *     webserver -- client;
 *  }
 *  \enddot
 *
 *  The \ref webserver.h "Webserver submodule" is the interface for
 *  anyone that needs to development using this webserver
 *  implementation.  Most users only needs to know the implementation
 *  within this submodule. The webser submodule also handles the
 *  acceptance of new clients.
 *
 *  The \ref client.h "Client submodule" handles the communication with
 *  a single client connected to the webserver. It takes over as the
 *  accepter of any connection made to the webserver submodule. 
 *
 *  \{
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <stddef.h>

enum ws_log_level {
   WS_LOG_FATAL,
   WS_LOG_ERROR,
   WS_LOG_WARN,
   WS_LOG_INFO,
   WS_LOG_DEBUG
};

// HTTP status codes according to
// http://www.w3.org/Protocols/rfc2616/rfc2616.html
#define WS_HTTP_STATUS_CODE_MAP(XX) \
	XX(200,200 OK) \
	XX(404,404 Not Found)

enum ws_http_status_code
{
#define XX(num, str) WS_HTTP_##num = num,
	WS_HTTP_STATUS_CODE_MAP(XX)
#undef XX
};

// Port numbers are assigned according to
// http://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xml
// (Site may have very long loading time)
#define WS_PORT_MAP(XX) \
   XX(  80, HTTP    ) \
   XX( 443, HTTPS   ) \
   XX(8080, HTTP_ALT)

enum ws_port
{
#define XX(num, str) WS_PORT_##str = num,
   WS_PORT_MAP(XX)
#undef XX
};

// Structs
struct ev_loop;
struct ws_instance;
struct ws_request;

// Callbacks
typedef struct ws_response *(*request_cb)(struct ws_request *req);

// Settings struct
struct ws_settings {
   unsigned short int port;
   size_t max_request_size;
   request_cb on_request_begin;
   request_cb on_request_url;
   request_cb on_request_status;
   request_cb on_request_header;
   request_cb on_request_header_complete;
   request_cb on_request_body;
   request_cb on_request_complete;
};
#define WS_SETTINGS_DEFAULT { \
   .port = WS_PORT_HTTP, \
   .max_request_size = 1024*1024, \
   .on_request_begin = NULL, \
   .on_request_url = NULL, \
   .on_request_status = NULL, \
   .on_request_header = NULL, \
   .on_request_header_complete = NULL, \
   .on_request_body = NULL, \
   .on_request_complete = NULL }

// Webserver instance functions
struct ws_instance *ws_instance_create(
      struct ws_settings *settings,
      struct ev_loop *loop);
void ws_instance_free(struct ws_instance *instance);
void ws_start(struct ws_instance *instance);
void ws_stop(struct ws_instance *instance);

// Webserver request functions
char *ws_request_get_url(struct ws_request *req);
const char *ws_request_get_method_str(struct ws_request *req);
char *ws_request_get_body(struct ws_request *req);

// Webserver response functions
struct ws_response *ws_response_create(
      struct ws_request *req,
      enum ws_http_status_code status,
      char *body);

#endif

/** } */
