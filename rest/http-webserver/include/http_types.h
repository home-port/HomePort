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

#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

#include "ws_types.h"
#include "http_parser.h"

// TODO Remove these and use hpd_types.h?
// HTTP status codes according to
// http://www.w3.org/Protocols/rfc2616/rfc2616.html
#define HTTPWS_HTTP_STATUS_CODE_MAP(XX) \
   XX(200,200 OK) \
   XX(201,201 Created) \
   XX(303,303 See Other) \
   XX(400,400 Bad Request) \
   XX(404,404 Not Found) \
   XX(405,405 Method Not Allowed) \
   XX(406,406 Not Acceptable) \
   XX(408,408 Request Timeout) \
   XX(415,415 Unsupported Media Type) \
   XX(500,500 Internal Server Error) \
   XX(504,504 Gateway Timeout)

/// HTTP status codes
/**
 *  According to RFC 2616, see
 *  http://www.w3.org/Protocols/rfc2616/rfc2616.html
 */
enum httpws_http_status_code
{
#define XX(num, str) WS_HTTP_##num = num,
	HTTPWS_HTTP_STATUS_CODE_MAP(XX)
#undef XX
};

/// Convert from enum http_method to a string
const char *http_method_str(enum http_method m);

#endif
