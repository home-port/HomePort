 #include "url_parser.h"

#include <stdlib.h>
 #include <stdio.h>

void on_begin()
{
	printf("Starting to parse..\n");
}

void on_protocol(char* protocol)
{
	printf("PCOL!\n");
}

 int main()
 {
 	printf("Its running at least...\n");

 	struct url_parser_settings *settings = malloc(sizeof(struct url_parser_settings));
 	settings->on_begin = &on_begin;
 	settings->on_protocol = &on_protocol;

 	struct url_parser_instance *instance = up_create(settings);

 	char* chunkA = "http://localhost:8080/device/washer";

 	up_add_chunk(instance, chunkA, 35);

 	//up_add_chunk(instance, "/one",4);

 	up_destroy(instance);

 	printf("Over and out.\n");

 	return 0;
 }