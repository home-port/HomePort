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
#include "http_parser.h"
#include "url_parser.h"
#include "header_parser.h"
#include "hpd_shared_api.h"
#include <string.h>
#include <stdlib.h>

/// The possible states of a request
enum state {
    S_START,           ///< The initial state
    S_BEGIN,           ///< Message begun received
    S_URL,             ///< Receiving URL
    S_HEADER_FIELD,    ///< Receiving a header field
    S_HEADER_VALUE,    ///< Receiving a header value
    S_HEADER_COMPLETE, ///< Received all headers
    S_BODY,            ///< Receiving body
    S_COMPLETE,        ///< Received all of message
    S_STOP,            ///< Callback requested a stop
    S_ERROR            ///< An error has happened
};

/**
 * An http request.
 *
 * <h1>Public interface</h1>
 *
 * nodata_cb and data_cb functions in hpd_tcpd_settings provides a request as
 * their parameters. This allows the implenter of a webserver to receive
 * requests.
 *
 * ws_response_create() uses the request to create a response to the
 * client.
 *
 * <h1>Internal interface</h1>
 *
 * \warning These methods is for use only within the webserver library,
 * implementers of the webserver should use the public interface
 * described above.
 *
 * ws_request_create() Creates new requests. Requests should be
 * deallocated using ws_request_destroy() afterwards.
 *
 * Initially requests are empty, they are filled with data using
 * ws_request_parse() which appends a new string to the request. Note
 * that the request do not save any data itself, beside from its state.
 * All data in the string will be passed on as pointers to the callbacks
 * defined in hpd_tcpd_settings.
 *
 * ws_request_get_client() gets the client that sent the request.
 *
 * <h1>States</h1>
 *
 * A request will take states in the following order, the states of
 * S_STOP and S_ERROR can be assumed as always possible target for a
 * transition. The labels on the edges denote the callback from
 * hpd_tcpd_settings, that will be called upon the transition.
 * \dot
 * digraph request_states {
 * node [fontsize=10];
 * edge [fontsize=8];
 * S_START -> S_BEGIN
 * [ label = "on_requst_begin();" ];
 * S_BEGIN -> S_URL
 * [ label = "on_request_method();\non_request_url();" ];
 * S_URL -> S_URL
 * [ label = "on_request_url();" ];
 * S_URL -> S_HEADER_FIELD
 * [ label = "on_request_url_complete();\non_request_header_field();" ];
 * S_HEADER_VALUE -> S_HEADER_FIELD
 * [ label = "on_request_header_field();" ];
 * S_HEADER_FIELD -> S_HEADER_FIELD
 * [ label = "on_request_header_field();" ];
 * S_HEADER_FIELD -> S_HEADER_VALUE
 * [ label = "on_request_header_value();" ];
 * S_HEADER_VALUE -> S_HEADER_VALUE
 * [ label = "on_request_header_value();" ];
 * S_URL -> S_HEADER_COMPLETE
 * [ label = "on_request_url_complete();\non_request_header_complete();" ];
 * S_HEADER_VALUE -> S_HEADER_COMPLETE
 * [ label = "on_request_header_complete;" ];
 * S_HEADER_COMPLETE -> S_BODY
 * [ label = "on_request_body();" ];
 * S_BODY -> S_BODY
 * [ label = "on_request_body();" ];
 * S_HEADER_COMPLETE -> S_COMPLETE
 * [ label = "on_request_complete();" ];
 * S_BODY -> S_COMPLETE
 * [ label = "on_request_complete();" ];
 * }
 * \enddot
 *
 */
struct hpd_httpd_request
{
    hpd_module_t *context;
    hpd_httpd_t *webserver;         ///< HTTP Webserver
    hpd_httpd_settings_t *settings; ///< Settings
    hpd_tcpd_conn_t *conn;          ///< Connection to client
    http_parser parser;             ///< HTTP parser
    struct up *url_parser;          ///< URL Parser
    struct hp *header_parser;       ///< Header Parser
    enum state state;               ///< Current state
    char *url;                      ///< URL
    hpd_map_t *arguments;           ///< URL Arguments
    hpd_map_t *headers;             ///< Header Pairs
    hpd_map_t *cookies;             ///< Cookie Pairs
    void* data;                     ///< User data
    hpd_httpd_method_t method;
};

