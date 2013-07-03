// load_test.c

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

#include "webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>
#include <curl/curl.h>
#include <pthread.h> 

#define NTHREADS 16

struct mt_args {
    char *url;
    char* contains;
    unsigned long assaults;
};

static int multithreaded_results;
static pthread_mutex_t lock;
static pthread_t threadID[NTHREADS];
static int started = 0;
static struct ws *ws = NULL;

static void init_libcurl()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

static void cleanup_libcurl()
{
	curl_global_cleanup();
}

static size_t data_from_curl(char *buffer, size_t buffer_size, size_t nmemb, char **userdata)
{
	*userdata = malloc(buffer_size*nmemb + 1);
	memcpy(*userdata, buffer, buffer_size*nmemb);
	(*userdata)[buffer_size*nmemb] = '\0';

	return buffer_size*nmemb;
}

static char* simple_get_request(char* url)
{
	CURL *handle = curl_easy_init();
	CURLcode c;
	char* userdata = "";

	if(handle != NULL)
	{
		curl_easy_setopt(handle, CURLOPT_URL, url);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &data_from_curl);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &userdata);

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

static void init_tests()
{
	init_libcurl();
}

static void cleanup_tests()
{
	cleanup_libcurl();
}

static int basic_get_contains_test(char* url, char* contains)
{
	char *received = simple_get_request(url);
	int result = 0;

	if(strstr(received, contains) != NULL)
	{
		result = 1;
	}
	else
	{
		printf("The following bad string was received: %s\n", received);
	}
	free(received);

	return result;
}

static void set_bad_multithreaded_result()
{
	pthread_mutex_lock(&lock);
	multithreaded_results = 0;
	pthread_mutex_unlock(&lock);
}

static void *get_mt_loop(void *args)
{
	struct mt_args *arguments = (struct mt_args *)args;
	unsigned long i;
	for(i = 0; i<arguments->assaults; i++)
	{
		if(basic_get_contains_test(arguments->url,arguments->contains) == 0)
		{
			set_bad_multithreaded_result();
			break;
		}
	}

   return NULL;
}

static int basic_get_multithreaded_stress_test(char* url, char* contains)
{
	int threads = 0;
	int err = 0;
	multithreaded_results = 1;

	struct mt_args *args = malloc(sizeof(struct mt_args));
	args->url = url;
	args->contains = contains;
	args->assaults = 500;

	while(threads < NTHREADS)
    {
        err=pthread_create(&(threadID[threads]), NULL, &get_mt_loop, (void*)args);
        if(err != 0)
            printf("\nError creating thread: %i %i %s",threads, err,strerror(err));
        threads++;
    }

    threads = 0;
    while(threads < NTHREADS)
    {
    	pthread_join((threadID[threads]), NULL);
    	threads++;
    }

	return multithreaded_results;
}

static int test_thread()
{
	int testresult;
   int ret =1;
	init_tests();

	printf("Running webserver tests:\n");

	printf("\tBasic connection test: ");
	testresult = basic_get_contains_test("http://localhost:8080", "Hello");
	if(testresult == 1)
		printf("Success\n");
	else
		printf("Failed\n");
   ret += testresult;

	
	printf("\tBasic multithreaded stress test: ");
	testresult = basic_get_multithreaded_stress_test("http://localhost:8080", "Hello");
	if(testresult == 1)
		printf("Sucess\n");
	else
		printf("Failed\n");
   ret += testresult;
	

	cleanup_tests();
	printf("\nDone.\n");

	return ret;
}

static int on_receive(struct ws *instance, struct ws_conn *conn, void *ctx, void **data,
                      const char *buf, size_t len)
{
   ws_conn_sendf(conn, "HTTP/1.1 200 OK\r\n\r\nHello");
   ws_conn_close(conn);
   return 0;
}

// TODO Move this up to top of file
static struct ev_loop *loop;
static struct ev_async exit_watcher;

// Handle correct exiting
static void exit_handler(int sig)
{
   fprintf(stderr, "Sending stop signal\n");
   ev_async_send(loop, &exit_watcher);
}

static void exit_cb(EV_P_ ev_async *watcher, int revents)
{
   fprintf(stderr, "Stopping webserver\n");
   // Stop webserver
   if (ws != NULL) {
      ws_stop(ws);
      ws_destroy(ws);
   }

   ev_break(loop, EVBREAK_ALL);
}

static void *webserver_thread(void *arg)
{
   // The event loop for the webserver to run on
   loop = EV_DEFAULT;

   // Add a watcher to stop it again
   ev_async_init(&exit_watcher, exit_cb);
   ev_async_start(loop, &exit_watcher);

   // Settings for the webserver
   struct ws_settings settings = WS_SETTINGS_DEFAULT;
   settings.port = WS_PORT_HTTP_ALT;
   settings.on_receive = on_receive;

   // Inform if we have been built with debug flag
#ifdef DEBUG
   printf("Debugging is set\n");
#endif

   // Register signals for correctly exiting
   signal(SIGINT, exit_handler);
   signal(SIGTERM, exit_handler);

   // Create webserver
   ws = ws_create(&settings, loop);
   ws_start(ws);

   // Start the event loop and webserver
   started = 1;
   ev_run(loop, 0);

   return NULL;
}

int main(int argc, char *argv[])
{
   int stat;
   pthread_t server_thread;
   pthread_create(&server_thread, NULL, webserver_thread, NULL);

   // TODO Someone needs too add a timeout here...
   while (!started);

   stat = test_thread();
   exit_handler(0);

   pthread_join(server_thread, NULL);

   if (stat == 0) return 1;
   else return 0;
}
