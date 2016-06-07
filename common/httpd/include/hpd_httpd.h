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

#ifndef HOMEPORT_HPD_HTTPD_H
#define HOMEPORT_HPD_HTTPD_H

#include "hpd_map.h"
#include <stddef.h>
#include <stdarg.h>
#include "hpd_types.h"
#include "hpd_httpd_types.h"

// Structs
typedef struct hpd_httpd hpd_httpd_t;
typedef struct hpd_httpd_request hpd_httpd_request_t;
typedef struct hpd_httpd_response hpd_httpd_response_t;
typedef struct hpd_httpd_settings hpd_httpd_settings_t;

typedef enum hpd_httpd_return {
    HPD_HTTPD_R_CONTINUE = 0,
    HPD_HTTPD_R_STOP = 1,
} hpd_httpd_return_t;

/**********************************************************************
 *  Callbacks                                                         *
 **********************************************************************/

typedef hpd_httpd_return_t (*hpd_httpd_data_f)(hpd_httpd_t *httpd, hpd_httpd_request_t *req, void* httpd_ctx, void** req_data, const char *buf, size_t len);
typedef hpd_httpd_return_t (*hpd_httpd_nodata_f)(hpd_httpd_t *httpd, hpd_httpd_request_t *req, void* httpd_ctx, void** req_data);

/**
 * Settings struct for webserver.
 *
 *  Please initialise this struct as following, to ensure that all
 *  settings have acceptable default values:
 *  \code
 *  hpd_httpd_settings_t *settings = HPD_HTTPD_SETTINGS_DEFAULT;
 *  \endcode
 *
 *  The settings hold a series of callbacks of type either data_cb or
 *  nodata_cb. Do not expect the string parameter in data callbacks to
 *  be null terminated. All data callbacks, except on_req_method, are
 *  called on chunks and therefore may be called multiple times.  That
 *  is on_req_hdr_field may be called first with 'hos' and then with
 *  't'. It is up to the implementer of these callbacks to concatenate
 *  the results if needed.
 *
 *  The callbacks are called in the following order (on_req_destroy may be
 *  called at any time):
 *  \dot
 *  digraph callback_order {
 *  on_req_begin -> on_req_url;
 *  on_req_url -> on_req_url;
 *  on_req_url -> on_req_url_cmpl;
 *  on_req_url_cmpl -> on_req_hdr_field;
 *  on_req_url_cmpl -> on_req_hdr_cmpl;
 *  on_req_hdr_field -> on_req_hdr_field;
 *  on_req_hdr_field -> on_req_hdr_value;
 *  on_req_hdr_value -> on_req_hdr_value;
 *  on_req_hdr_value -> on_req_hdr_field;
 *  on_req_hdr_value -> on_req_hdr_cmpl;
 *  on_req_hdr_cmpl -> on_req_body;
 *  on_req_hdr_cmpl -> on_req_cmpl;
 *  on_req_body -> on_req_body;
 *  on_req_body -> on_req_cmpl;
 *  }
 *  \enddot
 *
 *  Return values should generally be interpreted as follows:
 *  - zero: Continue parsing of message.
 *  - non-zero: Stop any further parsing of message.
 *  Note that the connection is kept open in both cases, thus a response can be
 *  sent to the client without parsing it entirely. The return value of
 *  on_req_destroy is ignored.
 */
struct hpd_httpd_settings {
    hpd_tcpd_port_t port;
    int timeout;
    void* httpd_ctx;
    hpd_httpd_nodata_f on_req_begin;
    hpd_httpd_data_f   on_req_url;
    hpd_httpd_nodata_f on_req_url_cmpl;
    hpd_httpd_data_f   on_req_hdr_field;
    hpd_httpd_data_f   on_req_hdr_value;
    hpd_httpd_nodata_f on_req_hdr_cmpl;
    hpd_httpd_data_f   on_req_body;
    hpd_httpd_nodata_f on_req_cmpl;
    hpd_httpd_nodata_f on_req_destroy;
};

/**
 * Default settings for httpd.
 *
 *  Use this as:
 *  \code
 *  hpd_httpd_settings_t *settings = HPD_HTTPD_SETTINGS_DEFAULT;
 *  \endcode
 */
#define HPD_HTTPD_SETTINGS_DEFAULT { \
   .port = HPD_TCPD_P_HTTP, \
   .timeout = 15, \
   .httpd_ctx = NULL, \
   .on_req_begin = NULL, \
   .on_req_url = NULL, \
   .on_req_url_cmpl = NULL, \
   .on_req_hdr_field = NULL, \
   .on_req_hdr_value = NULL, \
   .on_req_hdr_cmpl = NULL, \
   .on_req_body = NULL, \
   .on_req_destroy = NULL, \
   .on_req_cmpl = NULL }

// Webserver functions
hpd_error_t hpd_httpd_create(hpd_httpd_t **httpd, hpd_httpd_settings_t *settings, const hpd_module_t *context,
                             hpd_ev_loop_t *loop);
hpd_error_t hpd_httpd_destroy(hpd_httpd_t *httpd);
hpd_error_t hpd_httpd_start(hpd_httpd_t *httpd);
hpd_error_t hpd_httpd_stop(hpd_httpd_t *httpd);

// Request functions
hpd_error_t hpd_httpd_request_get_method(hpd_httpd_request_t *req, hpd_httpd_method_t *method);
hpd_error_t hpd_httpd_request_get_url(hpd_httpd_request_t *req, const char **url);
hpd_error_t hpd_httpd_request_get_headers(hpd_httpd_request_t *req, hpd_map_t **headers);
hpd_error_t hpd_httpd_request_get_header(hpd_httpd_request_t *req, const char *key, const char **value);
hpd_error_t hpd_httpd_request_get_arguments(hpd_httpd_request_t *req, hpd_map_t **arguments);
hpd_error_t hpd_httpd_request_get_argument(hpd_httpd_request_t *req, const char *key, const char **val);
hpd_error_t hpd_httpd_request_get_cookies(hpd_httpd_request_t *req, hpd_map_t **cookies);
hpd_error_t hpd_httpd_request_get_cookie(hpd_httpd_request_t *req, const char *key, const char **val);
hpd_error_t hpd_httpd_request_get_ip(hpd_httpd_request_t *req, const char **ip);
hpd_error_t hpd_httpd_request_keep_open(hpd_httpd_request_t *req);

// Response functions
hpd_error_t hpd_httpd_response_destroy(hpd_httpd_response_t *res);
hpd_error_t hpd_httpd_response_create(hpd_httpd_response_t **response, hpd_httpd_request_t *req,
                                      hpd_status_t status);
hpd_error_t hpd_httpd_response_add_header(hpd_httpd_response_t *res, const char *field, const char *value);
hpd_error_t hpd_httpd_response_sendf(hpd_httpd_response_t *res, const char *fmt, ...);
hpd_error_t hpd_httpd_response_vsendf(hpd_httpd_response_t *res, const char *fmt, va_list arg);
hpd_error_t hpd_httpd_response_add_cookie(hpd_httpd_response_t *res, const char *field, const char *value,
                                          const char *expires, const char *max_age, const char *domain,
                                          const char *path,
                                          int secure, int http_only, const char *extension);

#endif // HOMEPORT_HTTPD_H
