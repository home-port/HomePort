// request.c

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

#include "request.h"
#include "http-parser/http_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/// The possible states of a request
enum request_state {
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

/// An http request
/**
 *
 * <h1>Public interface</h1>
 *
 * nodata_cb and data_cb functions in ws_settings provides a request as
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
 * defined in ws_settings.
 *
 * ws_request_get_client() gets the client that sent the request.
 *
 * <h1>States</h1>
 *
 * A request will take states in the following order, the states of
 * S_STOP and S_ERROR can be assumed as always possible target for a
 * transition. The labels on the edges denote the callback from
 * ws_settings, that will be called upon the transition.
 * \dot
 * digraph request_states {
 * node [fontsize=10];
 * edge [fontsize=8];
 * S_START -> S_BEGIN
 * [ label = "on_requst_begin();\non_request_method();" ];
 * S_BEGIN -> S_URL
 * [ label = "on_request_url();" ];
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
struct ws_request
{
   struct libws_client *client;  ///< The client that sent the request
   struct ws_settings *settings; ///< Settings of the webserver
   http_parser parser;           ///< HTTP parser in use
   enum request_state state;     ///< Current state of the request
};

// Methods for http_parser settings
static int parser_message_begin_cb(http_parser *parser);
static int parser_url_cb(http_parser *parser, const char *buf, size_t len);
static int parser_header_field_cb(http_parser *parser, const char *buf, size_t len);
static int parser_header_value_cb(http_parser *parser, const char *buf, size_t len);
static int parser_headers_complete_cb(http_parser *parser);
static int parser_body_cb(http_parser *parser, const char *buf, size_t len);
static int parser_message_complete_cb(http_parser *parser);

/// Global settings for http_parser
static http_parser_settings parser_settings = 
{
   .on_message_begin = parser_message_begin_cb,
   .on_url = parser_url_cb,
   .on_status_complete = NULL,
   .on_header_field = parser_header_field_cb,
   .on_header_value = parser_header_value_cb,
   .on_headers_complete = parser_headers_complete_cb,
   .on_body = parser_body_cb,
   .on_message_complete = parser_message_complete_cb
};

/// Message begin callback for http_parser
/**
 *  Called when the http request message begins, by the http_parser.
 *  This will all on_request_begin() and on_request_method() from
 *  ws_settings.
 *
 *  @param  parser The http_parser calling.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_message_begin_cb(http_parser *parser)
{
   int stat = 0;
   const char *method;
   struct ws_request *req = parser->data;
   struct ws_settings *settings = req->settings;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_START:
         req->state = S_BEGIN;
         stat = settings->on_request_begin(req);
         if (stat) { req->state = S_STOP; return stat; }
         method = http_method_str(parser->method);
         stat = settings->on_request_method(req, method, strlen(method));
         if (stat) { req->state = S_STOP; return stat; }
         return 0;
      default:
         req->state = S_ERROR;
         return 1;
   }
}

/// URL callback for http_parser
/**
 *  Called from http_parser with chunks of the URL. Each chunk is sent
 *  to the on_request_url() callback in ws_settings.
 *
 *  @param  parser The http_parser calling.
 *  @param  buf    The buffer containing the chunk. Note that the buffer
 *                 is not \\0 terminated.
 *  @param  len    The length of the chunk.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_url_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct ws_request *req = parser->data;
   struct ws_settings *settings = req->settings;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_BEGIN:
         req->state = S_URL;
      case S_URL:
         stat = settings->on_request_url(req, buf, len);
         if (stat) { req->state = S_STOP; return stat; }
         return 0;
      default:
         req->state = S_ERROR;
         return 1;
   }
}

/// Header field callback for http_parser
/**
 *  Called from http_parser with chunks of a header field. The first
 *  call to this callback will trigger a on_request_url_complete. Each
 *  chunk is sent on to the on_request_header_field() callback from
 *  ws_settings.
 *
 *  @param  parser The http_parser calling.
 *  @param  buf    The buffer containing the chunk. Note that the buffer
 *                 is not \\0 terminated.
 *  @param  len    The length of the chunk.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_header_field_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct ws_request *req = parser->data;
   struct ws_settings *settings = req->settings;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_URL:
         stat = settings->on_request_url_complete(req);
         if (stat) { req->state = S_STOP; return stat; }
      case S_HEADER_VALUE:
         req->state = S_HEADER_FIELD;
      case S_HEADER_FIELD:
         stat = settings->on_request_header_field(req, buf, len);
         if (stat) { req->state = S_STOP; return stat; }
         return 0;
      default:
         req->state = S_ERROR;
         return 1;
   }
}

/// Header value callback for http_parser
/**
 *  Called from http_parser with chunks of a header value. The chunk
 *  will be sent on to the on_request_header_value() callback in
 *  ws_settings.
 *
 *  @param  parser The http_parser calling.
 *  @param  buf    The buffer containing the chunk. Note that the buffer
 *                 is not \\0 terminated.
 *  @param  len    The length of the chunk.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_header_value_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct ws_request *req = parser->data;
   struct ws_settings *settings = req->settings;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_HEADER_FIELD:
         req->state = S_HEADER_VALUE;
      case S_HEADER_VALUE:
         stat = settings->on_request_header_value(req, buf, len);
         if (stat) { req->state = S_STOP; return stat; }
         return 0;
      default:
         req->state = S_ERROR;
         return 1;
   }
}

/// Header complete callback for http_parser.
/**
 *  Called from http_parser when all headers are parsed. If any remains
 *  are left of the message, they are assumed to be the body. If there
 *  were no headers in the message, this will trigger a call to
 *  on_request_url_complete() from ws_settings. For both messages with
 *  or without headers, on_request_header_complete() will also be
 *  called.
 *
 *  @param  parser The http_parser calling.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_headers_complete_cb(http_parser *parser)
{
   int stat = 0;
   struct ws_request *req = parser->data;
   struct ws_settings *settings = req->settings;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_URL:
         stat = settings->on_request_url_complete(req);
         if (stat) { req->state = S_STOP; return stat; }
      case S_HEADER_VALUE:
         req->state = S_HEADER_COMPLETE;
         stat = settings->on_request_header_complete(req);
         if (stat) { req->state = S_STOP; return stat; }
         return 0;
      default:
         req->state = S_ERROR;
         return 1;
   }
}

/// Body callback for http_parser.
/**
 *  Called from http_parser each time it receives a chunk of the message
 *  body. Each chunk will be sent on to on_request_body() from
 *  ws_settings.
 *
 *  @param  parser The http_parser calling.
 *  @param  buf    The buffer containing the chunk. Note that the buffer
 *                 is not \\0 terminated.
 *  @param  len    The length of the chunk.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_body_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct ws_request *req = parser->data;
   struct ws_settings *settings = req->settings;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_HEADER_COMPLETE:
         req->state = S_BODY;
      case S_BODY:
         stat = settings->on_request_body(req, buf, len);
         if (stat) { req->state = S_STOP; return stat; }
         return 0;
      default:
         req->state = S_ERROR;
         return 1;
   }
}

/// Messages complete callback for http_parser.
/**
 *  Called from http_parser when the full message have been parsed. This
 *  will trigger a call to on_request_complete() in ws_settings.
 *
 *  @param  parser The http_parser calling.
 *  @return 1 to signal the parser to stop, or 0 to signal a continue.
 */
static int parser_message_complete_cb(http_parser *parser)
{
   int stat = 0;
   struct ws_request *req = parser->data;
   struct ws_settings *settings = req->settings;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_HEADER_COMPLETE:
      case S_BODY:
         req->state = S_COMPLETE;
         stat = settings->on_request_complete(req);
         if (stat) { req->state = S_STOP; return stat; }
         return 0;
      default:
         req->state = S_ERROR;
         return 1;
   }
}

