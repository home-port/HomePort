 #include "url_parser.h"

#include <stdlib.h>
 #include <stdio.h>

void on_begin()
{
	printf("Starting to parse..\n");
}

void on_protocol(const char* protocol, int length)
{
	printf("PCOL: %.*s\n",length,protocol);
}

void on_host(const char* host, int length)
{
	printf("HOST: %.*s\n", length, host);
}

void on_port(const char* port, int length)
{
	printf("PORT: %.*s\n", length, port);
}

void on_path_segment(const char* seg, int length)
{
	printf("SEGM: %.*s\n", length, seg);
}

void on_key_value(const char* key, int key_length, const char* value, int value_length)
{
	printf("PAIR: %.*s %.*s\n",key_length, key, value_length, value);
}

 int main()
 {
 	printf("Its running at least...\n");

 	struct url_parser_settings settings = URL_PARSER_SETTINGS_DEFAULT;

 	settings.on_begin = &on_begin;
 	settings.on_protocol = &on_protocol;
 	settings.on_host = &on_host;
 	settings.on_port = &on_port;
 	settings.on_path_segment = &on_path_segment;
 	settings.on_key_value = &on_key_value;

 	struct url_parser_instance *instance = up_create(&settings);

 	char* chunkA = "http://localhost:8080/device/washer?id=2&name=hej";

 	up_add_chunk(instance, chunkA, 49);

 	up_add_chunk(instance, "&evening=yes",12);

 	up_complete(instance);

 	up_destroy(instance);

 	printf("Over and out.\n");

 	return 0;
 }