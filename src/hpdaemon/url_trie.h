#ifndef URL_TRIE_H
#define URL_TRIE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hpd_error.h"

typedef struct RequestContainer RequestContainer;
typedef struct UrlTrieElement UrlTrieElement;

typedef size_t (*RequestHandler) ( char *buffer, size_t max_buffer_size, RequestContainer *rc, int *http_ret_code );



struct UrlTrieElement
{
  char *url_segment;
  UrlTrieElement *children;
  UrlTrieElement *next;
  RequestHandler get_handler;
  RequestHandler put_handler;
  RequestHandler post_handler;
  RequestHandler delete_handler;
  void *data_ptr;
};



struct RequestContainer
{
  RequestHandler req_handler;
  int argc;
  char **argv;
  char *req_body;
  void *data_ptr;
};

UrlTrieElement* create_url_trie_element(  char *url_segment, void *data_ptr );

int destroy_url_trie_element( UrlTrieElement *ute_to_destroy );

RequestContainer *create_request_container();

int destroy_request_container( RequestContainer *rc_to_destroy );

int register_url( UrlTrieElement *head, char *url, RequestHandler get_handler, RequestHandler put_handler, 
                  RequestHandler post_handler, RequestHandler delete_handler, void *data_ptr );

int lookup_for_url_trie_element( UrlTrieElement *head, char *url, const char* http_method, RequestContainer *rc_out );

int free_argv( int argc, char **argv );

int free_url_trie( UrlTrieElement *head );

#endif

