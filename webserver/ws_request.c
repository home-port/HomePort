// ws_request.c

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

#include "ws_request.h"

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

struct ws_request
{
   struct ws_client *client;
   
   enum request_state state;
   http_parser parser;
   char *hdr_field_tmp;
   char *hdr_value_tmp;

   char *url;                    ///< The URL requested.
   enum http_method method;      ///< The used method for a request.
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

static int check_request_size(struct ws_request *req)
{
   size_t size = sizeof(req) + sizeof(req->url);
   if (size > ws_client_get_settings(req->client)->max_request_size)
      return 1;
   else
      return 0;
}

void ws_request_destroy(struct ws_request *req)
{
   free(req->hdr_field_tmp);
   free(req->hdr_value_tmp);
   free(req->url);
   free(req);
}

struct ws_request *ws_request_create(struct ws_client *client)
{
   struct ws_request *req = malloc(sizeof(struct ws_request));
	if(req == NULL) {
		fprintf(stderr, "ERROR: Cannot allocate memory\n");
		return NULL;
	}

   // Init references
   req->client = client;

   // Init parser
   req->state = S_START;
   http_parser_init(&(req->parser), HTTP_REQUEST);
   req->parser.data = req;

   // Init data
   req->url = NULL;
   req->method = -1;
   req->hdr_field_tmp = NULL;
   req->hdr_value_tmp = NULL;

   // Check initial size
   if (check_request_size(req) != 0) {
      fprintf(stderr, "ERROR: Max request size is set too low\n");
      ws_request_destroy(req);
      return NULL;
   }

   return req;
}

struct ws_client *ws_request_get_client(struct ws_request *req)
{
   return req->client;
}

char *ws_request_get_url(struct ws_request *req)
{
   return req->url;
}

const char *ws_request_get_method_str(struct ws_request *req)
{
   return http_method_str(req->method);
}

const char *ws_request_get_client_ip(struct ws_request *req)
{
   return ws_client_get_ip(req->client);
}

static int cpy_str(
      struct ws_request *req,
      char **str,
      const char *buf,
      size_t len)
{
   char *new;

   if (*str == NULL)
      new = malloc((len+1)*sizeof(char));
   else
      new = realloc(*str, (len+1)*sizeof(char));

   if (new == NULL) {
      fprintf(stderr, "ERROR: Cannot allocation enough memory\n");
      return 1;
   }
   *str = new;

   if (check_request_size(req) != 0) {
      fprintf(stderr, "Request too big\n");
      free(*str);
      *str = NULL;
      return 1;
   }

   strncpy(*str, buf, len);
   (*str)[len] = '\0';
   return 0;
}

static int cat_str(
      struct ws_request *req,
      char **str,
      const char *buf,
      size_t len)
{
   size_t new_len = strlen(*str)+len+1;
   char *new_str = realloc(*str, new_len);
   
   if (new_str == NULL) {
      fprintf(stderr, "ERROR: Cannot allocation enough memory\n");
      return 1;
   }

   *str = new_str;
   if (check_request_size(req) != 0) {
      fprintf(stderr, "Request too big\n");
      free(*str);
      *str = NULL;
      return 1;
   }

   strncat(*str, buf, len);
   (*str)[len] = '\0';
   return 0;
}

size_t ws_request_parse(
      struct ws_request *req,
      const char *buf,
      size_t len)
{
   if (req->state == S_STOP) {
      return len;
   }

   if (req->state == S_ERROR) {
      fprintf(stderr, "ERROR: Parser in error state, ignoring input\n");
      return -1;
   }

   return http_parser_execute(&req->parser, &parser_settings, buf, len);
}

static int check_error(struct ws_request *req, int code)
{
   if (code)
      req->state = S_ERROR;
   return code;
}

static int check_stop(struct ws_request *req, int code)
{
   if (code)
      req->state = S_STOP;
   return 0;
}

static int parser_message_begin_cb(http_parser *parser)
{
   int stat = 0;
   struct ws_response;
   struct ws_request *req = parser->data;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_START:
         req->method = parser->method;
         req->state = S_BEGIN;
         stat = ws_client_on_request_begin(req->client, req);
         stat = check_stop(req, stat);
         break;
      default:
         stat = ws_client_on_request_error(req->client, req);
         break;
   }