// Methods for http_parser settings
static int parser_msg_begin(http_parser *parser);
static int parser_url      (http_parser *parser, const char *buf, size_t len);
static int parser_hdr_field(http_parser *parser, const char *buf, size_t len);
static int parser_hdr_value(http_parser *parser, const char *buf, size_t len);
static int parser_hdr_cmpl (http_parser *parser);
static int parser_body     (http_parser *parser, const char *buf, size_t len);
static int parser_msg_cmpl (http_parser *parser);

/// Global settings for http_parser
static http_parser_settings parser_settings =
        {
                .on_message_begin = parser_msg_begin,
                .on_url = parser_url,
                .on_status = NULL,
                .on_header_field = parser_hdr_field,
                .on_header_value = parser_hdr_value,
                .on_headers_complete = parser_hdr_cmpl,
                .on_body = parser_body,
                .on_message_complete = parser_msg_cmpl
        };

/**
 * Callback for URL parser.
 *
 *  Called when the full path has been parsed and stores the full path
 *  in the url field of the request.
 *
 *  \param  data            The HTTP Request
 *  \param  parsendSegment  Full path, not null-terminated.
 *  \param  segment_length  Length of path in characters
 */
static hpd_error_t url_parser_path_complete(void *data, const char* parsedSegment, size_t segment_length)
{
    hpd_httpd_request_t *req = data;
    char *newUrl = realloc(req->url, sizeof(char)*(segment_length+1));
    if (!newUrl) HPD_LOG_RETURN_E_NULL(req->context);
    req->url = newUrl;
    strncpy(req->url, parsedSegment, segment_length);
    req->url[segment_length] = '\0';

    return HPD_E_SUCCESS;
}

/**
 * Callback for URL parser.
 *
 *  Called when the URL parser has parsed an argument pair, with a key
 *  and a value.
 *
 *  \param  data       The HTTP Request
 *  \param  key        The key, not null-terminated
 *  \param  key_len    The length of the key
 *  \param  value      The value, not null-terminated
 *  \param  value_len  The length of the value
 */
static hpd_error_t url_parser_key_value(void *data, const char *key, size_t key_len, const char *value, size_t value_len)
{
    hpd_httpd_request_t *req = data;
    return hpd_map_set_n(req->arguments, key, key_len, value, value_len);
}

/**
 * Callback for the header parser.
 *
 *  Is called for each header pairs with a field and a value. If the
 *  header pair is a cookie, the cookied will be parsed and stored in
 *  the list of cookies. Multiple headers will be combined into a
 *  a single with a comma-seperated list of values, according to the RFC
 *  2616.
 *
 *  \param  data          The HTTP Request
 *  \param  field         The field, not null-terminated
 *  \param  field_length  The length of the field
 *  \param  value         The value, not null-terminated
 *  \param  value_length  The length of the value
 */
