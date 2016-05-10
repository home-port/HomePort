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
   lr_data_cb on_get;
   lr_data_cb on_post;
   lr_data_cb on_put;
   lr_data_cb on_delete;
   lr_nodata_cb on_destroy;
   void *srv_data;
};

struct lr_request {
   struct lr_service *service;
   struct http_request *req;
   struct http_response *res;
   void *data;
};

static void lr_service_print(struct lr_service *srv)
{
   printf("----- libRest Service -----\n");
   if (!srv) {
      printf("   (null)\n");
   } else {
      printf("   Callbacks:");
      if (srv->on_get) printf (" GET");
      if (srv->on_post) printf (" POST");
      if (srv->on_put) printf (" PUT");
      if (srv->on_delete) printf (" DELETE");
      if (srv->on_destroy) printf (" DESTROY");
      printf("\n");
   }
}

void lr_request_print(struct lr_request *req)
{
   printf("----- libRest Request -----\n");
   if (!req) {
      printf("   (null)\n");
   } else {
      lr_service_print(req->service);
      http_request_print(req->req);
      http_response_print(req->res);
   }
}

static void lr_request_destroy(struct lr_request *req)
{
   free(req);
}

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

  printf("[librest] Got request for '%s'\n", url);

  if (url == NULL) {
    struct http_response *res = http_response_create(req, WS_HTTP_400);
    http_response_sendf(res, "Malformed URL");
    http_response_destroy(res);

    return 1;
  }

  struct trie_iter *iter = trie_lookup(lr_ins->trie, url);

  if (iter == NULL) { // URL not registered
     fprintf(stderr, "[librest] Service on '%s' not found\n", url);
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
#ifdef LR_ORIGIN
     case HTTP_OPTIONS:
        break;
#endif

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
  lrreq->data = NULL;
  *req_data = lrreq;

  return 0;
}

static int on_hdr_cmpl(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data)
{
  struct lr_request *lrreq = *req_data;
  struct lr_service *service = lrreq->service;
  struct http_response *res;
  char methods[23];
  methods[0] = '\0';

#ifdef LR_ORIGIN
  switch(http_request_get_method(req))
  {
     case HTTP_OPTIONS:
        res = http_response_create(req, WS_HTTP_200);
        http_response_add_header(res,
              "Access-Control-Allow-Origin",
              "*");
        if (service->on_get != NULL) {
           strcat(methods, "GET");
        }
        if (service->on_delete != NULL) {
           if (strlen(methods) > 0) strcat(methods, ", ");
           strcat(methods, "DELETE");
        }
        if (service->on_post != NULL) {
           if (strlen(methods) > 0) strcat(methods, ", ");
           strcat(methods, "POST");
        }
        if (service->on_put != NULL) {
           if (strlen(methods) > 0) strcat(methods, ", ");
           strcat(methods, "PUT");
        }
        http_response_add_header(res,
              "Access-Control-Allow-Methods", methods);
        // TODO Having so many specified headers here is not really
        // thaaaat good
        http_response_add_header(res,
              "Access-Control-Allow-Headers", "Content-Type, "
              "Cache-Control, Accept, X-Requested-With");
        http_response_sendf(res, "OK");
        http_response_destroy(res);
        return 1;
        break;
     default:
        return 0;
        break;
  }
#endif

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
      return service->on_get(service->srv_data, &lrreq->data, lrreq, chunk, len);

    case HTTP_DELETE:
      return service->on_delete(service->srv_data, &lrreq->data, lrreq, chunk, len);

    case HTTP_POST:
      return service->on_post(service->srv_data, &lrreq->data, lrreq, chunk, len);

    case HTTP_PUT:
      return service->on_put(service->srv_data, &lrreq->data, lrreq, chunk, len);

    default:
      return 1;
  }
}

static int on_cmpl(struct httpws *ins, struct http_request *req, void* ws_ctx, void** req_data)
{
  return on_body(ins, req, ws_ctx, req_data, NULL, 0);
}

static int on_destroy(struct httpws *ins, struct http_request *req, void* ws_ctx, void** req_data)
{
   int rc = 0;
   if (*req_data == NULL) return 0;
   struct lr_request *lrreq = *req_data;
   struct lr_service *service = lrreq->service;

   if (service->on_destroy != NULL)
      rc = service->on_destroy(service->srv_data, &lrreq->data, lrreq);

   lr_request_destroy(lrreq);

   return rc;
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
   ws_set.on_req_hdr_cmpl = on_hdr_cmpl;
   ws_set.on_req_body = on_body;
   ws_set.on_req_cmpl = on_cmpl;
   ws_set.on_req_destroy = on_destroy;

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

/// Stops the server
/**
 *  Stops the server and kills all connections to it. Data that have not been
 *  sent will be lost.
 */
void lr_stop(struct lr *ins)
{
  if(ins)
    httpws_stop(ins->webserver);
}

/// Creates a new service
/**
 *  Meaning of return values of on_get, on_post, on_put, and on_delete:
 *  - Zero: Continue parsing the request.
 *  - Non-zero: Stop parsing the request. Connection will not be stopped, so it
 *    is safe to send message to client afterwards.
 *  Return value of on_destroy is ignored.
 */
int lr_register_service(struct lr *ins,
                         char *url,
                         lr_data_cb on_get,
                         lr_data_cb on_post,
                         lr_data_cb on_put,
                         lr_data_cb on_delete,
                         lr_nodata_cb on_destroy,
                         void *srv_data)
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
   service->on_destroy = on_destroy;
   service->srv_data = srv_data;

   struct trie_iter* iter = trie_insert(ins->trie, url, service);

   if(iter == NULL){
      return 1;
   }

   return 0;
}

