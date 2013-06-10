// instance.c

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

#include "libREST.h"
#include "trie.h"
#include "http-webserver.h"

#include <stdlib.h>
#include <stdio.h>

struct lr {
   struct lr_service *services; // TODO: Use the trie. Implement a map function on it!
   struct TrieNode *trie;
   struct httpws *webserver;
};

struct lr_service {
   lr_cb on_get;
   lr_cb on_post;
   lr_cb on_put;
   lr_cb on_delete;
   struct lr_service *next;
};

static void method_not_allowed(struct http_request *req)
{
  struct http_response *res = http_response_create(req, WS_HTTP_405);
  
  http_response_send(res, "Method Not Allowed");
}

// on_req_url_cmpl
static int on_url_cmpl(struct httpws *ins, struct http_request *req, void* ws_ctx, void** req_data)
{
  struct lr *lr_ins = ws_ctx;
  const char *url = http_request_get_url(req);

  if(url == NULL)
  {
    struct http_response *res = http_response_create(req, WS_HTTP_400);
    http_response_send(res, "Malformed URL");

    return 1;
  }

  struct ListElement *node = trie_lookup_node(lr_ins->trie, url);

  if(node == NULL) // URL not registered
  {
    struct http_response *res = http_response_create(req, WS_HTTP_404);
    // TODO: Find out if we need to add headers

    http_response_send(res, "Resource not found"); // TODO: Decide on appropriate body
    return 1;
  }

  struct lr_service *service = get_listElement_value(node);
  if(service == NULL)
  {
    fprintf(stderr, "Error: The service for an element in the trie was NULL!\n");
    method_not_allowed(req);
    return 1;
  }

  switch(http_request_get_method(req))
  {
    case HTTP_GET:
      if(service->on_get == NULL){
        method_not_allowed(req);
        return 1;
      }
    break;

    case HTTP_DELETE:
      if(service->on_delete == NULL){
        method_not_allowed(req);
        return 1;
      }

    break;

    case HTTP_POST:
      if(service->on_post == NULL){
        method_not_allowed(req);
        return 1;
      }

    break;

    case HTTP_PUT:
      if(service->on_put == NULL){
        method_not_allowed(req);
        return 1;
      }
    break;

    default:
        method_not_allowed(req);
        return 1;
  }

  *req_data = service;

  return 0;
}

static int on_body(struct httpws *ins, struct http_request *req, void* ws_ctx, void** req_data, const char* chunk, size_t len)
{
  struct lr_service *service = *req_data;

  switch(http_request_get_method(req))
  {
    case HTTP_GET:
      return service->on_get(req, chunk, len);

    case HTTP_DELETE:
      return service->on_delete(req, chunk, len);

    case HTTP_POST:
      return service->on_post(req, chunk, len);

    case HTTP_PUT:
      return service->on_put(req, chunk, len);

    default:
      return 1;
  }
}

static int on_cmpl(struct httpws *ins, struct http_request *req, void* ws_ctx, void** req_data)
{
  return on_body(ins, req, ws_ctx, req_data, NULL, 0);
}

struct lr *lr_create(struct lr_settings *settings, struct ev_loop *loop)
{
   struct lr *ins = malloc(sizeof(struct lr));
   if (ins == NULL) {
      fprintf(stderr, "Cannot allocate lr instance\n");
      return NULL;
   }

   ins->trie = trie_create();

   ins->services = NULL;

   struct httpws_settings ws_set = HTTPWS_SETTINGS_DEFAULT;
   ws_set.port = settings->port;
   ws_set.timeout = settings->timeout;
   ws_set.ws_ctx = ins;
   ws_set.on_req_url_cmpl = on_url_cmpl;
   ws_set.on_req_body = on_body;
   ws_set.on_req_cmpl = on_cmpl;

   ins->webserver = httpws_create(&ws_set, loop);

   return ins;
}

void lr_destroy(struct lr *ins)
{
   // TODO Remove the list and this destroy hack, when trie supports a
   // function to free objects
   if(ins != NULL) {
      struct lr_service *next;
      struct lr_service *service = ins->services;
 
      while (service != NULL) {
         next = service->next;
         free(service);
         service = next;
      }
 
      httpws_destroy(ins->webserver);
 
      trie_destroy(ins->trie);
      free(ins);
   }
}

int lr_start(struct lr *ins)
{
  if(ins)
    return httpws_start(ins->webserver);
  return 1;
}

void lr_stop(struct lr *ins)
{
  if(ins)
    httpws_stop(ins->webserver);
}

void lr_register_service(struct lr *ins,
                         char *url,
                         lr_cb on_get,
                         lr_cb on_post,
                         lr_cb on_put,
                         lr_cb on_delete)
{
   struct lr_service *service = ins->services;

   if (service == NULL) {
      ins->services = malloc(sizeof(struct lr_service));
      service = ins->services;
   } else {
      while (service->next != NULL) service = service->next;
      service->next = malloc(sizeof(struct lr_service));
      service = service->next;
   }
   
   if (service == NULL) {
      fprintf(stderr, "Not enough memory to allocate service\n");
      return;
   }

   service->on_get = on_get;
   service->on_post = on_post;
   service->on_put = on_put;
   service->on_delete = on_delete;
   service->next = NULL;

   struct ListElement* element = trie_insert(ins->trie, url);
   set_listElement_value(element, service);
}


// TODO: Make lr_unregister_service