   return check_error(req, stat);
}

static int parser_url_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct ws_request *req = parser->data;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_BEGIN:
         req->state = S_URL;
         stat = cpy_str(req, &req->url, buf, len);
         break;
      case S_URL:
         stat = cat_str(req, &req->url, buf, len);
         break;
      default:
         stat = ws_client_on_request_error(req->client, req);
         break;
   }

   return check_error(req, stat);
}

static int parser_header_field_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct ws_request *req = parser->data;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_URL:
         req->state = S_HEADER_FIELD;
         stat = ws_client_on_request_url(req->client, req, req->url);
         stat = check_stop(req, stat);
         if (stat) break;
         stat = cpy_str(req, &req->hdr_field_tmp, buf, len);
         break;
      case S_HEADER_FIELD:
         stat = cat_str(req, &req->hdr_field_tmp, buf, len);
         break;
      case S_HEADER_VALUE:
         req->state = S_HEADER_FIELD;
         stat = ws_client_on_request_header(req->client, req,
               req->hdr_field_tmp, req->hdr_value_tmp);
         stat = check_stop(req, stat);
         if (stat) break;
         stat = cpy_str(req, &req->hdr_field_tmp, buf, len);
         break;
      default:
         stat = ws_client_on_request_error(req->client, req);
         break;
   }

   return check_error(req, stat);
}

static int parser_header_value_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct ws_request *req = parser->data;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_HEADER_FIELD:
         req->state = S_HEADER_VALUE;
         stat = cpy_str(req, &req->hdr_value_tmp, buf, len);
         break;
      case S_HEADER_VALUE:
         stat = cat_str(req, &req->hdr_value_tmp, buf, len);
         break;
      default:
         stat = ws_client_on_request_error(req->client, req);
         break;
   }

   return check_error(req, stat);
}

static int parser_headers_complete_cb(http_parser *parser)
{
   int stat = 0;
   struct ws_request *req = parser->data;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_URL:
         req->state = S_HEADER_COMPLETE;
         stat = ws_client_on_request_url(req->client, req, req->url);
         stat = check_stop(req, stat);
         if (stat) break;
         stat = ws_client_on_request_header_complete(req->client, req);
         stat = check_stop(req, stat);
         break;
      case S_HEADER_VALUE:
         req->state = S_HEADER_COMPLETE;
         stat = ws_client_on_request_header(req->client, req,
               req->hdr_field_tmp, req->hdr_value_tmp);
         stat = check_stop(req, stat);
         if (stat) break;
         stat = ws_client_on_request_header_complete(req->client, req);
         stat = check_stop(req, stat);
         break;
      default:
         stat = ws_client_on_request_error(req->client, req);
         break;
   }

   return check_error(req, stat);
}

static int parser_body_cb(http_parser *parser, const char *buf, size_t len)
{
   int stat = 0;
   struct ws_request *req = parser->data;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_HEADER_COMPLETE:
         req->state = S_BODY;
         // break left out on purpose
      case S_BODY:
         stat = ws_client_on_request_body(req->client, req, buf, len);
         stat = check_stop(req, stat);
         break;
      default:
         stat = ws_client_on_request_error(req->client, req);
         break;
   }

   return check_error(req, stat);
}

static int parser_message_complete_cb(http_parser *parser)
{
   int stat = 0;
   struct ws_request *req = parser->data;

   switch (req->state) {
      case S_STOP:
         return 1;
      case S_HEADER_COMPLETE:
      case S_BODY:
         req->state = S_COMPLETE;
         stat = ws_client_on_request_complete(req->client, req);
         stat = check_stop(req, stat);
         break;
      default:
         stat = ws_client_on_request_error(req->client, req);
         break;
   }

   return check_error(req, stat);
}