static hpd_error_t header_parser_field_value_pair_complete(void* data, const char* field, size_t field_length, const char* value, size_t value_length)
{
    hpd_error_t rc;
    hpd_httpd_request_t *req = data;

    const char *existing;
    switch ((rc = hpd_map_get_n(req->headers, field, field_length, &existing))) {
        case HPD_E_SUCCESS:
        case HPD_E_NOT_FOUND:
            break;
        default:
            return rc;
    }

    // If cookie, then store it in cookie list
    if (strncmp(field, "Cookie", 6) == 0) {
        size_t key_s = 0, key_e, val_s, val_e;

        while (key_s < value_length) {
            for (key_e = key_s; key_e < value_length && value[key_e] != '='; key_e++);
            if (key_e == value_length) HPD_LOG_RETURN(req->context, HPD_E_ARGUMENT, "Parse error.");
            val_s = key_e + 1;
            for (val_e = val_s; val_e < value_length && value[val_e] != ';'; val_e++);
            if (key_e-key_s > 0 && val_e-val_s > 0) {
                if ((rc = hpd_map_set_n(req->cookies, &value[key_s], (size_t) key_e - key_s, &value[val_s], val_e - val_s))) return rc;
            } else {
                HPD_LOG_RETURN(req->context, HPD_E_ARGUMENT, "Parse error.");
            }
            key_s = val_e + 2;
        }
    }

    // Store header in headers list
    if (existing) {
        // Combine values
        size_t new_len = strlen(existing) + 1 + value_length + 1;
        char *new = malloc(new_len * sizeof(char));
        if (!new) HPD_LOG_RETURN_E_ALLOC(req->context);
        strcpy(new, existing);
        strcat(new, ",");
        strncat(new, value, value_length);
        new[new_len-1] = '\0';

        // Replace
        rc = hpd_map_set_n(req->headers, field, field_length, new, new_len);

        // Clean up
        free(new);

        if (rc) return rc;
    } else {
        if ((rc = hpd_map_set_n(req->headers, field, field_length, value, value_length))) return rc;
    }

    return HPD_E_SUCCESS;
}

