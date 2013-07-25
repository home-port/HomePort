// linkedmap.c

/*  Copyright 2013 Aalborg University. All rights reserved.
 *   
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  
 *  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *  
 *  The views and conclusions contained in the software and
 *  documentation are those of the authors and should not be interpreted
 *  as representing official policies, either expressed.
 */

#include "linkedmap.h"
#include "linked_list.h"
#include <string.h>
#include <stdio.h>

struct lm
{
	struct ll *pairs;
};

struct pair
{
	char* key;
	char* value;
};

// create a new linked map
struct lm *lm_create()
{
	struct lm *ret = malloc(sizeof(struct lm));
	if(ret == NULL)	{
		fprintf(stderr, "Malloc failed when creating new linkedmap\n");
		return NULL;
	}

	ll_create(ret->pairs);
	if(ret->pairs == NULL) {
		fprintf(stderr, "Malloc failed when creating new linkedlist in linkedmap\n");
		return NULL; 
	}

	return ret;
}

// Destroy a linked map. Also deallocates contents
void lm_destroy(struct lm *map)
{
	struct ll_iter *it;

	if(map)	{
		// Loop through and destroy all keys
		for(it = ll_head(map->pairs); it != NULL; it = ll_next(it))	{
			free(((struct pair*)ll_data(it))->key);
			free(((struct pair*)ll_data(it))->value);
			free(((struct pair*)ll_data(it)));
		}

		ll_destroy(map->pairs);
	}

   free(map);
}

int lm_insert_n(struct lm *map, const char* key, size_t key_len,
                                const char* value, size_t value_len)
{
	// Check if the item is already in the list
	if(lm_find(map, key) != NULL)
		return 1;

	char *mKey = malloc((key_len+1)*sizeof(char));
	if(mKey == NULL) {
		fprintf(stderr, "Malloc failed when allocating key for linkedmap\n");
		return 2;
	}

	char *mValue = malloc((value_len+2)*sizeof(char));
	if(mValue == NULL) {
		fprintf(stderr, "Malloc failed when allocating value for linkedmap\n");
      free(mKey);
		return 2;
	}

	struct pair *p = malloc(sizeof(struct pair));
	if(p == NULL) {
		fprintf(stderr, "Malloc failed when allocating pair struct for linkedmap\n");
      free(mValue);
      free(mKey);
		return 2;
	}

	strncpy(mKey, key, key_len);
   mKey[key_len] = '\0';
	strncpy(mValue, value, value_len);
   mValue[value_len] = '\0';

	p->key = mKey;
	p->value = mValue;

	ll_insert(map->pairs, ll_tail(map->pairs), p);

   return 0;
}

// Insert a key/value pair in the map. Key and value will be copied to the map
int lm_insert(struct lm *map, const char* key, const char* value)
{
   return lm_insert_n(map, key, strlen(key), value, strlen(value));
}

// Remove a key and value pair
void lm_remove_n(struct lm *map, const char* key, size_t key_len)
{
	struct ll_iter *it, *next;

	if(map){
		for(it = ll_head(map->pairs); it != NULL;) {
			if(strncmp(((struct pair*)ll_data(it))->key, key, key_len) == 0) {
				free(((struct pair*)ll_data(it))->key);
				free(((struct pair*)ll_data(it))->value);
				free(((struct pair*)ll_data(it)));
            next = ll_next(it);
				ll_remove(it);
            it = next;
			} else {
            it = ll_next(it);
         }
		}
	}
}

// Remove a key and value pair
void lm_remove(struct lm *map, const char* key)
{
   lm_remove_n(map, key, strlen(key));
}

// Get the value of a key in the linked map
char* lm_find_n(struct lm *map, const char* key, size_t key_len)
{
	struct ll_iter *it;

	if(map){
		for(it = ll_head(map->pairs); it != NULL; it = ll_next(it)) {
			if(strncmp(((struct pair*)ll_data(it))->key, key, key_len) == 0) {
				return ((struct pair*)ll_data(it))->value;
			}
		}
	}
	return NULL;
}

// Get the value of a key in the linked map
char* lm_find(struct lm *map, const char* key)
{
   return lm_find_n(map, key, strlen(key));
}

// Map a read-only function over the internal linked list.
void lm_map(struct lm *map, lm_map_cb func, void *data)
{
	if(map && func) {
		struct ll_iter *it;
		for(it = ll_head(map->pairs); it != NULL; it = ll_next(it)) {
			func(data, ((struct pair*)ll_data(it))->key, ((struct pair*)ll_data(it))->value);
		}
	}
}






