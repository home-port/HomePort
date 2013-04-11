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

 int main()
 {
 	printf("Its running at least...\n");

 	struct url_parser_settings *settings = malloc(sizeof(struct url_parser_settings));
 	settings->on_begin = &on_begin;
 	settings->on_protocol = &on_protocol;
 	settings->on_host = &on_host;
 	settings->on_port = &on_port;
 	settings->on_path_segment = &on_path_segment;

 	struct url_parser_instance *instance = up_create(settings);

 	char* chunkA = "https://google.com:8080/device/washer";

 	up_add_chunk(instance, chunkA, 37);

 	up_add_chunk(instance,"/kitchen",8);

 	up_complete(instance);

 	//up_add_chunk(instance, "/one",4);

 	up_destroy(instance);

 	printf("Over and out.\n");

 	return 0;
 }