/// Create a new ws_request
/**
 *  The created ws_request is ready to receive data through
 *  ws_reqeust_parse(), and it should be freed using
 *  ws_request_destroy() to avoid memory leaks.
 *
 *  @param  client   The client who sent the request.
 *  @param  settings The settings for the webserver receiving the
 *                   request. This will determine which callbacks to
 *                   call on events.
 *  @return The newly create ws_request.
 */
struct ws_request *ws_request_create(
      struct libws_client *client,
      struct ws_settings *settings)
{
   struct ws_request *req = malloc(sizeof(struct ws_request));
	if(req == NULL) {
		fprintf(stderr, "ERROR: Cannot allocate memory\n");
		return NULL;
	}

   // Init references
   req->client = client;
   req->settings = settings;

   // Init parser
   http_parser_init(&(req->parser), HTTP_REQUEST);
   req->parser.data = req;
   req->state = S_START;

   return req;
}

/// Destroy a ws_request
/**
 *  All ws_requests should be freed by a call to this function to avoid
 *  memory leaks.
 *
 *  @param req The request to be destroyed.
 */
void ws_request_destroy(struct ws_request *req)
{
   free(req);
}

/// Parse a new chunk of the message.
/**
 *  This will sent the chunk to the http_parser, which will parse the
 *  new chunk and call the callbacks defined in parser_settings on
 *  events. The callbacks will change state of the ws_request and make
 *  calls on the functions defined in ws_settings.
 *
 *  @param  req The request, to which the chunk should be added.
 *  @param  buf The chunk, which is not assumed to be \\0 terminated.
 *  @param  len Length of the chuck.
 *  @return What http_parser_execute() returns.
 */
size_t ws_request_parse(
      struct ws_request *req,
      const char *buf,
      size_t len)
{
   return http_parser_execute(&req->parser, &parser_settings, buf, len);
}

/// Get the client that sent the request.
/**
 *  @param  The request.
 *  @return The client, as provided to ws_request_create(), when the
 *  request was created.
 */
struct libws_client *ws_request_get_client(struct ws_request *req)
{
   return req->client;
}
