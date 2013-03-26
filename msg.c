// msg.c

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

#include "msg.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define HTTP_VERSION "HTTP/1.1"
#define SP " "
#define CRLF "\r\n"

static char* http_status_codes_to_str(enum http_status_codes status)
{
#define XX(num, str) if(status == num) {return #str;}
	HTTP_STATUS_CODE_MAP(XX)
#undef XX
	return NULL;
}

struct ws_msg
{
	enum http_status_codes status_code;
	char* body;

	char* msg_as_string;
};

struct ws_msg* ws_msg_create(enum http_status_codes status_code, char* body)
{
	struct ws_msg *msg = malloc(sizeof (struct ws_msg));
	if(msg == NULL)
	{
		perror("Malloc failed when allocating response message: ");
		return NULL;
	}

	msg->status_code = status_code;
	msg->body = body;

	msg->msg_as_string = NULL;

	return msg;
}

void ws_msg_destroy(struct ws_msg *msg)
{
	free(msg->msg_as_string);
	free(msg);
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

char* ws_msg_tostring(struct ws_msg* msg)
{
	char* response = NULL;

	response = str_builder(response, HTTP_VERSION);
	response = str_builder(response, SP);
	response = str_builder(response, http_status_codes_to_str(msg->status_code));
	response = str_builder(response, CRLF);

	if(msg->body != NULL)
	{
		int body_length = strlen(msg->body);
		char body_length_str[(int)floor(log10(abs(body_length))) + 2];
		sprintf(body_length_str, "%d", body_length);

		response = str_builder(response, "Content-Length: ");
		response = str_builder(response, body_length_str);
		response = str_builder(response, CRLF);
	}

	response = str_builder(response, CRLF);
	if(msg->body != NULL)
	{
		response = str_builder(response, msg->body);
	}

	if(msg->msg_as_string != NULL)
		free(msg->msg_as_string);
	msg->msg_as_string = response;

	return msg->msg_as_string;
}