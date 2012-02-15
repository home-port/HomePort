#include "url_trie.h"
#include "utlist.h"

int free_argv( int argc, char ***argv )
{
  int i;
  
  if( !*argv )
    return HPD_E_NULL_POINTER;

  for( i = 0; i < argc; i++ )
  {
    if( (*argv)[i] )
      free( (*argv)[i] );
  }
  
  free( *argv );

  *argv = NULL;

  return 0;
} 

int free_url_trie( UrlTrieElement *head )
{
  UrlTrieElement *iterator = NULL, *to_free;
  if( !head )
    return HPD_E_NULL_POINTER;

  iterator = head->children;
  while( iterator )
  {
    to_free = iterator;
    iterator = iterator->next;
    free_url_trie( to_free );
  }

  destroy_url_trie_element( head );
  
  return 0;
}

UrlTrieElement *create_url_trie_element( char *url_segment, void *data_ptr )
{
  UrlTrieElement *new_url_trie_element = malloc( sizeof( *new_url_trie_element ) );
  if( !new_url_trie_element )
      return NULL;

  if( !url_segment )
    new_url_trie_element->url_segment = NULL;
  else
  {
    new_url_trie_element->url_segment = malloc( sizeof( char ) * ( strlen( url_segment ) + 1 ) );
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

int destroy_url_trie_element( UrlTrieElement *ute_to_destroy )
{
  if( !ute_to_destroy )
    return HPD_E_NULL_POINTER;

  if( ute_to_destroy->url_segment )
    free( ute_to_destroy->url_segment );

  free( ute_to_destroy );

  return 0;
}

RequestContainer *create_request_container()
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

int destroy_request_container( RequestContainer *rc_to_destroy )
{
  if( !rc_to_destroy )
    return HPD_E_NULL_POINTER;

  if( rc_to_destroy->argv )
    free_argv( rc_to_destroy->argc, &rc_to_destroy->argv );

  if( rc_to_destroy->req_body )
    free( rc_to_destroy->req_body );

  return 0;
}

int register_url( UrlTrieElement *head, char *url, RequestHandler get_handler, RequestHandler put_handler,
                  RequestHandler post_handler, RequestHandler delete_handler, void *data_ptr)
{
  char *segment = NULL, *copy_url = NULL;
  int found = 0;
  UrlTrieElement *cur_node = head, *elt;
  if( !url || !head )
    return HPD_E_NULL_POINTER;

  copy_url = malloc( sizeof( char ) * strlen( url ) + 1); 
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
    return -100;
  }

  return 0;

}

int lookup_for_url_trie_element( UrlTrieElement *head, char *url, const char* http_method, RequestContainer **rc_out )
{
  char *segment = NULL, *copy_url = NULL;
  UrlTrieElement *cur_node = head, *elt;
  int found = 0;

  if( !head || !url || !http_method )
    return HPD_E_NULL_POINTER;

  if( *rc_out )
    return HPD_E_BAD_PARAMETER;
  
  *rc_out = create_request_container();

  copy_url = malloc( sizeof( char ) * strlen( url ) + 1);
  strcpy( copy_url, url );

  segment = strtok( copy_url, "/" );
  if( !segment )
  {
    printf("No segment\n");
    free( copy_url );
    destroy_request_container( *rc_out );
    *rc_out = NULL;
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
          (*rc_out)->argc++;
          (*rc_out)->argv = realloc( (*rc_out)->argv, ((*rc_out)->argc) * sizeof(char*) );
          if( !((*rc_out)->argv) )
          {
            free( copy_url );
            destroy_request_container( *rc_out );
            *rc_out = NULL;
            return -1;
          }
          ((*rc_out)->argv)[((*rc_out)->argc)-1] = strdup(segment);
          break;
        }
      }
    }
    if( !found )
    {
      free( copy_url );
      destroy_request_container( *rc_out );
      *rc_out = NULL;
      return -1;
    }
    segment = strtok( NULL, "/" );
    found = 0;
  }

  free( copy_url );

  if( 0 == strcmp( http_method, "GET" ) )
  {
    if( !cur_node->get_handler )
    {
      destroy_request_container( *rc_out );
      *rc_out = NULL;
      return -1;
    }
    else
      (*rc_out)->req_handler = cur_node->get_handler;
  }
  else if( 0 == strcmp( http_method, "PUT" ) )
  {
    if( !cur_node->put_handler )
    {
      destroy_request_container( *rc_out );
      *rc_out = NULL;
      return -1;
    }
    else
      (*rc_out)->req_handler = cur_node->put_handler;
  }
  else if( 0 == strcmp( http_method, "POST" ) )
  {
    if( !cur_node->post_handler )
    {
      destroy_request_container( *rc_out );
      *rc_out = NULL;
      return -1;
    }
    else
      (*rc_out)->req_handler = cur_node->post_handler;
  }
  else if( 0 == strcmp( http_method, "DELETE" ) )
  {
    if( !cur_node->delete_handler )
    {
      destroy_request_container( *rc_out );
      *rc_out = NULL;
      return -1;
    }
    else
      (*rc_out)->req_handler = cur_node->delete_handler;
  }

  (*rc_out)->data_ptr = cur_node->data_ptr;

  return 0;
}

