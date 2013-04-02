// ws_parser.c

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

#include "ws_parser.h"
#include "ws_http.h"
#include "http-parser/http_parser.h"
#include <stdlib.h>
#include <stdio.h>

enum parser_state {
   S_START,
   S_BEGIN,
   S_URL,
   S_HEADER_FIELD,
   S_HEADER_VALUE,
   S_HEADER_COMPLETE,
   S_BODY,
   S_COMPLETE
};

struct ws_parser {
   enum parser_state state;
   http_parser parser;
};

// Methods for http_parser settings
static int parser_message_begin_cb(http_parser *parser);
static int parser_url_cb(http_parser *parser, const char *buf,
      size_t len);
static int parser_header_field_cb(http_parser *parser, const char *buf,
      size_t len);
static int parser_header_value_cb(http_parser *parser, const char *buf,
      size_t len);
static int parser_headers_complete_cb(http_parser *parser);
static int parser_body_cb(http_parser *parser, const char *buf,
      size_t len);
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

struct ws_parser *ws_parser_create()
{
   struct ws_parser *parser = malloc(sizeof(struct ws_parser));
   if (parser == NULL) {
      fprintf(stderr, "Not enough memory!\n");
      return NULL;
   }

   parser->state = S_START;
   http_parser_init(&(parser->parser), HTTP_REQUEST);
   parser->parser.data = parser;

   return parser;
}

void ws_parser_destroy(struct ws_parser *parser)
{
   free(parser);
}

size_t ws_parser_parse(
      struct ws_parser *parser,
      const char *buf,
      size_t len)
{
   return http_parser_execute(&parser->parser, &parser_settings, buf, len);
}

static int parser_message_begin_cb(http_parser *parser)
{
   switch (parser->state) {
      case S_START:
         // TODO SET METHOD IN REQUEST
         // TODO CALLBACK ON BEGIN
         parser->state = S_BEGIN;
         return 0;
      default:
         // TODO ERROR
         return 1;
   }

}

static int parser_url_cb(http_parser *parser, const char *buf, size_t len)
{
   switch (parser->state) {
      case S_BEGIN:
         // TODO START URL STRING
         parser->state = S_URL;
         return 0;
      case S_URL:
         // TODO CAT URL
         return 0;
      default:
         return 1;
   }
}

static int parser_header_field_cb(http_parser *parser, const char *buf, size_t len)
{
   switch (parser->state) {
      case S_URL:
         // TODO CALLBACK ON URL
         // TODO START FIELD STRING
         parser->state = S_HEADER_FIELD;
         return 0;
      case S_HEADER_FIELD:
         // TODO CAT FIELD
         return 0;
      case S_HEADER_VALUE:
         // TODO CALLBACK ON HEADER PAIR
         // TODO START HEADER FIELD
         parser->state = S_HEADER_FIELD;
         return 0;
      default:
         return 1;
   }
}

static int parser_header_value_cb(http_parser *parser, const char *buf, size_t len)
{
   switch (parser->state) {
      case S_HEADER_FIELD:
         // TODO START VALUE STRING
         parser->state = S_HEADER_VALUE;
         return 0;
      case S_HEADER_VALUE:
         // TODO CAT VALUE
         return 0;
      default:
         return 1;
   }
}

static int parser_headers_complete_cb(http_parser *parser)
{
   switch (parser->state) {
      case S_URL:
         // TODO CALLBACK ON URL
         parser->state = S_HEADER_COMPLETE;
         return 0;
      case S_HEADER_VALUE:
         // TODO CALLBACK ON HEADER PAIR
         parser->state = S_HEADER_COMPLETE;
         return 0;
      default:
         return 1;
   }
}

static int parser_body_cb(http_parser *parser, const char *buf, size_t len)
{
   switch (parser->state) {
      case S_HEADER_COMPLETE:
         // TODO SEND BODY STUB
         parser->state = S_BODY;
         return 0;
      case S_BODY:
         // TODO SEND BODY STUB
         return 0;
      default:
         return 1;
   }
}

static int parser_message_complete_cb(http_parser *parser)
{
   switch (parser->state) {
      case S_HEADER_COMPLETE:
      case S_BODY:
         // TODO SEND COMPLETE
         parser->state = S_COMPLETE;
         return 0;
      default:
         return 1;
   }
}