/**
 * Message begin callback for http_parser.
 *
 *  Called when the http request message begins, by the http_parser.
 *  This will call on_request_begin() and on_request_method() from
 *  hpd_tcpd_settings.
 *
 *  @param  parser The http_parser calling.
 *
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_msg_begin(http_parser *parser)
{
    hpd_httpd_return_t stat;
    hpd_httpd_request_t *req = parser->data;
    const hpd_httpd_settings_t *settings = req->settings;

    switch (req->state) {
        case S_STOP:
            return 1;
        case S_START:
            req->state = S_BEGIN;
            // Send request begin
            if(settings->on_req_begin && (stat = settings->on_req_begin(req->webserver, req, settings->httpd_ctx, &req->data))) {
                req->state = S_STOP; return stat;
            }

            return 0;
        default:
            HPD_LOG_ERROR(req->context, "Unexpected state.");
            req->state = S_ERROR;
            return 1;
    }
}

/**
 * URL callback for http_parser.
 *
 *  Called from http_parser with chunks of the URL. Each chunk is sent
 *  to the on_request_url() callback in hpd_tcpd_settings and the the URL
 *  parser.
 *
 *  @param  parser The http_parser calling.
 *  @param  buf    The buffer containing the chunk. Note that the buffer
 *                 is not \\0 terminated.
 *  @param  len    The length of the chunk.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_url(http_parser *parser, const char *buf, size_t len)
{
    hpd_error_t rc;
    hpd_httpd_return_t stat;
    hpd_httpd_request_t *req = parser->data;
    hpd_httpd_settings_t *settings = req->settings;

    switch ((enum http_method) req->parser.method) {
        case HTTP_GET:
            req->method = HPD_HTTPD_M_GET;
            break;
        case HTTP_PUT:
            req->method = HPD_HTTPD_M_PUT;
            break;
        case HTTP_OPTIONS:
            req->method = HPD_HTTPD_M_OPTIONS;
            break;
        default:
            req->method = HPD_HTTPD_M_UNKNOWN;
            break;
    }

    switch (req->state) {
        case S_STOP:
            return 1;
        case S_BEGIN:
            req->state = S_URL;
        case S_URL:
            if ((rc = up_add_chunk(req->url_parser, buf, len))) {
                HPD_LOG_ERROR(req->context, "URL parser failed (code: %d).", rc);
                req->state = S_ERROR;
                return 1;
            }
            if(settings->on_req_url && (stat = settings->on_req_url(req->webserver, req, settings->httpd_ctx, &req->data, buf, len))) {
                req->state = S_STOP;
                return stat;
            }
            return 0;
        default:
            HPD_LOG_ERROR(req->context, "Unexpected state.");
            req->state = S_ERROR;
            return 1;
    }
}

/**
 * Header field callback for http_parser.
 *
 *  Called from http_parser with chunks of a header field. The first
 *  call to this callback will trigger a on_request_url_complete. Each
 *  chunk is sent on to the on_request_header_field() callback from
 *  hpd_tcpd_settings.
 *
 *  @param  parser The http_parser calling.
 *  @param  buf    The buffer containing the chunk. Note that the buffer
 *                 is not \\0 terminated.
 *  @param  len    The length of the chunk.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_hdr_field(http_parser *parser, const char *buf, size_t len)
{
    hpd_error_t rc;
    hpd_httpd_return_t stat;
    hpd_httpd_request_t *req = parser->data;
    hpd_httpd_settings_t *settings = req->settings;

    switch (req->state) {
        case S_STOP:
            return 1;
        case S_URL:
            if ((rc = up_complete(req->url_parser))) {
                HPD_LOG_ERROR(req->context, "URL parser failed (code: %d).", rc);
                req->state = S_ERROR;
                return 1;
            }
            if(settings->on_req_url_cmpl && (stat = settings->on_req_url_cmpl(req->webserver, req, settings->httpd_ctx, &req->data))) {
                req->state = S_STOP;
                return stat;
            }
        case S_HEADER_VALUE:
            req->state = S_HEADER_FIELD;
        case S_HEADER_FIELD:
            if ((rc = hp_on_header_field(req->header_parser, buf, len))) {
                HPD_LOG_ERROR(req->context, "Header parser failed (code: %d).", rc);
                req->state = S_ERROR;
                return 1;
            }
            if(settings->on_req_hdr_field && (stat = settings->on_req_hdr_field(req->webserver, req, settings->httpd_ctx, &req->data, buf, len))) {
                req->state = S_STOP;
                return stat;
            }
            return 0;
        default:
            HPD_LOG_ERROR(req->context, "Unexpected state.");
            req->state = S_ERROR;
            return 1;
    }
}

/**
 * Header value callback for http_parser.
 *
 *  Called from http_parser with chunks of a header value. The chunk
 *  will be sent on to the on_request_header_value() callback in
 *  hpd_tcpd_settings.
 *
 *  @param  parser The http_parser calling.
 *  @param  buf    The buffer containing the chunk. Note that the buffer
 *                 is not \\0 terminated.
 *  @param  len    The length of the chunk.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_hdr_value(http_parser *parser, const char *buf, size_t len)
{
    hpd_error_t rc;
    hpd_httpd_return_t stat;
    hpd_httpd_request_t *req = parser->data;
    hpd_httpd_settings_t *settings = req->settings;

    switch (req->state) {
        case S_STOP:
            return 1;
        case S_HEADER_FIELD:
            req->state = S_HEADER_VALUE;
        case S_HEADER_VALUE:
            if ((rc = hp_on_header_value(req->header_parser, buf, len))) {
                HPD_LOG_ERROR(req->context, "Header parser failed (code: %d).", rc);
                req->state = S_ERROR;
                return 1;
            }
            if(settings->on_req_hdr_value && (stat = settings->on_req_hdr_value(req->webserver, req, settings->httpd_ctx, &req->data, buf, len))) {
                req->state = S_STOP;
                return stat;
            }
            return 0;
        default:
            HPD_LOG_ERROR(req->context, "Unexpected state.");
            req->state = S_ERROR;
            return 1;
    }
}

/**
 * Header complete callback for http_parser.
 *
 *  Called from http_parser when all headers are parsed. If any remains
 *  are left of the message, they are assumed to be the body. If there
 *  were no headers in the message, this will trigger a call to
 *  on_request_url_complete() from hpd_tcpd_settings. For both messages with
 *  or without headers, on_request_header_complete() will also be
 *  called.
 *
 *  @param  parser The http_parser calling.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_hdr_cmpl(http_parser *parser)
{
    hpd_error_t rc;
    hpd_httpd_return_t stat;
    hpd_httpd_request_t *req = parser->data;
    hpd_httpd_settings_t *settings = req->settings;

    switch (req->state) {
        case S_STOP:
            return 1;
        case S_URL:
            if ((rc = up_complete(req->url_parser))) {
                HPD_LOG_ERROR(req->context, "Header parser failed (code: %d).", rc);
                req->state = S_ERROR;
                return 1;
            }
            if(settings->on_req_url_cmpl && (stat = settings->on_req_url_cmpl(req->webserver, req, settings->httpd_ctx, &req->data))){
                req->state = S_STOP;
                return stat;
            }
        case S_HEADER_VALUE:
            if ((rc = hp_on_header_complete(req->header_parser))) {
                HPD_LOG_ERROR(req->context, "Header parser failed (code: %d).", rc);
                req->state = S_ERROR;
                return 1;
            }
            req->state = S_HEADER_COMPLETE;
            if(settings->on_req_hdr_cmpl && (stat = settings->on_req_hdr_cmpl(req->webserver, req, settings->httpd_ctx, &req->data))) {
                req->state = S_STOP;
                return stat;
            }
            return 0;
        default:
            HPD_LOG_ERROR(req->context, "Unexpected state.");
            req->state = S_ERROR;
            return 1;
    }
}

/**
 * Body callback for http_parser.
 *
 *  Called from http_parser each time it receives a chunk of the message
 *  body. Each chunk will be sent on to on_request_body() from
 *  hpd_tcpd_settings.
 *
 *  @param  parser The http_parser calling.
 *  @param  buf    The buffer containing the chunk. Note that the buffer
 *                 is not \\0 terminated.
 *  @param  len    The length of the chunk.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_body(http_parser *parser, const char *buf, size_t len)
{
    hpd_httpd_return_t stat;
    hpd_httpd_request_t *req = parser->data;
    hpd_httpd_settings_t *settings = req->settings;

    switch (req->state) {
        case S_STOP:
            return 1;
        case S_HEADER_COMPLETE:
            req->state = S_BODY;
        case S_BODY:
            if (settings->on_req_body) {
                stat = settings->on_req_body(req->webserver, req, settings->httpd_ctx, &req->data, buf, len);
                if (stat) { req->state = S_STOP; return stat; }
            }
            return 0;
        default:
            HPD_LOG_ERROR(req->context, "Unexpected state.");
            req->state = S_ERROR;
            return 1;
    }
}

/**
 * Messages complete callback for http_parser.
 *
 *  Called from http_parser when the full message have been parsed. This
 *  will trigger a call to on_request_complete() in hpd_tcpd_settings.
 *
 *  @param  parser The http_parser calling.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_msg_cmpl(http_parser *parser)
{
    hpd_httpd_return_t stat;
    hpd_httpd_request_t *req = parser->data;
    hpd_httpd_settings_t *settings = req->settings;

    switch (req->state) {
        case S_STOP:
            return 1;
        case S_HEADER_COMPLETE:
        case S_BODY:
            req->state = S_COMPLETE;
            if(settings->on_req_cmpl && (stat = settings->on_req_cmpl(req->webserver, req, settings->httpd_ctx, &req->data))) {
                req->state = S_STOP;
                return stat;
            }
            return 0;
        default:
            HPD_LOG_ERROR(req->context, "Unexpected state.");
            req->state = S_ERROR;
            return 1;
    }
}

/**
 * Create a new ws_request.
 *
 *  The created ws_request is ready to receive data through
 *  ws_reqeust_parse(), and it should be freed using
 *  ws_request_destroy() to avoid memory leaks.
 *
 *  @param  webserver  The httpd creating the request
 *  @param  settings   The settings for the webserver receiving the
 *                     request. This will determine which callbacks to
 *                     call on events.
 *  @param  conn       The connection on which the request is being
 *                     received
 *
 *  @return The newly create ws_request.
 */
