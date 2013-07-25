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
   struct trie *trie;
   struct httpws *webserver;
};

struct lr_service {
   lr_cb on_get;
   lr_cb on_post;
   lr_cb on_put;
   lr_cb on_delete;
   void *data;
};

struct lr_request {
   struct lr_service *service;
   struct http_request *req;
   struct http_response *res;
};

static void method_not_allowed(struct http_request *req)
{
   struct http_response *res = http_response_create(req, WS_HTTP_405);
   http_response_sendf(res, "Method Not Allowed");
   http_response_destroy(res);
}

// on_req_url_cmpl
static int on_url_cmpl(struct httpws *ins, struct http_request *req,
                       void* ws_ctx, void** req_data)
{
  struct lr *lr_ins = ws_ctx;
  const char *url = http_request_get_url(req);

  if(url == NULL) {
    struct http_response *res = http_response_create(req, WS_HTTP_400);
    http_response_sendf(res, "Malformed URL");
    http_response_destroy(res);

    return 1;
  }

  struct trie_iter *iter = trie_lookup(lr_ins->trie, url);

  if(iter == NULL) { // URL not registered
     struct http_response *res = http_response_create(req, WS_HTTP_404);
     // TODO: Find out if we need to add headers
     http_response_sendf(res, "Resource not found"); // TODO: Decide on appropriate body
     http_response_destroy(res);
     return 1;
  }

  struct lr_service *service = trie_value(iter);
  if(service == NULL) {
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

  struct lr_request *lrreq = malloc(sizeof(struct lr_request));
  lrreq->service = service;
  lrreq->req = req;
  lrreq->res = NULL;
  *req_data = lrreq;

  return 0;
}

static int on_body(struct httpws *ins, struct http_request *req,
                   void* ws_ctx, void** req_data,
                   const char* chunk, size_t len)
{
  struct lr_request *lrreq = *req_data;
  struct lr_service *service = lrreq->service;

  switch(http_request_get_method(req))
  {
    case HTTP_GET:
      return service->on_get(service->data, lrreq, chunk, len);

    case HTTP_DELETE:
      return service->on_delete(service->data, lrreq, chunk, len);

    case HTTP_POST:
      return service->on_post(service->data, lrreq, chunk, len);

    case HTTP_PUT:
      return service->on_put(service->data, lrreq, chunk, len);

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

void free_service(void *element)
{
	free(element);
}

void lr_destroy(struct lr *ins)
{
   if(ins != NULL) {
      httpws_destroy(ins->webserver);
      trie_destroy(ins->trie, free_service);

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

int lr_register_service(struct lr *ins,
                         char *url,
                         lr_cb on_get,
                         lr_cb on_post,
                         lr_cb on_put,
                         lr_cb on_delete,
                         void *data)
{
   struct lr_service *service = malloc(sizeof(struct lr_service));

   if (service == NULL) {
      fprintf(stderr, "Not enough memory to allocate service\n");
      return 1;
   }

   service->on_get = on_get;
   service->on_post = on_post;
   service->on_put = on_put;
   service->on_delete = on_delete;
   service->data = data;

   struct trie_iter* iter = trie_insert(ins->trie, url, service);

   if(iter == NULL){
      return 1;
   }

   return 0;
}

void *lr_unregister_service(struct lr *ins, char *url)
{
   struct lr_service *service  = trie_remove(ins->trie, url);
   void *data = service->data;
   free(service);
   return data;
}

void *lr_lookup_service(struct lr *ins, char *url)
{
   struct trie_iter *iter = trie_lookup(ins->trie, url);
   if (!iter) return NULL;
   return trie_value(iter);
}

void lr_sendf(struct lr_request *req, enum httpws_http_status_code status,
              char *fmt, ...)
{
   va_list arg;

   va_start(arg, fmt);
   lr_send_start(req, status);
   lr_send_vchunkf(req, fmt, arg);
   lr_send_stop(req);
   va_end(arg);
}

void lr_request_destroy(struct lr_request *req)
{
   free(req);
}

void lr_send_start(struct lr_request *req,
                   enum httpws_http_status_code status)
{
   req->res = http_response_create(req->req, status);
   // TODO Consider headers to add
}

void lr_send_chunkf(struct lr_request *req, char *fmt, ...)
{
   va_list arg;
   va_start(arg, fmt);
   lr_send_vchunkf(req, fmt, arg);
   va_end(arg);
}

void lr_send_vchunkf(struct lr_request *req, char *fmt, va_list arg)
{
   http_response_vsendf(req->res, fmt, arg);
}

void lr_send_stop(struct lr_request *req)
{
   http_response_destroy(req->res);
}

enum http_method lr_request_get_method(struct lr_request *req)
{
   return http_request_get_method(req->req);
}

const char *lr_request_get_url(struct lr_request *req)
{
   return http_request_get_url(req->req);
}

struct lm *lr_request_get_headers(struct lr_request *req)
{
   return http_request_get_headers(req->req);
}

const char *lr_request_get_header(struct lr_request *req, const char* key)
{
   return http_request_get_header(req->req, key);
}

struct lm *lr_request_get_arguments(struct lr_request *req)
{
   return http_request_get_arguments(req->req);
}

const char *lr_request_get_argument(struct lr_request *req, const char* key)
{
   return http_request_get_argument(req->req, key);
}


