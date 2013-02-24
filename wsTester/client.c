#include "client.h"

void init_libcurl()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

size_t data_from_curl(char *buffer, size_t buffer_size, size_t nmemb, void *userdata)
{
	// TODO: Compare data with real data
	printf("Received: %s", buffer);
	return buffer_size*nmemb;
}

char* simple_get_request(char* url)
{
	CURL *handle = curl_easy_init();
	CURLcode c;

	if(handle != NULL)
	{
		curl_easy_setopt(handle, CURLOPT_URL, url);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &data_from_curl);

		if((c = curl_easy_perform(handle)) != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform failed: %s\n", curl_easy_strerror(c));
		}

		curl_easy_cleanup(handle);
		return "abc";
	}
	else
	{
		perror("Could not initialize curl: ");
		return "b";
	}
}