hpd_error_t http_request_create(hpd_httpd_request_t **req, hpd_httpd_t *httpd, hpd_httpd_settings_t *settings,
                                hpd_tcpd_conn_t *conn, hpd_module_t *context)
{
    if (!context) return HPD_E_NULL;
    if (!req || !httpd || !settings || !conn) HPD_LOG_RETURN_E_NULL(context);

    hpd_error_t rc, rc2;

    (*req) = malloc(sizeof(hpd_httpd_request_t));
    if(!(*req)) HPD_LOG_RETURN_E_ALLOC(context);

    // Init references
    (*req)->context = context;
    (*req)->webserver = httpd;
    (*req)->conn = conn;
    (*req)->settings = settings;

    // Init parser
    http_parser_init(&((*req)->parser), HTTP_REQUEST);
    (*req)->parser.data = (*req);
    (*req)->state = S_START;

    // Init URL Parser
    struct up_settings up_settings = UP_SETTINGS_DEFAULT;
    up_settings.on_path_complete = url_parser_path_complete;
    up_settings.on_key_value = url_parser_key_value;
    if ((rc = up_create(&(*req)->url_parser, &up_settings, context, (*req)))) goto error;

    // Init Header Parser
    struct hp_settings hp_settings = HP_SETTINGS_DEFAULT;
    hp_settings.data = (*req);
    hp_settings.on_field_value_pair = header_parser_field_value_pair_complete;
    if ((rc = hp_create(&(*req)->header_parser, &hp_settings, context))) goto error;

    // Create linked maps
    if ((rc = hpd_map_alloc(&(*req)->arguments))) goto error;
    if ((rc = hpd_map_alloc(&(*req)->headers))) goto error;
    if ((rc = hpd_map_alloc(&(*req)->cookies))) goto error;

    // Other field to init
    (*req)->url = NULL;
    (*req)->data = NULL;

    return HPD_E_SUCCESS;
    
    error:
        if ((rc2 = http_request_destroy(*req)))
            HPD_LOG_ERROR(context, "Failed to destroy request (code: %d)\n", rc2);
        return rc;
}

