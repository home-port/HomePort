// libREST_test.c

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
#include "unit_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>
#include <curl/curl.h>
#include <pthread.h> 

static char *req_data = "Hello world";

static int started = 0;
static struct lr *ws = NULL;
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
static char* simple_get_request(char* url, long *res_code)
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
      curl_easy_setopt(handle, CURLOPT_HTTPHEADER, chunk);

		if((c = curl_easy_perform(handle)) != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform failed: %s\n", curl_easy_strerror(c));
			return "";
		}

      curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, res_code);

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

static int basic_get_test(char *host, char* path, long code, char *body)
{
   int _errors = 0;

   // Construct URL
   int len = strlen(host) + strlen(path) + 1;
   char *url = malloc(len*sizeof(char));
   strcpy(url, host);
   strcat(url, path);

   // Perform request
   long res_code = 0;
   char *res = simple_get_request(url, &res_code);
   ASSERT_EQUAL(res_code, code);
   ASSERT_STR_EQUAL(res, body);
  
   free(url);
   free(res);
   return _errors;
}

/// Test thread
static int test_thread()
{
   int ret = 0;
	
   // Init
	curl_global_init(CURL_GLOBAL_ALL);

   // Run test
	printf("Running webserver tests\n");
	ret += basic_get_test("http://localhost:8080", "/",
         404, "Resource not found");
	ret += basic_get_test("http://localhost:8080", "/device",
         405, "Method Not Allowed");
	ret += basic_get_test("http://localhost:8080", "/devices",
         200, "PUT!");

   // Check result
   if (ret) {
		printf("Test failed\n");
   } else {
      printf("Test succeeded\n");
   }

   // Clean up
	curl_global_cleanup();

	return ret;
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
      lr_stop(ws);
      lr_destroy(ws);
   }

   ev_break(loop, EVBREAK_ALL);
}

static int get_cb(struct lr_request *req, const char *body, size_t len)
{
   if (body == NULL) {
      lr_sendf(req, WS_HTTP_200, "GET!");
      lr_request_destroy(req);
   }
   return 0;
}

static int post_cb(struct lr_request *req, const char *body, size_t len)
{
   if (body == NULL) {
      lr_sendf(req, WS_HTTP_200, "POST!");
      lr_request_destroy(req);
   }
   return 0;
}

static int put_cb(struct lr_request *req, const char *body, size_t len)
{
   if (body == NULL) {
      lr_sendf(req, WS_HTTP_200, "PUT!");
      lr_request_destroy(req);
   }
   return 0;
}

static int delete_cb(struct lr_request *req, const char *body, size_t len)
{
   if (body == NULL) {
      lr_sendf(req, WS_HTTP_200, "DELETE!");
      lr_request_destroy(req);
   }
   return 0;
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
   struct lr_settings settings = LR_SETTINGS_DEFAULT;
   settings.port = WS_PORT_HTTP_ALT;

   // Inform if we have been built with debug flag
#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Register signals for correctly exiting
   signal(SIGINT, exit_handler);
   signal(SIGTERM, exit_handler);

   // Create webserver
   ws = lr_create(&settings, loop);
   lr_register_service(ws, "/device", get_cb, NULL, NULL, NULL);
   lr_register_service(ws, "/devices", get_cb, post_cb, put_cb, delete_cb);
   lr_start(ws);

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

