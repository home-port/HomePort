// reponse.c

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

#include "response.h"
#include "request.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define HTTP_VERSION "HTTP/1.1 "
#define CRLF "\r\n"

enum response_state {
   S_START,
   S_HEADER_FIELD,
   S_HEADER_VALUE,
   S_BODY
};

struct ws_response
{
   struct libws_client *client;
   enum response_state state;
   char *msg;
};

static char* http_status_codes_to_str(enum ws_http_status_code status)
{
#define XX(num, str) if(status == num) {return #str;}
	WS_HTTP_STATUS_CODE_MAP(XX)
#undef XX
	return NULL;
}

void ws_response_destroy(struct ws_response *res)
{
   free(res->msg);
   free(res);
}

struct ws_response *ws_response_create(
      struct ws_request *req,
      enum ws_http_status_code status)
{
   struct ws_response *res = NULL;
   int len;

   // Get data
   char *status_str = http_status_codes_to_str(status);

   // Calculate msg length
   len = 1;
   len += strlen(HTTP_VERSION);
   len += strlen(status_str);
   len += strlen(CRLF);
   
   // Allocate space
   res = malloc(sizeof(struct ws_response));
   if (res == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory\n");
      return NULL;
   }
   res->msg = malloc(len*sizeof(char));
   if (res == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory\n");
      ws_response_destroy(res);
      return NULL;
   }
  
   // Init struct
   res->client = ws_request_get_client(req);
   res->state = S_START;

   // Construct msg
   strcpy(res->msg, HTTP_VERSION);
   strcat(res->msg, status_str);
   strcat(res->msg, CRLF);

   return res;
}

int ws_response_add_header_field(struct ws_response *res,
      const char *buf, size_t len)
{
   char *msg;
   int msg_len = strlen(res->msg)+1;

   // Calculate new message size
   switch(res->state) {
      case S_HEADER_VALUE:
         msg_len += strlen(CRLF);
      case S_START:
      case S_HEADER_FIELD:
         msg_len += len;
      default:
         break;
   }

   // Allocate
   msg = realloc(res->msg, msg_len);
   if (msg == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory\n");
      return 1;
   }
   res->msg = msg;

   // Expand msg
   switch(res->state) {
      case S_HEADER_VALUE:
         strcat(res->msg, CRLF);
      case S_START:
         res->state = S_HEADER_FIELD;
      case S_HEADER_FIELD:
         sprintf(&(res->msg[strlen(msg)]), "%.*s", (int)len, buf);
         return 0;
      default:
         return 1;
   }
}

int ws_response_add_header_value(struct ws_response *res,
      const char *buf, size_t len)
{
   char *msg;
   int msg_len = strlen(res->msg)+1;

   // Calculate new message size
   switch(res->state) {
      case S_HEADER_FIELD:
         msg_len++;
      case S_HEADER_VALUE:
         msg_len += len;
      default:
         break;
   }

   // Allocate
   msg = realloc(res->msg, msg_len);
   if (msg == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory\n");
      return 1;
   }
   res->msg = msg;
   
   // Expand msg
   switch(res->state) {
      case S_HEADER_FIELD:
         strcat(res->msg, ":");
         res->state = S_HEADER_VALUE;
      case S_HEADER_VALUE:
         sprintf(&(res->msg[strlen(msg)]), "%.*s", (int)len, buf);
         return 0;
      default:
         return 1;
   }
}

int ws_response_add_body(struct ws_response *res,
      const char *buf, size_t len)
{
   char *msg;
   int msg_len = strlen(res->msg)+1;

   // Calculate new message size
   switch(res->state) {
      case S_HEADER_VALUE:
         msg_len += strlen(CRLF);
      case S_START:
         msg_len += strlen(CRLF);
      case S_BODY:
         msg_len += len;
      default:
         break;
   }

   // Allocate
   msg = realloc(res->msg, msg_len);
   if (msg == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory\n");
      return 1;
   }
   res->msg = msg;
   
   // Expand msg
   switch(res->state) {
      case S_HEADER_VALUE:
         strcat(res->msg, CRLF);
      case S_START:
         strcat(res->msg, CRLF);
         res->state = S_BODY;
      case S_BODY:
         sprintf(&(res->msg[strlen(msg)]), "%.*s", (int)len, buf);
         return 0;
      default:
         return 1;
   }
}

void ws_response_send(struct ws_response *res)
{
   fprintf(stderr, "ws_response_send is in a very early alpha version!\n");
   ws_client_sendf(res->client, res->msg);
}
