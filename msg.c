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

#define HTTP_VERSION "HTTP/1.1"

struct ws_msg
{
	int status_code;
	char* body;
};

struct ws_msg* ws_msg_create(int status_code, char* body)
{
	struct ws_msg *msg = malloc(sizeof (struct ws_msg));
	if(msg == NULL)
	{
		perror("Malloc failed when allocating response message: ");
		return NULL;
	}

	msg->status_code = status_code;
	msg->body = body;

	return msg;
}

void ws_msg_destroy(struct ws_msg *msg)
{
	free(msg);
}

static char* str_builder(char* msg, char* to_append)
{
	size_t len = strlen(to_append)+1;

	if(msg != NULL)
	{
		len += strlen(msg);
	}

	// TODO: There might be a memory leak if realloc fails
	msg = realloc(msg, len * sizeof(char));
	if(msg == NULL)
	{
		perror("realloc failed when allocating response message: ");
		return NULL;
	}

	strcat(msg, to_append);

	return msg;
}

char* ws_msg_tostring(struct ws_msg* msg)
{
	char* response = NULL;
	response = str_builder(response, HTTP_VERSION);
	response = str_builder(response, " 200 OK\n");

	return response;
}