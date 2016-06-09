/*
 * Copyright 2011 Aalborg University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 */

#include "http-webserver.h"
#include "unit_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>
#include <curl/curl.h>
#include <pthread.h> 

struct data {
   int state;
   char *method;
   char *url;
   char *hdr_field;
   int hdr_field_l;
   char *hdr_value;
   int hdr_value_l;
   int hdr_count;
   char *body;
   int _errors;
};

static char *req_data = "Hello world";

static int started = 0;
static struct httpws *ws = NULL;
static struct ev_loop *loop;
static struct ev_async exit_watcher;

static size_t data_from_curl(char *buffer, size_t buffer_size, size_t nmemb, char **userdata)
{
	*userdata = realloc(*userdata, strlen(*userdata) + buffer_size*nmemb + 1);
   char *data = &(*userdata)[strlen(*userdata)];
	memcpy(data, buffer, buffer_size*nmemb);
	data[buffer_size*nmemb] = '\0';

	return buffer_size*nmemb;
}

static size_t data_to_curl(void *ptr, size_t size, size_t nmemb,
                           void *userdata)
{
   size_t *sent = userdata;
   char *data = &req_data[*sent];
   char *buf = ptr;
   size_t copied;

   printf("Data to CURL\n");

   // Copy data
   strncpy(buf, data, size*nmemb);

   // Calculate how much was copied
   if (strlen(data) < size*nmemb) {
      copied = strlen(buf);
   } else {
      copied = size*nmemb;
   }

   *sent += copied;
   
   return copied;
}

// TODO This should take method as parameter as basic_get_test does
static char* simple_get_request(char* url)
{
	CURL *handle = curl_easy_init();
	CURLcode c;
	char* userdata = malloc(sizeof(char));
   userdata[0] = '\0';
   size_t sent = 0;

	if(handle != NULL)
	{
		curl_easy_setopt(handle, CURLOPT_URL, url);
      curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, data_from_curl);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &userdata);
		curl_easy_setopt(handle, CURLOPT_READFUNCTION, data_to_curl);
		curl_easy_setopt(handle, CURLOPT_READDATA, &sent);
      curl_easy_setopt(handle, CURLOPT_INFILESIZE, strlen(req_data));
#ifdef DEBUG
      //curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
#endif
      struct curl_slist *chunk = NULL;
      chunk = curl_slist_append(chunk, "Transfer-Encoding:");
      chunk = curl_slist_append(chunk,
            "Cookie: cookie1=val1; cookie2=val2");
      curl_easy_setopt(handle, CURLOPT_HTTPHEADER, chunk);

		if((c = curl_easy_perform(handle)) != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform failed: %s\n", curl_easy_strerror(c));
			return "";
		}

      curl_slist_free_all(chunk);
		curl_easy_cleanup(handle);
		return userdata;
	}
	else
	{
		perror("Could not initialize curl: ");
		return "";
	}
}

static int basic_get_test(enum http_method method, char *host, char* path)
{
   // Construct URL
   int len = strlen(host) + strlen(path) + 1;
   char *url = malloc(len*sizeof(char));
   strcpy(url, host);
   strcat(url, path);

   // Perform request
   char *res = simple_get_request(url);

   // Parse errors
   int _errors = atoi(res);

   // Construct method + path
   len = strlen(http_method_str(method)) + strlen(path) + 2;
   char *first_line = malloc(len*sizeof(char));
   strcpy(first_line, http_method_str(method));
   strcat(first_line, " ");
   strcat(first_line, path);

   // Construct content-length
   len = snprintf("", 0, "Content-Length: %i\r\n", strlen(req_data));
   char *content_length = malloc(len*sizeof(char));
   snprintf(content_length, len, "Content-Length: %i\r\n",
         strlen(req_data));

   // Check for method + path
   if (strstr(res, first_line) == NULL) {
      ASSERT(1);
      printf("The following bad string was received: %s\n", res);
   }

   // Check for content-length
   if (strstr(res, content_length) == NULL) {
      ASSERT(1);
      printf("The following bad string was received: %s\n", res);
   }

   // Check for body
   if(strstr(res, req_data) == NULL) {
      ASSERT(1);
      printf("The following bad string was received: %s\n", res);
   }
   
   free(url);
   free(first_line);
   free(content_length);
   free(res);
   return _errors;
}