/**
 * Destroy a ws_request.
 *
 *  All ws_requests should be freed by a call to this function to avoid
 *  memory leaks.
 *
 *  @param req The request to be destroyed.
 */
hpd_error_t http_request_destroy(hpd_httpd_request_t *req)
{
    if (!req) return HPD_E_NULL;
    
    hpd_error_t rc = HPD_E_SUCCESS, tmp;

    // Call callback
    hpd_httpd_settings_t *settings = req->settings;
    if (settings->on_req_destroy)
        settings->on_req_destroy(req->webserver, req, settings->httpd_ctx, &req->data);

    // Free request
    if ((tmp = up_destroy(req->url_parser)) && !rc) rc = tmp;
    if ((tmp = hpd_map_free(req->arguments)) && !rc) rc = tmp;
    if ((tmp = hpd_map_free(req->headers)) && !rc) rc = tmp;
    if ((tmp = hpd_map_free(req->cookies)) && !rc) rc = tmp;
    if ((tmp = hp_destroy(req->header_parser)) && !rc) rc = tmp;
    free(req->url);
    free(req);
    
    return rc;
}

/**
 * Parse a new chunk of the message.
 *
 *  This will sent the chunk to the http_parser, which will parse the
 *  new chunk and call the callbacks defined in parser_settings on
 *  events. The callbacks will change state of the ws_request and make
 *  calls on the functions defined in hpd_tcpd_settings.
 *
 *  @param  req The request, to which the chunk should be added.
 *  @param  buf The chunk, which is not assumed to be \\0 terminated.
 *  @param  len Length of the chuck.
 *
 *  @return What http_parser_execute() returns.
 */
