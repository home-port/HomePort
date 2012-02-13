#include "url_trie.h"
#include "utlist.h"

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
    free( copy_url );
    return HPD_E_BAD_PARAMETER;
  }
  while( segment )
  {
    DL_FOREACH( cur_node->children, elt )
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
      DL_APPEND( cur_node->children, new_url_trie_element );
      cur_node = new_url_trie_element;
    }
    segment = strtok( NULL, "/" ); 
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

UrlTrieElement *lookup_for_url_trie_element( UrlTrieElement *head, char *url )
{
  char *segment = NULL, *copy_url = NULL;
  UrlTrieElement *cur_node = head, *elt;
  int found = 0;
 
  if( !head || !url )
    return HPD_E_NULL_POINTER;
  
  copy_url = malloc( sizeof( char ) * strlen( url ) + 1);
  strcpy( copy_url, url );

  segment = strtok( copy_url, "/" );
  if( !segment )
  {
    free( copy_url );
    return HPD_E_BAD_PARAMETER;
  }

  while( segment )
  {
    DL_FOREACH( cur_node->children, elt )
    {
      if( elt->url_segment )
      {
        if( strcmp( elt->url_segment, segment ) == 0 )
        {
          found = 1;
          cur_node = elt;
          break;
        }
      }
    }
    if( !found )
    {
      free( copy_url );
      return NULL;
    }
    segment = strtok( NULL, "/" );
  }

  free( copy_url );
  return cur_node;
}

