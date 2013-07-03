// http-webserver_test.c

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

#include "http-webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>
#include <curl/curl.h>
#include <pthread.h> 

static char *req_data = "Hello world";

static int started = 0;
static struct httpws *ws = NULL;
static struct ev_loop *loop;
static struct ev_async exit_watcher;

static size_t data_from_curl(char *buffer, size_t buffer_size, size_t nmemb, char **userdata)
{
   printf("Data from curl\n");
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
#ifdef DEBUG
      //curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
#endif
      struct curl_slist *chunk = NULL;
      chunk = curl_slist_append(chunk, "Transfer-Encoding:");
      curl_easy_setopt(handle, CURLOPT_HTTPHEADER, chunk);

		if((c = curl_easy_perform(handle)) != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform failed: %s\n", curl_easy_strerror(c));
			return "";
		}

		curl_easy_cleanup(handle);
		return userdata;
	}
	else
	{
		perror("Could not initialize curl: ");
		return "";
	}
}

static int basic_get_test(char* url)
{
   char *received = simple_get_request(url);
   int result = 0;
   
   if(strstr(received, req_data) != NULL) {
      result = 1;
   } else {
      printf("The following bad string was received: %s\n", received);
   }
   free(received);
   
   return result;
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
	testresult = basic_get_test("http://localhost:8080");

   // Check result
   if (testresult == 1) {
      printf("Test succeeded\n");
   } else {
		printf("Test failed\n");
      ret++;
   }

   // Clean up
	curl_global_cleanup();

	return ret;
}

static char *catn(char *s1, const char *s2, int s2_len)
{
   int len = strlen(s1) + s2_len + 1;
   char *str = realloc(s1, len);
   strncat(str, s2, s2_len);
   return str;
}

static char *cat(char *s1, const char *s2)
{
   int len = strlen(s1) + strlen(s2) + 1;
   char *str = realloc(s1, len);
   strcat(str, s2);
   return str;
}

static int on_req_begin(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data)
{
   char *res = malloc(sizeof(char));
   res[0] = '\0';
   res = cat(res, "<html><body><dl>");

   *req_data = res;
   return 0;
}

static int on_req_method(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   char *res = *req_data;

   int res_len = strlen(res);
   if (res_len > 5 && strcmp(&res[res_len-5], "</dd>") == 0) {
      res[res_len-5] = '\0';
   } else {
      res = cat(res, "<dt>Method:</dt>");
      res = cat(res, "<dd>");
   }
   res = catn(res, buf, len);
   res = cat(res, "</dd>");

   *req_data = res;
   return 0;
}

static int on_req_url(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   return 0;
}

static int on_req_url_cmpl(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data)
{
   return 0;
}

static int on_req_hdr_field(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   return 0;
}

static int on_req_hdr_value(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   return 0;
}

static int on_req_hdr_cmpl(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data)
{
   return 0;
}

static int on_req_body(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data,
      const char *buf, size_t len)
{
   return 0;
}

static int on_req_cmpl(
      struct httpws *ins, struct http_request *req,
      void *ws_ctx, void **req_data)
{
   char *res_body = *req_data;

   res_body = cat(res_body, "</dl></body></html>");

   struct http_response *res = http_response_create(req, WS_HTTP_200);
   http_response_send(res, res_body);

   free(res_body);
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
   settings.port = WS_PORT_HTTP_ALT;
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

