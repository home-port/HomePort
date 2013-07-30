// http-webserver.h

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

#ifndef HTTP_WEBSERVER_H
#define HTTP_WEBSERVER_H

#include "http_types.h"
#include "linkedmap.h"
#include <stddef.h>
#include <stdarg.h>

// TODO: Move timeouts here from the webserver

struct ev_loop;
struct httpws;
struct http_request;
struct http_response;

typedef int (*httpws_data_cb)(
      struct httpws *ins, struct http_request *req,
      void* ws_ctx, void** req_data,
      const char *buf, size_t len);
typedef int (*httpws_nodata_cb)(
      struct httpws *ins, struct http_request *req,
      void* ws_ctx, void** req_data);

struct httpws_settings {
   enum ws_port port;
   int timeout;
   void* ws_ctx;
   httpws_nodata_cb on_req_begin;
   httpws_data_cb   on_req_method;
   httpws_data_cb   on_req_url;
   httpws_nodata_cb on_req_url_cmpl;
   httpws_data_cb   on_req_hdr_field;
   httpws_data_cb   on_req_hdr_value;
   httpws_nodata_cb on_req_hdr_cmpl;
   httpws_data_cb   on_req_body;
   httpws_nodata_cb on_req_cmpl;
   httpws_nodata_cb on_req_destroy;
};
#define HTTPWS_SETTINGS_DEFAULT { \
   .port = WS_PORT_HTTP, \
   .timeout = 15, \
   .ws_ctx = NULL, \
   .on_req_begin = NULL, \
   .on_req_method = NULL, \
   .on_req_url = NULL, \
   .on_req_url_cmpl = NULL, \
   .on_req_hdr_field = NULL, \
   .on_req_hdr_value = NULL, \
   .on_req_hdr_cmpl = NULL, \
   .on_req_body = NULL, \
   .on_req_destroy = NULL, \
   .on_req_cmpl = NULL }

// Webserver functions
struct httpws *  httpws_create  (struct httpws_settings *settings,
                                 struct ev_loop *loop);
void             httpws_destroy (struct httpws *instance);
int              httpws_start   (struct httpws *instance);
void             httpws_stop    (struct httpws *instance);

// Request functions
enum http_method  http_request_get_method    (struct http_request *req);
const char *      http_request_get_url       (struct http_request *req);
struct lm *       http_request_get_headers   (struct http_request *req);
const char *      http_request_get_header    (struct http_request *req,
                                              const char* key);
struct lm *       http_request_get_arguments (struct http_request *req);
const char *      http_request_get_argument  (struct http_request *req,
                                              const char* key);
struct lm *       http_request_get_cookies   (struct http_request *req);
const char *      http_request_get_cookie    (struct http_request *req,
                                              const char* key);
const char *      http_request_get_ip        (struct http_request *req);
void              http_request_keep_open     (struct http_request *req);

// Response functions
void  http_response_destroy    (struct http_response *res);
struct http_response *
      http_response_create     (struct http_request *req,
                                enum httpws_http_status_code status);
int   http_response_add_header (struct http_response *res,
                                const char *field, const char *value);
void  http_response_sendf      (struct http_response *res,
                                const char *fmt, ...);
void  http_response_vsendf     (struct http_response *res,
                                const char *fmt, va_list arg);
int   http_response_add_cookie (struct http_response *res,
                                const char *field, const char *value,
                                const char *expires, const char *max_age,
                                const char *domain, const char *path,
                                int secure, int http_only,
                                const char *extension);


#endif
