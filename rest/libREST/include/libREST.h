// libREST.h

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

#ifndef LIBREST_H
#define LIBREST_H

#include "http_types.h"
#include <stddef.h>
#include <stdarg.h>
#include "hpd_map.h"

// Structs
struct lr;
struct lr_request;
struct ev_loop;

struct lr_settings {
	int port;
	int timeout;
};
#define LR_SETTINGS_DEFAULT { \
	.port = WS_PORT_HTTP, \
	.timeout = 15 }

// Callbacks
typedef int (*lr_data_cb)(void *srv_data, void **req_data,
                          struct lr_request *req,
                          const char* body, size_t len);
typedef int (*lr_nodata_cb)(void *srv_data, void **req_data,
                            struct lr_request *req);

// libREST instance functions
struct lr *lr_create(struct lr_settings *settings, struct ev_loop *loop);
void lr_destroy(struct lr *ins);

int lr_start(struct lr *ins);
void lr_stop(struct lr *ins);

// Registers a new service. Returns 1 if the url is already registered
// or not enough memory
int lr_register_service(struct lr *ins,
                         char *url,
                         lr_data_cb on_get,
                         lr_data_cb on_post,
                         lr_data_cb on_put,
                         lr_data_cb on_delete,
                         lr_nodata_cb on_destroy,
                         void *srv_data);

// Unregister service. Returns the data stored in the service
void *lr_unregister_service(struct lr *ins, const char *url);

void *lr_lookup_service(struct lr *ins, char *url);

// Request functions
enum http_method lr_request_get_method(struct lr_request *req);
const char *lr_request_get_url(struct lr_request *req);
map_t *lr_request_get_headers(struct lr_request *req);
const char *lr_request_get_header(struct lr_request *req, const char* key);
map_t *lr_request_get_arguments(struct lr_request *req);
const char *lr_request_get_argument(struct lr_request *req, const char* key);
map_t *lr_request_get_cookies(struct lr_request *req);
const char *lr_request_get_cookie(struct lr_request *req, const char* key);
const char *lr_request_get_ip(struct lr_request *req);
void lr_request_keep_open(struct lr_request *req);
void lr_request_print(struct lr_request *req);

// Send response functions
void lr_sendf(struct lr_request *req,
              enum httpws_http_status_code status,
              map_t *headers, const char *fmt, ...);
void lr_send_start(struct lr_request *req,
				   enum httpws_http_status_code status,
				   map_t *headers);
int lr_send_add_cookie_simple(struct lr_request *req,
                              const char *field, const char *value);
int lr_send_add_cookie(struct lr_request *req,
                       const char *field, const char *value,
                       const char *expires, const char *max_age,
                       const char *domain, const char *path,
                       int secure, int http_only,
                       const char *extension);
void lr_send_chunkf(struct lr_request *req, const char *fmt, ...);
void lr_send_vchunkf(struct lr_request *req, const char *fmt, va_list arg);
void lr_send_stop(struct lr_request *req);

#endif
