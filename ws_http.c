// ws_http.c

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

#include "ws_http.h"
#include "http-parser/http_parser.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/// The maximum lengt of the URL
#define MAXURLLENGTH 512

/// The maximum length of the body

#define HTTP_VERSION "HTTP/1.1"
#define SP " "
#define CRLF "\r\n"

struct ws_request
{
   struct ws_client *client;
   http_parser parser;                   ///< The parser in use.

   char url[MAXURLLENGTH];       ///< The URL requested.
   enum http_method method;      ///< The used method for a request.
   char *body;     ///< The BODY from the request.
};

struct ws_response
{
   struct ws_client *client;

	enum ws_http_status_code status;
	char* body;
	char* full_string;
};

struct ws_request *ws_request_create(struct ws_client *client)
{
   struct ws_request *req = malloc(sizeof(struct ws_request));
	if(req == NULL)
	{
		fprintf(stderr, "ERROR: Cannot allocate memory\n");
		return NULL;
	}

   req->client = client;
   http_parser_init(&(req->parser), HTTP_REQUEST);
   req->parser.data = req;
   req->url[0] = '\0';
   req->body = malloc(sizeof(char));
   req->body[0] = '\0';
   req->method = -1;

   return req;
}

void ws_request_destroy(struct ws_request *req)
{
   free(req->body);
   free(req);
}

struct ws_response *ws_response_create(
      struct ws_request *req,
      enum ws_http_status_code status,
      char *body)
{
   struct ws_response *res = malloc(sizeof(struct ws_response));
	if(res == NULL)
	{
		fprintf(stderr, "ERROR: Cannot allocate memory\n");
		return NULL;
	}

   res->client = req->client;
   res->status = status;
   res->body = NULL;
   res->full_string = NULL;

   if (body != NULL) {
      res->body = malloc((strlen(body)+1)*sizeof(char));
	   if(res->body == NULL)
	   {
	   	fprintf(stderr, "ERROR: Cannot allocate memory\n");
	   	return NULL;
	   }
      strcpy(res->body, body);
   } else {
      res->body = NULL;
   }

   return res;
}

void ws_response_destroy(struct ws_response *res)
{
   free(res->body);
   free(res->full_string);
   free(res);
}

struct ws_client *ws_request_get_client(struct ws_request *req)
{
   return req->client;
}

void ws_request_set_method(struct ws_request *req)
{
   req->method = req->parser.method;
}

char *ws_request_get_url(struct ws_request *req)
{
   return req->url;
}

const char *ws_request_get_method_str(struct ws_request *req)
{
   return http_method_str(req->method);
}

char *ws_request_get_body(struct ws_request *req)
{
   return req->body;
}

void ws_request_cat_url(
      struct ws_request *req,
      const char *buf,
      size_t len)
{
   // TODO Check of out-of-bounds
   strncat(req->url, buf, len);
}

int ws_request_cat_body(
      struct ws_request *req,
      const char *buf,
      size_t len)
{
   size_t new_len = strlen(req->body)+len+1;
   char *new_body = realloc(req->body, new_len);

   if (new_body == NULL) {
      fprintf(stderr, "ERROR: Cannot allocation enough memory for " \
            "body\n");
      return 1;
   }

   req->body = new_body;
   strncat(req->body, buf, len);
   return 0;
}

size_t ws_request_parse(
      struct ws_request *req,
      http_parser_settings *settings,
      const char *buf,
      size_t len)
{
   return http_parser_execute(&req->parser, settings, buf, len);
}

static char* http_status_codes_to_str(enum ws_http_status_code status)
{
#define XX(num, str) if(status == num) {return #str;}
	WS_HTTP_STATUS_CODE_MAP(XX)
#undef XX
	return NULL;
}

static char* str_builder(char* old_msg, char* to_append)
{
	// TODO: Need to allocate space in 1 step (pre-calculate it)
	char* new_msg;
	size_t len = strlen(to_append)+1;

	if(old_msg != NULL)	{
		len += strlen(old_msg);
	}

	new_msg = realloc(old_msg, len * sizeof(char));
	
	if(new_msg == NULL)	{
		perror("Failed when allocating response message: ");
		return old_msg;
	}

	if(old_msg == NULL)	{
		strcpy(new_msg, to_append);
	} else {
		strcat(new_msg, to_append);
	}

	return new_msg;
}

char* ws_response_str(struct ws_response* res)
{
	char* response = NULL;

	response = str_builder(response, HTTP_VERSION);
	response = str_builder(response, SP);
	response = str_builder(response, http_status_codes_to_str(res->status));
	response = str_builder(response, CRLF);

	if(res->body != NULL)
	{
		int body_length = strlen(res->body);
		char body_length_str[(int)floor(log10(abs(body_length))) + 2];
		sprintf(body_length_str, "%d", body_length);

		response = str_builder(response, "Content-Length:");
		response = str_builder(response, body_length_str);
		response = str_builder(response, CRLF);
	}

	response = str_builder(response, CRLF);
	if(res->body != NULL)
	{
		response = str_builder(response, res->body);
	}

	if(res->full_string != NULL)
		free(res->full_string);
	res->full_string = response;

	return res->full_string;
}