void *lr_unregister_service(struct lr *ins, const char *url)
{
   printf("Unregistering service on '%s'\n", url);
   struct lr_service *service = trie_remove(ins->trie, url);
   void *srv_data = service->srv_data;
   free(service);
   return srv_data;
}

void *lr_lookup_service(struct lr *ins, char *url)
{
   struct trie_iter *iter = trie_lookup(ins->trie, url);
   if (!iter) return NULL;
   struct lr_service *srv = trie_value(iter);
   return srv->srv_data;
}


/// Send response to a request
/**
 *  lr_sendf is basically a simple wrapper around lr_send_start,
 *  lr_send_vchunkf, and lr_send_stop, so there is no need to call any of
 *  these. Actually the result is undefined if you do.
 *
 *  After the status, headers and message have been sent it closes the connection.
 */
void lr_sendf(struct lr_request *req,
              enum httpws_http_status_code status,
              map_t *headers, const char *fmt, ...)
{
   //NOTE THAT POINTER SIZES ARE DIFFERENT ON 64BIT
   //printf("%d  %s\n", (int)(req), __func__);
   va_list arg;

   va_start(arg, fmt);
   lr_send_start(req, status, headers);
   lr_send_vchunkf(req, fmt, arg);
   lr_send_stop(req);
   va_end(arg);
}

static void add_header(void *data,
                       const char *key, const char *value)
{
   struct http_response *res = data;
   // TODO Has a return value
   http_response_add_header(res, key, value);
}

/// Creates the response to a request.
/**
 *  For sending chunks only, is not needed for lr_sendf.
 *
 *  Sends the status and headers. Keeps connection open for chunks. The
 *  connection should be closed with lr_send_stop after all chunks have been
 *  sent.
 */
void lr_send_start(struct lr_request *req,
                   enum httpws_http_status_code status,
                   map_t *headers)
{
   //NOTE THAT POINTER SIZES ARE DIFFERENT ON 64BIT
   //printf("%d  %s\n", (int)(req), __func__);
   req->res = http_response_create(req->req, status);
   // TODO Consider headers to add
#ifdef LR_ORIGIN
        http_response_add_header(req->res,
              "Access-Control-Allow-Origin",
              "*");
#endif

    hpd_pair_t *pair;
    MAP_FOREACH(pair, headers) {
        add_header(req->res, pair->k, pair->v);
    }
}

int lr_send_add_cookie(struct lr_request *req,
                       const char *field, const char *value,
                       const char *expires, const char *max_age,
                       const char *domain, const char *path,
                       int secure, int http_only,
                       const char *extension)
{
   return http_response_add_cookie(req->res, field, value, expires,
         max_age, domain, path, secure, http_only, extension);
}

int lr_send_add_cookie_simple(struct lr_request *req,
                              const char *field, const char *value)
{
   return lr_send_add_cookie(req, field, value,
                             NULL, NULL, NULL, NULL, 0,0, NULL);
}

/// Send message in chunks as response to the request
/**
 *  See lr_send_vchunkf for more details.
 */
void lr_send_chunkf(struct lr_request *req, const char *fmt, ...)
{
   //NOTE THAT POINTER SIZES ARE DIFFERENT ON 64BIT
   //printf("%d  %s\n", (int)(req), __func__);
   va_list arg;
   va_start(arg, fmt);
   lr_send_vchunkf(req, fmt, arg);
   va_end(arg);
}

/// Send message in chunks as response to the request
/**
 *  Note that you should call lr_send_start before calling this function and
 *  call lr_send_stop after this function. To send a simple non-chunked
 *  message, have a look at lr_sendf instead.
 *
 *  Keeps the connection open for additional chunks afterwards.
 */
void lr_send_vchunkf(struct lr_request *req, const char *fmt, va_list arg)
{
   //NOTE THAT POINTER SIZES ARE DIFFERENT ON 64BIT
   //printf("%d  %s\n", (int)(req), __func__);
   http_response_vsendf(req->res, fmt, arg);
}

/// Ends the reponse initiated with lr_send_start and closes connection
/**
 *  Only for use with lr_send_start and sending chunks. Users of lr_sendf does
 *  not need to call this.
 *
 *  Does a graceful closure of the connection. That is, the connection will be
 *  close after the remaining data has been sent on it.
 */
void lr_send_stop(struct lr_request *req)
{
   //NOTE THAT POINTER SIZES ARE DIFFERENT ON 64BIT
   //printf("%d  %s\n", (int)(req), __func__);
   if (req->res)
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

map_t *lr_request_get_headers(struct lr_request *req)
{
   return http_request_get_headers(req->req);
}

const char *lr_request_get_header(struct lr_request *req, const char* key)
{
   return http_request_get_header(req->req, key);
}

map_t *lr_request_get_arguments(struct lr_request *req)
{
   return http_request_get_arguments(req->req);
}

const char *lr_request_get_argument(struct lr_request *req, const char* key)
{
   return http_request_get_argument(req->req, key);
}

map_t *lr_request_get_cookies(struct lr_request *req)
{
   return http_request_get_cookies(req->req);
}

const char *lr_request_get_cookie(struct lr_request *req, const char* key)
{
   return http_request_get_cookie(req->req, key);
}

const char *lr_request_get_ip(struct lr_request *req)
{
   return http_request_get_ip(req->req);
}

/// Keep the connection open
/**
 *  Normally each connection is closed after a timeout. This prevents that from
 *  happening by deactivating the timeout.
 */
void lr_request_keep_open(struct lr_request *req)
{
   http_request_keep_open(req->req);
}
