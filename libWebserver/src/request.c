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

enum request_state {
   S_START,
   S_BEGIN,
   S_URL,
   S_HEADER_FIELD,
   S_HEADER_VALUE,
   S_HEADER_COMPLETE,
   S_BODY,
   S_COMPLETE,
   S_STOP,
   S_ERROR
};

struct libws_request
{
   struct libws_client *client;
   struct libws_settings *settings;
   http_parser parser;
   enum request_state state;
};

// Methods for http_parser settings
static int parser_message_begin_cb(http_parser *parser);
static int parser_url_cb(http_parser *parser, const char *buf, size_t len);
static int parser_header_field_cb(http_parser *parser, const char *buf, size_t len);
static int parser_header_value_cb(http_parser *parser, const char *buf, size_t len);
static int parser_headers_complete_cb(http_parser *parser);
static int parser_body_cb(http_parser *parser, const char *buf, size_t len);
static int parser_message_complete_cb(http_parser *parser);

// Global settings for http_parser
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

static int parser_message_begin_cb(http_parser *parser)
{
   int stat = 0;
   const char *method;
   struct libws_request *req = parser->data;
   struct libws_settings *settings = req->settings;

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

static int parser_url_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct libws_request *req = parser->data;
   struct libws_settings *settings = req->settings;

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

static int parser_header_field_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct libws_request *req = parser->data;
   struct libws_settings *settings = req->settings;

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

static int parser_header_value_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct libws_request *req = parser->data;
   struct libws_settings *settings = req->settings;

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

static int parser_headers_complete_cb(http_parser *parser)
{
   int stat = 0;
   struct libws_request *req = parser->data;
   struct libws_settings *settings = req->settings;

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

static int parser_body_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct libws_request *req = parser->data;
   struct libws_settings *settings = req->settings;

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

static int parser_message_complete_cb(http_parser *parser)
{
   int stat = 0;
   struct libws_request *req = parser->data;
   struct libws_settings *settings = req->settings;

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

struct libws_request *libws_request_create(
      struct libws_client *client,
      struct libws_settings *settings)
{
   struct libws_request *req = malloc(sizeof(struct libws_request));
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

void libws_request_destroy(struct libws_request *req)
{
   free(req);
}

size_t libws_request_parse(
      struct libws_request *req,
      const char *buf,
      size_t len)
{
   return http_parser_execute(&req->parser, &parser_settings, buf, len);
}

struct libws_client *libws_request_get_client(struct libws_request *req)
{
   return req->client;
}
