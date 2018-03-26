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

#ifndef HOMEPORT_HTTPD_REQUEST_H
#define HOMEPORT_HTTPD_REQUEST_H

#include "hpd-0.6/common/hpd_httpd.h"
#include "hpd-0.6/common/hpd_tcpd.h"
#include <stddef.h>

hpd_error_t http_request_create(hpd_httpd_request_t **req, hpd_httpd_t *httpd, hpd_httpd_settings_t *settings,
                                hpd_tcpd_conn_t *conn, const hpd_module_t *context);
hpd_error_t http_request_destroy(hpd_httpd_request_t *req);
hpd_error_t http_request_parse(hpd_httpd_request_t *req, const char *buf, size_t len);
hpd_error_t http_request_get_connection(hpd_httpd_request_t *req, hpd_tcpd_conn_t **conn);
hpd_error_t http_request_get_context(hpd_httpd_request_t *req, const hpd_module_t **context);

#endif
