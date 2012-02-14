#ifndef URL_TRIE_H
#define URL_TRIE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hpd_error.h"

typedef size_t (*RequestHandler) ( char *buffer, size_t max_buffer_size, int argc, char **argv );

typedef struct UrlTrieElement UrlTrieElement;

struct UrlTrieElement
{
  char *url_segment;
  UrlTrieElement *children;
  UrlTrieElement *next;
  RequestHandler get_handler;
  RequestHandler put_handler;
  RequestHandler post_handler;
  RequestHandler delete_handler;
};

UrlTrieElement* create_url_trie_element(  char *url_segment );

int destroy_url_trie_element( UrlTrieElement *to_destroy );

int register_url( UrlTrieElement *head, char *url, RequestHandler get_handler, RequestHandler put_handler, 
                  RequestHandler post_handler, RequestHandler delete_handler);

int lookup_for_url_trie_element( UrlTrieElement *head, char *url, UrlTrieElement **url_out, int *argc, char ***argv );

int free_argv( int argc, char ***argv );

int free_url_trie( UrlTrieElement *head );

#endif

