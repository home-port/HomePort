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

#include <stdlib.h>
#include <stdio.h>

#include "libREST.h"
#include "url_parser.h"
#include "instance.h"



struct lr_request {
	struct lr *instance;
	struct url_parser_instance *url_parser;
	enum lr_method method;
	void *data;
};

static void on_path_complete(void *_req, const char* path, size_t len)
{
	struct lr_request *req = _req;

	struct TrieNode *trie = lr_get_trie(req->instance); 

	struct ListElement *el = lookup_trieNode(trie, path);
	if(el != NULL) {

		struct lr_service *service = get_listElement_value(el);

		lr_service_call(service, req->method, req->data, path, len);

	} else {
		printf("Not in trie! %.*s \n", (int)len, path);
	}

}

struct lr_request *lr_request_create(struct lr *instance, void *data)
{
	struct lr_request *request = malloc(sizeof(struct lr_request));

	if(request == NULL)
	{
		fprintf(stderr, "malloc failed in libREST when creating instance\n");
		return NULL;
	}

	struct url_parser_settings settings = URL_PARSER_SETTINGS_DEFAULT;
	settings.on_path_complete = on_path_complete;

	struct url_parser_instance *url_parser = up_create(&settings, request);
	request->url_parser = url_parser;

	request->instance = instance;

	request->data = data;

	return request;
}

void lr_request_destroy(struct lr_request *req)
{
	if(req) {
		up_destroy(req->url_parser);

		free(req);
	}
}

int lr_request_method(void *_req, const char *chunk, size_t len)
{
	struct lr_request *req = _req;

	if(len == 6 && strncmp("DELETE",chunk,len) == 0){
		req->method = DELETE;
	} else if(len == 4 && strncmp("POST",chunk,len) == 0){
		req->method = POST;
	} else if(len == 3 && strncmp("GET",chunk,len) == 0){
		req->method = GET;
	} else if(len == 3 && strncmp("PUT",chunk,len) == 0){
		req->method = PUT;
	} else {
		fprintf(stderr, "Unsupported method: %.*s\n", (int)len, chunk);
		return 1;
	}

	return 0;
}

int lr_request_url(void *_req, const char *chunk, size_t len)
{
	struct lr_request *req = _req;
	return up_add_chunk(req->url_parser, chunk, len);
}

int lr_request_url_cmpl(void *_req)
{
	struct lr_request *req = _req;
	return up_complete(req->url_parser);
}

int lr_request_hdr_field(void *_req, const char *chunk, size_t len)
{
	struct lr_request *req = _req;
	return 0;
}

int lr_request_hdr_value(void *_req, const char *chunk, size_t len)
{
	struct lr_request *req = _req;
	return 0;
}

int lr_request_hdr_cmpl(void *_req)
{
	struct lr_request *req = _req;
	return 0;
}

int lr_request_body(void *_req, const char *chunk, size_t len)
{
	struct lr_request *req = _req;
	return 0;
}

int lr_request_cmpl(void *_req)
{
	struct lr_request *req = _req;
	return 0;
}

