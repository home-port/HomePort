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

UrlTrieElement *create_url_trie_element( char *url_segment )
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
  
  return new_url_trie_element;

}

int destroy_url_trie_element( UrlTrieElement *to_destroy )
{
  if( !to_destroy )
    return HPD_E_NULL_POINTER;

  if( to_destroy->url_segment )
    free( to_destroy->url_segment );

  free( to_destroy );

  return 0;
}


int register_url( UrlTrieElement *head, char *url, RequestHandler get_handler, RequestHandler put_handler,
                  RequestHandler post_handler, RequestHandler delete_handler)
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
      UrlTrieElement *new_url_trie_element = create_url_trie_element( segment );
      if( *segment != '@' ) 
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

int lookup_for_url_trie_element( UrlTrieElement *head, char *url, UrlTrieElement **url_out, int *argc, char ***argv )
{
  char *segment = NULL, *copy_url = NULL;
  UrlTrieElement *cur_node = head, *elt;
  int found = 0, i = 0;
 
  if( !head || !url )
    return HPD_E_NULL_POINTER;

  if( *argv || *url_out || !argc )
    return HPD_E_BAD_PARAMETER;
  
  *argc = 0;

  copy_url = malloc( sizeof( char ) * strlen( url ) + 1);
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
          (*argc)++;
          *argv = realloc( *argv, (*argc) * sizeof(char*) );
          if( !(*argv) )
            return -1;
          (*argv)[(*argc)-1] = strdup(segment);
          break;
        }
      }
    }
    if( !found )
    {
      free( copy_url );
      if( *argv )
      {
        for( i = 0; i < *argc; i++ )
        {
          free( *argv[i] );
        }
        free( *argv );
      }
      return -1;
    }
    segment = strtok( NULL, "/" );
    found = 0;
  }
  free( copy_url );
  *url_out = cur_node;
  return 0;
}