/// Test thread
static int test_thread()
{
	int testresult;
   int ret = 0;
	
   // Init
	curl_global_init(CURL_GLOBAL_ALL);

   // Run test
	printf("Running webserver tests\n");
	testresult = basic_get_test(HTTP_PUT, "http://localhost:8080", "/");

   // Check result
   if (testresult) {
		printf("Test failed\n");
      ret++;
   } else {
      printf("Test succeeded\n");
   }

   // Clean up
	curl_global_cleanup();

	return ret;
}

static char *ncat(char *s1, const char *s2, int s2_len)
{
   int len = s2_len + 1;
   if (s1 != NULL) len += strlen(s1);
   char *str = realloc(s1, len);
   if (s1 == NULL) str[0] = '\0';
   strncat(str, s2, s2_len);
   return str;
}

static int on_req_begin(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data)
{
   struct data *data = malloc(sizeof(struct data));
   *req_data = data;

   data->state = 1;
   data->method = NULL;
   data->url = NULL;
   data->hdr_field = NULL;
   data->hdr_field_l = 0;
   data->hdr_value = NULL;
   data->hdr_value_l = 0;
   data->hdr_count = 0;
   data->body = NULL;
   data->_errors = 0;

   return 0;
}

static int on_req_method(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   struct data *data = *req_data;
   int _errors = 0;

   ASSERT_EQUAL(data->state, 1);
   data->state = (data->state | 2);
   data->method = ncat(data->method, buf, len);

   data->_errors += _errors;
   return 0;
}

static int on_req_url(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   struct data *data = *req_data;
   int _errors = 0;

   ASSERT_EQUAL(data->state, 3);
   data->state = (data->state | 4);
   data->url = ncat(data->url, buf, len);

   data->_errors += _errors;
   return 0;
}

static int on_req_url_cmpl(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data)
{
   struct data *data = *req_data;
   int _errors = 0;

   // Update state
   ASSERT_EQUAL(data->state, 7);
   data->state = (data->state | 8);

   // Check URL
   const char *url = http_request_get_url(req);
   ASSERT_STR_EQUAL(url, data->url);

   data->_errors += _errors;
   return 0;
}

static int on_req_hdr_field(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   struct data *data = *req_data;
   int _errors = 0;

   ASSERT_EQUAL(data->state, 15);
   data->state = (data->state | 16);

   int i;
   char *str;
   data->hdr_field_l += len + 1;
   data->hdr_field = realloc(data->hdr_field,
         data->hdr_field_l*sizeof(char));
   str = data->hdr_field;
   for (i = 0; i < data->hdr_count; i++) {
      str = &str[strlen(str)+1];
   }
   strncpy(str, buf, len);
   str[len] = '\0';

   data->_errors += _errors;
   return 0;
}

static int on_req_hdr_value(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   struct data *data = *req_data;
   int _errors = 0;

   ASSERT_EQUAL(data->state, 31);
   data->state = (data->state & 15);

   int i;
   char *str;
   data->hdr_value_l += len + 1;
   data->hdr_value = realloc(data->hdr_value,
         data->hdr_value_l*sizeof(char));
   str = data->hdr_value;
   for (i = 0; i < data->hdr_count; i++) {
      str = &str[strlen(str)+1];
   }
   strncpy(str, buf, len);
   str[len] = '\0';

   data->hdr_count++;

   data->_errors += _errors;
   return 0;
}

static int on_req_hdr_cmpl(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data)
{
   struct data *data = *req_data;
   int _errors = 0;

   ASSERT_EQUAL(data->state, 15);
   data->state = (data->state | 32);

   int i;
   char *f = data->hdr_field;
   char *v = data->hdr_value;
   for (i = 0; i < data->hdr_count; i++) {
      const char *got = http_request_get_header(req, f);
      ASSERT_STR_EQUAL(v, got);

      f = &f[strlen(f)+1];
      v = &v[strlen(v)+1];
   }

   data->_errors += _errors;
   return 0;
}

static int on_req_body(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   struct data *data = *req_data;
   int _errors = 0;

   ASSERT_EQUAL(data->state, 47);
   data->state = (data->state | 64);
   data->body = ncat(data->body, buf, len);

   data->_errors += _errors;
   return 0;
}

