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

#include "httpd_request.h"
#include "hpd_shared_api.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define HTTP_VERSION "HTTP/1.1 "
#define CRLF "\r\n"

/**
 * A http response.
 *
 *  A response should be created to an already existing request with
 *  hpd_httpd_response_create(), sent with http_response_sendf() or
 *  hpd_httpd_response_vsendf(), and freed with hpd_httpd_response_destroy().
 *
 *  Headers are added with hpd_httpd_response_add_header(), if the header is
 *  a cookie it can also be added with hpd_httpd_response_add_cookie().
 *
 *  The body is sent in chunks by repeating the calls to
 *  hpd_httpd_response_sendf() and http_response_vsentf(). The status and
 *  headers will be sent on the first call.
 */
struct hpd_httpd_response
{
    const hpd_module_t *context;
    hpd_tcpd_conn_t *conn; ///< The connection to send on
    char *msg;            ///< Status/headers to send
    hpd_status_t status;
};

/**
 * Convert a status code to string.
 *
 *  The result is constructed to match the textual status code in the
 *  first line in a http response, according to RFC 2616.
 *
 *  \param Status code as enum
 *
 *  \return The status code as text
 */
static char* http_status_codes_to_str(hpd_status_t status)
{
#define XX(num, str) if(status == num) {return #str;}
    HPD_HTTP_STATUS_CODE_MAP(XX)
#undef XX
    return NULL;
}

/**
 * Destroy a hpd_httpd_response.
 *
 *  The will close the connection and free up any memory used by the
 *  response.
 *
 *  Any data sent with hpd_httpd_response_sendf() and http_reponse_vsendf()
 *  will be sent before the connection is closed.
 *
 *  \param  res  The HTTP Response to destroy
 */
hpd_error_t hpd_httpd_response_destroy(hpd_httpd_response_t *res)
{
    if (!res) return  HPD_E_NULL;

    hpd_error_t rc = hpd_tcpd_conn_close(res->conn);
    free(res->msg);
    free(res);
    return rc;
}

/**
 *  Create the reponse and constructs the status line.
 *
 *  Currently it also adds the header "Connection: close", because it do
 *  not yet support persistant connections. That is, connection where
 *  status and headers are sent multiple times.
 *
 *  The response is not send before one of the send functions are
 *  called, it is possible to call these with a NULL body to send
 *  messages without it.
 *
 *  \param  req     The http request to repond to
 *  \param  status  The status code to respond with
 *
 *  \return  The http respond created
 */
hpd_error_t hpd_httpd_response_create(hpd_httpd_response_t **response, hpd_httpd_request_t *req, hpd_status_t status)
{
    if (!req) return HPD_E_NULL;
    hpd_error_t rc, rc2;
    const hpd_module_t *context;
    if ((rc = http_request_get_context(req, &context))) return rc;
    if (!response) HPD_LOG_RETURN_E_NULL(context);

    int len;

    // Get data
    char *status_str = http_status_codes_to_str(status);

    // Calculate msg length
    len = 1;
    len += strlen(HTTP_VERSION);
    // TODO Problem if this if is NULL !
    if (status_str) len += strlen(status_str);
    len += 4;
    len += strlen(CRLF);

    // Allocate space
    (*response) = malloc(sizeof(hpd_httpd_response_t));
    if (!(*response)) HPD_LOG_RETURN_E_ALLOC(context);
    (*response)->status = status;
    (*response)->msg = malloc(len*sizeof(char));
    if (!(*response)->msg) {
        if ((rc = hpd_httpd_response_destroy((*response))))
            HPD_LOG_ERROR(context, "Failed to destroy response (code: %d).", rc);
        HPD_LOG_RETURN_E_ALLOC(context);
    }

    // Init struct
    (*response)->context = context;
    if ((rc = http_request_get_connection(req, &(*response)->conn))) return rc;

    // Construct msg
    strcpy((*response)->msg, HTTP_VERSION);
    sprintf(&(*response)->msg[strlen(HTTP_VERSION)], "%i ", status);
    if (status_str) strcat((*response)->msg, status_str);
    strcat((*response)->msg, CRLF);

    // Real persistent connections is not supported, so tell client that we close connection after response has been sent
    rc = hpd_httpd_response_add_header((*response), "Connection", "close");
    if (rc && (rc2 = hpd_httpd_response_destroy(*response)))
        HPD_LOG_ERROR(context, "Failed to destroy response (code: %d).", rc2);;
    return rc;
}

/**
 * Add header to a response.
 *
 *  This returns an error if any of the send functions has already been
 *  called.
 *
 *  The field/value pair is stored internally in the response.
 *
 *  \param  res    The response to add headers to
 *  \param  field  The field of the header
 *  \param  value  The value of the header
 *
 *  \return 0 on success and 1 on failure
 */
hpd_error_t hpd_httpd_response_add_header(hpd_httpd_response_t *res, const char *field, const char *value)
{
    if (!res) return HPD_E_NULL;
    if (!field || !value) HPD_LOG_RETURN_E_NULL(res->context);

    // Headers already sent
    if (!res->msg)
        HPD_LOG_RETURN(res->context, HPD_E_STATE, "Cannot add header, they have already been sent to client.");

    char *msg;
    size_t msg_len = strlen(res->msg)+strlen(field)+2+strlen(value)+strlen(CRLF)+1;

    msg = realloc(res->msg, msg_len*sizeof(char));
    if (!msg) HPD_LOG_RETURN_E_ALLOC(res->context);
    res->msg = msg;

    strcat(res->msg, field);
    strcat(res->msg, ": ");
    strcat(res->msg, value);
    strcat(res->msg, CRLF);

    return HPD_E_SUCCESS;
}

