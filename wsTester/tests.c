// tests.c

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

#include "tests.h"

void init_tests()
{
	init_libcurl();
}

void cleanup_tests()
{
	cleanup_libcurl();
}

int basic_get_contains_test(char* url, char* contains)
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

int basic_get_multithreaded_stress_test(char* url, char* contains)
{
	int threads = 0;
	int err = 0;
	multithreaded_results = 1;

	struct mt_args *args = malloc(sizeof(struct mt_args));
	args->url = url;
	args->contains = contains;
	args->assaults = 900;

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

void set_bad_multithreaded_result()
{
	pthread_mutex_lock(&lock);
	multithreaded_results = 0;
	pthread_mutex_unlock(&lock);
}

void get_mt_loop(void *args)
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
}