static int construct_body(char *body, int len, struct data *data)
{
   int l = 0, i;
   char *field = data->hdr_field;
   char *value = data->hdr_value;

   l += snprintf(body, len, "%d %s %s\r\n", data->_errors, data->method, data->url);
   for (i = 0; i < data->hdr_count; i++) {
      l += snprintf(&body[l], len, "%s: %s\r\n", field, value);
      field = &field[strlen(field)+1];
      value = &value[strlen(value)+1];
   }
   if (data->body != NULL)
      l += snprintf(&body[l], len, "\r\n%s", data->body);

   return l+1;
}

static int on_req_cmpl(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data)
{
   struct data *data = *req_data;
   int _errors = 0;

   // Check state
   ASSERT_EQUAL((data->state & 47), 47);
   data->state = (data->state | 64);

   // Check method
   enum http_method method = http_request_get_method(req);
   if (strcmp(data->method, http_method_str(method)) != 0) {
      fprintf(stderr, "Got '%s', Expected '%s'\n", data->method,
            http_method_str(method));
      data->_errors++;
   }

   // Check cookie
   const char *cookie;
   cookie = http_request_get_cookie(req, "cookie1");
   ASSERT_STR_EQUAL(cookie, "val1");
   cookie = http_request_get_cookie(req, "cookie2");
   ASSERT_STR_EQUAL(cookie, "val2");

   // Construct body
   data->_errors += _errors;
   char *body = "";
   int body_len = construct_body(body, 0, data);
   body = malloc(body_len*sizeof(char));
   construct_body(body, body_len, data);

   // Send response
   struct http_response *res = http_response_create(req, WS_HTTP_200);
   http_response_sendf(res, body);
   http_response_destroy(res);

   // Clean up
   free(body);
   free(data->method);
   free(data->url);
   free(data->hdr_field);
   free(data->hdr_value);
   free(data->body);
   free(data);
   return 0;
}

/// Handle correct exiting
static void exit_handler(int sig)
{
   fprintf(stderr, "Sending stop signal\n");
   ev_async_send(loop, &exit_watcher);
}

/// Exit callback for async watcher (Webserver)
static void exit_cb(EV_P_ ev_async *watcher, int revents)
{
   fprintf(stderr, "Stopping webserver\n");
   // Stop webserver
   if (ws != NULL) {
      httpws_stop(ws);
      httpws_destroy(ws);
   }

   ev_break(loop, EVBREAK_ALL);
}

/// Webserver thread
static void *webserver_thread(void *arg)
{
   // The event loop for the webserver to run on
   loop = EV_DEFAULT;

   // Add a watcher to stop it again
   ev_async_init(&exit_watcher, exit_cb);
   ev_async_start(loop, &exit_watcher);

   // Settings for the webserver
   struct httpws_settings settings = HTTPWS_SETTINGS_DEFAULT;
   settings.port = HPD_TCPD_P_HTTP_ALT;
   settings.on_req_begin = on_req_begin;
   settings.on_req_method = on_req_method;
   settings.on_req_url = on_req_url;
   settings.on_req_url_cmpl = on_req_url_cmpl;
   settings.on_req_hdr_field = on_req_hdr_field;
   settings.on_req_hdr_value = on_req_hdr_value;
   settings.on_req_hdr_cmpl = on_req_hdr_cmpl;
   settings.on_req_body = on_req_body;
   settings.on_req_cmpl = on_req_cmpl;

   // Inform if we have been built with debug flag
#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Register signals for correctly exiting
   signal(SIGINT, exit_handler);
   signal(SIGTERM, exit_handler);

   // Create webserver
   ws = httpws_create(&settings, loop);
   httpws_start(ws);

   // Start the event loop and webserver
   started = 1;
   ev_run(loop, 0);

   return NULL;
}

/// Main function
int main(int argc, char *argv[])
{
   int stat;
   pthread_t server_thread;

   // Start webserver and wait for it to start
   pthread_create(&server_thread, NULL, webserver_thread, NULL);
   // TODO Someone needs too add a timeout here...
   while (!started);

   // Run test
   stat = test_thread();

   // Stop webserver properly
   exit_handler(0);
   pthread_join(server_thread, NULL);

   // Return result of test
   return stat;
}

