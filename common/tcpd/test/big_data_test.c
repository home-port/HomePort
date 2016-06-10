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

#include "webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>
#include <curl/curl.h>
#include <pthread.h> 

static char *big_data;

static int started = 0;
static hpd_ws_t *ws = NULL;
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
   char *data = &big_data[*sent];
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

static int basic_get_test(char* url)
{
   char *received = simple_get_request(url);
   int result = 0;
   
   if(strstr(received, big_data) != NULL) {
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
	printf("Running tcpd tests\n");
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

/// On connect callback for tcpd
static int on_connect(hpd_ws_t *instance,
                      hpd_ws_conn_t *conn, void *ws_ctx, void **data)
{
   char *body = malloc(sizeof(char));
   body[0] = '\0';
   *data = body;

   return 0;
}

/// On receive callback for tcpd
static int on_receive(hpd_ws_t *instance,
                      hpd_ws_conn_t *conn, void *ctx, void **data,
                      const char *buf, size_t len)
{
   char *body = *data, *new;
   size_t new_len;

   new_len = strlen(body)+len+1;
   new = realloc(body, sizeof(char)*new_len);
   if (new != NULL) {
      body = new;
      strncat(body, buf, len);
      body[new_len-1] = '\0';
      *data = body;
   } else {
      fprintf(stderr, "Reallocation failed\n");
   }

   if (strstr(body, "EOF") != NULL) {
      ws_conn_sendf(conn, "HTTP/1.1 200 OK\r\n\r\n%s", body);
      ws_conn_close(conn);
   }

   return 0;
}

/// On disconnect callback for tcpd
static int on_disconnect(hpd_ws_t *instance,
                         hpd_ws_conn_t *conn, void *ws_ctx, void **data)
{
   free(*data);
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
   fprintf(stderr, "Stopping tcpd\n");
   // Stop tcpd
   if (ws != NULL) {
      ws_stop(ws);
      ws_destroy(ws);
   }

   ev_break(loop, EVBREAK_ALL);
}

/// Webserver thread
static void *webserver_thread(void *arg)
{
   // The event loop for the tcpd to run on
   loop = EV_DEFAULT;

   // Add a watcher to stop it again
   ev_async_init(&exit_watcher, exit_cb);
   ev_async_start(loop, &exit_watcher);

   // Settings for the tcpd
   struct ws_settings settings = WS_SETTINGS_DEFAULT;
   settings.port = HPD_TCPD_P_HTTP_ALT;
   settings.on_connect = on_connect;
   settings.on_receive = on_receive;
   settings.on_disconnect = on_disconnect;

   // Inform if we have been built with debug flag
#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Register signals for correctly exiting
   signal(SIGINT, exit_handler);
   signal(SIGTERM, exit_handler);

   // Create tcpd
   ws = ws_create(&settings, loop);
   ws_start(ws);

   // Start the event loop and tcpd
   started = 1;
   ev_run(loop, 0);

   return NULL;
}

/// Main function
int main(int argc, char *argv[])
{
   int stat;
   pthread_t server_thread;

   unsigned int i, size = 1024*1024;
   big_data = malloc(sizeof(char)*size);
   for (i = 0; i < size; i++) {
      char c = rand() % ('z' - 'a' + 1) + 'a';
      big_data[i] = c;
   }
   big_data[size-4] = 'E';
   big_data[size-3] = 'O';
   big_data[size-2] = 'F';
   big_data[size-1] = '\0';

   // Start tcpd and wait for it to start
   pthread_create(&server_thread, NULL, webserver_thread, NULL);
   // TODO Someone needs too add a timeout here...
   while (!started);

   // Run test
   stat = test_thread();

   // Stop tcpd properly
   exit_handler(0);
   pthread_join(server_thread, NULL);

   // Return result of test
   return stat;
}
