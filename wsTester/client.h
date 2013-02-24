#ifndef CLIENT_H
#define CLIENT_H

#include <curl/curl.h>
#include <stdlib.h>

void init_libcurl();
char* simple_get_request(char* url);

#endif