hpd_error_t http_request_parse(hpd_httpd_request_t *req, const char *buf, size_t len)
{
    size_t read = http_parser_execute(&req->parser, &parser_settings, buf, len);

    enum http_errno err = HTTP_PARSER_ERRNO(&req->parser);

    switch (err) {
        case HPE_OK:
            break;
        case HPE_CB_message_begin:
        case HPE_CB_url:
        case HPE_CB_header_field:
        case HPE_CB_header_value:
        case HPE_CB_headers_complete:
        case HPE_CB_body:
        case HPE_CB_message_complete:
        case HPE_CB_status:
        case HPE_CB_chunk_header:
        case HPE_CB_chunk_complete:
            if (req->state == S_STOP) break;
        case HPE_INVALID_EOF_STATE:
        case HPE_HEADER_OVERFLOW:
        case HPE_CLOSED_CONNECTION:
        case HPE_INVALID_VERSION:
        case HPE_INVALID_STATUS:
        case HPE_INVALID_METHOD:
        case HPE_INVALID_URL:
        case HPE_INVALID_HOST:
        case HPE_INVALID_PORT:
        case HPE_INVALID_PATH:
        case HPE_INVALID_QUERY_STRING:
        case HPE_INVALID_FRAGMENT:
        case HPE_LF_EXPECTED:
        case HPE_INVALID_HEADER_TOKEN:
        case HPE_INVALID_CONTENT_LENGTH:
        case HPE_UNEXPECTED_CONTENT_LENGTH:
        case HPE_INVALID_CHUNK_SIZE:
        case HPE_INVALID_CONSTANT:
        case HPE_INVALID_INTERNAL_STATE:
        case HPE_STRICT:
        case HPE_PAUSED:
        case HPE_UNKNOWN:
            HPD_LOG_RETURN(req->context, HPD_E_ARGUMENT, "HTTP Parser failed with message: %s (code: %i).",
                           http_errno_description(err), err);
    }

    if (read != len  && req->state != S_STOP)
        HPD_LOG_RETURN(req->context, HPD_E_STATE, "Unexpected state.");

    if (req->parser.upgrade)
        HPD_LOG_RETURN(req->context, HPD_E_ARGUMENT, "Do not support HTTP upgrade messages.");

    return HPD_E_SUCCESS;
}

/**
 * Get the method of the http request.
 *
 *  \param  req  http request
 *
 *  \return The method as a enum http_method
 */
hpd_error_t hpd_httpd_request_get_method(hpd_httpd_request_t *req, hpd_httpd_method_t *method)
{
    if (!req) return HPD_E_NULL;
    if (!method) HPD_LOG_RETURN_E_NULL(req->context);

    (*method) = req->method;
    return HPD_E_SUCCESS;
}

/**
 * Get the URL of this request.
 *
 *  \param  req  http request
 *
 *  \return URL as a string
 */
hpd_error_t hpd_httpd_request_get_url(hpd_httpd_request_t *req, const char **url)
{
    if (!req) return HPD_E_NULL;
    if (!url) HPD_LOG_RETURN_E_NULL(req->context);

    (*url) = req->url;
    return HPD_E_SUCCESS;
}

/**
 * Get a linked map of all headers for a request.
 *
 *  \param  req  http request
 *
 *  \return Headers as a linkedmap (struct lm)
 */
hpd_error_t hpd_httpd_request_get_headers(hpd_httpd_request_t *req, hpd_map_t **headers)
{
    if (!req) return HPD_E_NULL;
    if (!headers) HPD_LOG_RETURN_E_NULL(req->context);

    (*headers) = req->headers;
    return HPD_E_SUCCESS;
}

/**
 * Get a specific header of a request.
 *
 *  \param  req  http request
 *  \param  key  Key for the header to get
 *
 *  \return The value of the header with the specified key, or NULL if
 *          not found
 */