/**
 *  Add cookie header to response.
 *
 *  Works similarily to hpd_httpd_response_add_header(), but constructs a
 *  cookie header based on the details in RFC 6265. Refer to this for
 *  the correct syntax of the parameters.
 *
 *  \param  res        Http reponse to add cookie to
 *  \param  field      The field of the cookie
 *  \param  value      The value of the cookie
 *  \param  expires    The expiring time as string or NULL to avoid
 *  \param  max_age    The maximum age as string or NULL to avoid
 *  \param  domain     The domain as string or NULL to avoid
 *  \param  path       The path as string or NULL to avoid
 *  \param  secure     0 to avoid, any other to add the keyword
 *  \param  http_only  0 to avoid, any other to add the keyword
 *  \param  extension  The extension as string or NULL to avoid
 *
 *  \return 0 on success and 1 otherwise
 */
hpd_error_t hpd_httpd_response_add_cookie(hpd_httpd_response_t *res,
                                          const char *field, const char *value,
                                          const char *expires, const char *max_age,
                                          const char *domain, const char *path,
                                          int secure, int http_only,
                                          const char *extension)
{
    if (!res) return HPD_E_NULL;
    if (!field || !value) HPD_LOG_RETURN_E_NULL(res->context);

    // Headers already sent
    if (!res->msg) return HPD_E_STATE;

    char *msg;
    size_t msg_len = strlen(res->msg) + 12 + strlen(field) + 1 + strlen(value) + strlen(CRLF) + 1;

    // Calculate length
    if (expires)   msg_len += 10 + strlen(expires);
    if (max_age)   msg_len += 10 + strlen(max_age);
    if (domain)    msg_len +=  9 + strlen(domain);
    if (path)      msg_len +=  7 + strlen(path);
    if (secure)    msg_len +=  8;
    if (http_only) msg_len += 10;
    if (extension) msg_len +=  2 + strlen(extension);

    // Reallocate message
    msg = realloc(res->msg, msg_len*sizeof(char));
    if (!msg) HPD_LOG_RETURN_E_ALLOC(res->context);
    res->msg = msg;

    // Apply header
    strcat(res->msg, "Set-Cookie: ");
    strcat(res->msg, field);
    strcat(res->msg, "=");
    strcat(res->msg, value);
    strcat(res->msg, CRLF);
    if (expires) {
        strcat(res->msg, "; Expires=");
        strcat(res->msg, expires);
    }
    if (max_age) {
        strcat(res->msg, "; Max-Age=");
        strcat(res->msg, max_age);
    }
    if (domain) {
        strcat(res->msg, "; Domain=");
        strcat(res->msg, domain);
    }
    if (path) {
        strcat(res->msg, "; Domain=");
        strcat(res->msg, domain);
    }
    if (secure) strcat(res->msg, "; Secure");
    if (http_only) strcat(res->msg, "; HttpOnly");
    if (extension) {
        strcat(res->msg, "; ");
        strcat(res->msg, extension);
    }

    return HPD_E_SUCCESS;
}

/**
 * Send response to client.
 *
 *  Similar to the standard printf function. See hpd_httpd_response_vsendf()
 *  for details.
 *
 *  \param  res  The respond to sent
 *  \param  fmt  The format string for the body
 */
hpd_error_t hpd_httpd_response_sendf(hpd_httpd_response_t *res, const char *fmt, ...)
{
    if (!res) return HPD_E_NULL;

    va_list arg;
    va_start(arg, fmt);
    hpd_error_t rc = hpd_httpd_response_vsendf(res, fmt, arg);
    va_end(arg);
    return rc;
}

/**
 * Send response to client.
 *
 *  Similar to the standard vprintf functions
 *
 *  First it sends the status and header lines, if these haven't been
 *  sent yet. Then it sends the body as given in the format string and
 *  variable arguments.
 *
 *  If NULL is given as format no body is sent.
 *
 *  The response is sent delayed, when the connection is ready for it. Likewise
 *  the connection is kept open afterwards for further messages.
 *
 *  \param  res  The http response to send.
 *  \param  fmt  The format string for the body
 */
hpd_error_t hpd_httpd_response_vsendf(hpd_httpd_response_t *res, const char *fmt, va_list arg)
{
    if (!res) return HPD_E_NULL;

    hpd_error_t rc;

    if (res->msg) {
        const char *ip;
        if ((rc = hpd_tcpd_conn_get_ip(res->conn, &ip))) {
            HPD_LOG_WARN(res->context, "Failed to get ip [code: %i].", rc);
            ip = "(unknown)";
        }
        HPD_LOG_VERBOSE(res->context, "Sending response to %s: %i %s.", ip, res->status, http_status_codes_to_str(res->status));
        if ((rc = hpd_tcpd_conn_sendf(res->conn, "%s%s", res->msg, CRLF))) return rc;
        free(res->msg);
        res->msg = NULL;
    }

    if (fmt) {
        return hpd_tcpd_conn_vsendf(res->conn, fmt, arg);
    }

    return HPD_E_SUCCESS;
}

