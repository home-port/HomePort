#include <assert.h>
#include "url_trie.h"
#include "utlist.h"
#include "hpd_error.h"

int 
free_argv( int argc, char **argv )
{
  int i;
  
  if( !argv )
    return HPD_E_NULL_POINTER;

  for( i = 0; i < argc; i++ )
  {
    if( argv[i] )
      free( argv[i] );
  }
  
  free( argv );

  argv = NULL;

  argc = 0;

  return HPD_E_SUCCESS;
} 

int
free_url_trie( UrlTrieElement *head )
{
  UrlTrieElement *iterator = NULL, *to_free;

  assert( head );

  iterator = head->children;
  while( iterator )
  {
    to_free = iterator;
    iterator = iterator->next;
    free_url_trie( to_free );
  }

  destroy_url_trie_element( head );
  
  return HPD_E_SUCCESS;
}

UrlTrieElement *
create_url_trie_element( char *url_segment, void *data_ptr )
{
  UrlTrieElement *new_url_trie_element = malloc( sizeof( *new_url_trie_element ) );
  if( !new_url_trie_element )
      return NULL;

  if( !url_segment )
    new_url_trie_element->url_segment = NULL;
  else
  {
    new_url_trie_element->url_segment = malloc( sizeof( char ) * ( strlen( url_segment ) + 1 ) );
    if( !new_url_trie_element->url_segment )
    {
      free( new_url_trie_element );
      return NULL;
    }
    strcpy( new_url_trie_element->url_segment, url_segment );
  }
  
  new_url_trie_element->children = NULL;

  new_url_trie_element->get_handler = NULL;
  new_url_trie_element->put_handler = NULL;
  new_url_trie_element->post_handler = NULL;
  new_url_trie_element->delete_handler = NULL;

  new_url_trie_element->data_ptr = data_ptr;
  
  return new_url_trie_element;

}

int 
destroy_url_trie_element( UrlTrieElement *ute_to_destroy )
{
  assert( ute_to_destroy );

  if( ute_to_destroy->url_segment )
    free( ute_to_destroy->url_segment );

  free( ute_to_destroy );

  return HPD_E_SUCCESS;
}

RequestContainer *
create_request_container()
{
  RequestContainer *new_request_container = malloc( sizeof( *new_request_container ) );
  if( !new_request_container )
    return NULL;

  new_request_container->req_handler = NULL;
  new_request_container->argc = 0;
  new_request_container->argv = NULL;
  new_request_container->req_body = NULL;

  return new_request_container;

}

int 
destroy_request_container( RequestContainer *rc_to_destroy )
{
  assert( rc_to_destroy );

  if( rc_to_destroy->argv )
    free_argv( rc_to_destroy->argc, rc_to_destroy->argv );

  if( rc_to_destroy->req_body )
    free( rc_to_destroy->req_body );

  free( rc_to_destroy );

  return HPD_E_SUCCESS;
}

int 
register_url( UrlTrieElement *head, char *url, RequestHandler get_handler, RequestHandler put_handler,
                  RequestHandler post_handler, RequestHandler delete_handler, void *data_ptr)
{
  char *segment = NULL, *copy_url = NULL;
  int found = 0;
  UrlTrieElement *cur_node = head, *elt;

  assert( url && head ); 

  copy_url = malloc( sizeof( char ) * strlen( url ) + 1); 
  if( !copy_url )
    return HPD_E_MALLOC_ERROR;
  strcpy( copy_url, url );

  segment = strtok( copy_url, "/" );
  if( !segment )
  {
    printf("No segment found\n");
    free( copy_url );
    return HPD_E_BAD_PARAMETER;
  }
  while( segment )
  {
    LL_FOREACH( cur_node->children, elt )
    {
      if( elt->url_segment )
      {
        if( strcmp( elt->url_segment, segment ) == 0 )
        {
          cur_node = elt;
          found = 1;
          break;
        }
      }
    }
    if( !found )
    {
      UrlTrieElement *new_url_trie_element = create_url_trie_element( segment, data_ptr );
      if( strcmp( segment, "@" ) ) 
        LL_PREPEND( cur_node->children, new_url_trie_element );
      else
        LL_APPEND( cur_node->children, new_url_trie_element );
      cur_node = new_url_trie_element;
    }
    segment = strtok( NULL, "/" ); 
	found = 0;
  }

  free( copy_url );

  if( !cur_node->get_handler && !cur_node->put_handler && !cur_node->post_handler && !cur_node->delete_handler )
  {
    cur_node->get_handler = get_handler;
    cur_node->put_handler = put_handler;
    cur_node->post_handler = post_handler;
    cur_node->delete_handler = delete_handler;
  }
  else
  {
    return HPD_E_URL_ALREADY_REGISTERED;
  }

  return HPD_E_SUCCESS;

}

int 
lookup_url( UrlTrieElement *head, const char *url, const char* http_method, RequestContainer *rc_out )
{
  char *segment = NULL, *copy_url = NULL;
  UrlTrieElement *cur_node = head, *elt;
  int found = 0;

  assert( head && url && http_method && rc_out );

  copy_url = malloc( sizeof( char ) * strlen( url ) + 1);
  if( !copy_url )
    return HPD_E_MALLOC_ERROR;
  strcpy( copy_url, url );

  segment = strtok( copy_url, "/" );
  if( !segment )
  {
    printf("No segment\n");
    free( copy_url );
    return HPD_E_BAD_PARAMETER;
  }

  while( segment )
  {
    LL_FOREACH( cur_node->children, elt )
    {
      if( elt->url_segment )
      {
        if( strcmp( elt->url_segment, segment ) == 0 )
        {
          found = 1;
          cur_node = elt;
          break;
        }
        else if( strcmp( elt->url_segment, "@" ) == 0 )
        {
          found = 1;
          cur_node = elt;
          rc_out->argc++;
          rc_out->argv = realloc( rc_out->argv, (rc_out->argc) * sizeof(char*) );
          if( !(rc_out->argv) )
          {
            free( copy_url );
            return -1;
          }
          (rc_out->argv)[(rc_out->argc)-1] = strdup(segment);
          break;
        }
      }
    }
    if( !found )
    { 
      free( copy_url );
      return HPD_E_URL_NOT_FOUND;
    }
    segment = strtok( NULL, "/" );
    found = 0;
  }

  free( copy_url );

  if( 0 == strcmp( http_method, "GET" ) )
  {
    if( !cur_node->get_handler )
    {
      return HPD_E_URL_METHOD_NOT_IMPLEMENTED;
    }
    else
      rc_out->req_handler = cur_node->get_handler;
  }
  else if( 0 == strcmp( http_method, "PUT" ) )
  {
    if( !cur_node->put_handler )
    {
      return HPD_E_URL_METHOD_NOT_IMPLEMENTED;
    }
    else
      rc_out->req_handler = cur_node->put_handler;
  }
  else if( 0 == strcmp( http_method, "POST" ) )
  {
    if( !cur_node->post_handler )
    {
      return HPD_E_URL_METHOD_NOT_IMPLEMENTED;
    }
    else
      rc_out->req_handler = cur_node->post_handler;
  }
  else if( 0 == strcmp( http_method, "DELETE" ) )
  {
    if( !cur_node->delete_handler )
    {
      return HPD_E_URL_METHOD_NOT_IMPLEMENTED;
    }
    else
      rc_out->req_handler = cur_node->delete_handler;
  }

  rc_out->data_ptr = cur_node->data_ptr;

  return HPD_E_SUCCESS;
}