hpd_error_t hpd_httpd_request_get_header(hpd_httpd_request_t *req, const char *key, const char **value)
{
    if (!req) return HPD_E_NULL;
    if (!key || !value) HPD_LOG_RETURN_E_NULL(req->context);

    return hpd_map_get(req->headers, key, value);
}

/**
 * Get a linked map of all URL arguements for a request.
 *
 *  \param  req  http request
 *
 *  \return Arguments as a linkedmap (struct lm)
 */
hpd_error_t hpd_httpd_request_get_arguments(hpd_httpd_request_t *req, hpd_map_t **arguments)
{
    if (!req) return HPD_E_NULL;
    if (!arguments) HPD_LOG_RETURN_E_NULL(req->context);

    (*arguments) = req->arguments;
    return HPD_E_SUCCESS;
}

/**
 * Get a specific argument of a request.
 *
 *  \param  req  http request
 *  \param  key  Key value of argument to get
 *
 *  \return Value of argument as string, or NULL if not found
 */
hpd_error_t hpd_httpd_request_get_argument(hpd_httpd_request_t *req, const char *key, const char **val)
{
    if (!req) return HPD_E_NULL;
    if (!key || !val) HPD_LOG_RETURN_E_NULL(req->context);

    return hpd_map_get(req->arguments, key, val);
}

/**
 * Get a all cookies for a request.
 *
 *  \param  req  http request
 *
 *  \return Cookies as a linkedmap (struct lm)
 */
hpd_error_t hpd_httpd_request_get_cookies(hpd_httpd_request_t *req, hpd_map_t **cookies)
{
    if (!req) return HPD_E_NULL;
    if (!cookies) HPD_LOG_RETURN_E_NULL(req->context);

    (*cookies) = req->cookies;
    return HPD_E_SUCCESS;
}

/**
 * Get a specific cookie for a request.
 *
 *  \param  req  http request
 *  \param  key  Key of cookie to get
 *
 *  \return The value of the cookie as String, or NULL if cookie was not
 *          found
 */
hpd_error_t hpd_httpd_request_get_cookie(hpd_httpd_request_t *req, const char *key, const char **val)
{
    if (!req) return HPD_E_NULL;
    if (!key || !val) HPD_LOG_RETURN_E_NULL(req->context);

    return hpd_map_get(req->cookies, key, val);
}

/**
 * Get the connection of a request.
 *
 *  \param  req  http request
 *
 *  \return The connection
 */
hpd_error_t http_request_get_connection(hpd_httpd_request_t *req, hpd_tcpd_conn_t **conn)
{
    if (!req) return HPD_E_NULL;
    if (!conn) HPD_LOG_RETURN_E_NULL(req->context);

    (*conn) = req->conn;
    return HPD_E_SUCCESS;
}

/**
 * Get the IP of a request.
 *
 *  \param  req  http request
 *
 *  \return IP as a string
 */
hpd_error_t hpd_httpd_request_get_ip(hpd_httpd_request_t *req, const char **ip)
{
    if (!req) return HPD_E_NULL;
    if (!ip) HPD_LOG_RETURN_E_NULL(req->context);

    return hpd_tcpd_conn_get_ip(req->conn, ip);
}

/**
 * Keep the connection for a request open.
 *
 *  Normally connections are closed when there has been no activity on
 *  it for the amount specified in the timeout field in the
 *  httpd settings struct. To keep the connection open forever,
 *  issue or call to this function with the http request. The connection
 *  will still be closed and destroyed when the client closes it.
 *
 *  \param  req  http request to keep open
 */
hpd_error_t hpd_httpd_request_keep_open(hpd_httpd_request_t *req)
{
    if (!req) return HPD_E_NULL;

    return hpd_tcpd_conn_keep_open(req->conn);
}

hpd_error_t http_request_get_context(hpd_httpd_request_t *req, hpd_module_t **context)
{
    if (!req) return HPD_E_NULL;
    if (!context) HPD_LOG_RETURN_E_NULL(req->context);

    (*context) = req->context;
    return HPD_E_SUCCESS;
}
