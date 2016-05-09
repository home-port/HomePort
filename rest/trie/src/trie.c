// trie.c

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

#include <string.h>
#include "trie.h"

struct trie;
struct trie_iter
{
   struct trie *trie;
   struct trie_iter *parent;
   struct trie_iter *next;
   struct trie_iter *child;
   char *key;
   void *value;
};

struct trie
{
   struct trie_iter *iter;
};

static char *substrn(const char *key, int from, int len)
{
   char *res = malloc((len+1)*sizeof(char));
   strncpy(res, &key[from], len);
   res[len] = '\0';
   return res;
}

static char *substr(const char *key, int from)
{
   return substrn(key, from, strlen(&key[from]));
}

struct trie* trie_create()
{
   struct trie* trie = malloc(sizeof(struct trie));
   if(trie == NULL)
   {
      fprintf(stderr,"malloc failed for creating a trie\n");
      return NULL;
   }
   trie->iter = NULL;

   return trie;
}

static void remove_all(struct trie_iter *iter, dealloc_cb destructor)
{
   if (iter == NULL) return;
   if (iter->child != NULL) remove_all(iter->child, destructor);
   if (iter->next != NULL) remove_all(iter->next, destructor);
   if (destructor != NULL) destructor(iter->value);
   free(iter->key);
   free(iter);
}

void trie_destroy(struct trie *trie, dealloc_cb destructor)
{
   if (trie->iter != NULL) remove_all(trie->iter, destructor);
   free(trie);
}

static int prefix(const char *a, const char *b)
{
   int i;
   const int al = strlen(a);
   const int bl = strlen(b);
   for (i = 0; i < al && i < bl && a[i] == b[i]; i++);
   return i;
}

static struct trie_iter* lookup(struct trie_iter *iter, const char *key,
                                int *iter_ptr, int *key_ptr)
{
   if (iter == NULL) return NULL;

   const int il = strlen(iter->key);
   int match = prefix(iter->key, &key[*key_ptr]);

   if (match == 0) {
      // No match
      if (iter->next) return lookup(iter->next, key, iter_ptr, key_ptr);
      else return iter->parent;
   } else if (match == il) {
      // Full match
      *iter_ptr = il;
      *key_ptr += match;
      if (*key_ptr == strlen(key)) return iter;
      else if (iter->child) return lookup(iter->child, key, iter_ptr, key_ptr);
      else return iter;
   } else {
      // Parial match
      *iter_ptr = match;
      *key_ptr += match;
      return iter;
   }
}

struct trie_iter* trie_insert(struct trie *trie,
                              const char *key, void *value)
{
   int iter_ptr, key_ptr;

   // Invalid cases
   if (key == NULL) return NULL;
   if (value == NULL) return NULL;

   // Construct new entry
   struct trie_iter *new = malloc(sizeof(struct trie_iter));
   new->trie = trie;
   new->parent = NULL;
   new->next = NULL;
   new->child = NULL;
   new->key = NULL;
   new->value = value;

   // The empty trie case
   if (trie->iter == NULL) {
      new->key = substr(key, 0);
      trie->iter = new;
      return new;
   }

   // Lookup key
   iter_ptr = 0;
   key_ptr = 0;
   struct trie_iter *iter = lookup(trie->iter, key, &iter_ptr, &key_ptr);

   // No matching prefix case
   if (iter == NULL) {
      iter = trie->iter;
      while (iter->next != NULL) iter = iter->next;
      new->key = substr(key, 0);
      iter->next = new;
      return new;
   }

   // Get lengths
   const int iter_len = strlen(iter->key);
   const int key_len = strlen(key);

   // Full match - key is already present
   if (iter_ptr == iter_len && key_ptr == key_len) {
      if (iter->value == NULL) {
         iter->value = value;
         free(new);
         return iter;
      } else {
         free(new);
         return NULL;
      }
   }

   // Iter needs to be broken
   if (iter_ptr < iter_len) {
      // New is parent
      if (key_ptr == key_len) {
         // Set keys
         new->key = substr(key, key_len-iter_ptr);
         char *str = iter->key;
         iter->key = substr(str, iter_ptr);
         free(str);

         // Fix pointers
         new->parent = iter->parent;
         new->next = iter->next;
         new->child = iter;
         iter->parent = new;
         iter->next = NULL;
         if (new->parent) {
            if (new->parent->child == iter) {
               new->parent->child = new;
            } else {
               struct trie_iter *i = new->parent->child;
               while (i->next != iter) i = i->next;
               i->next = new;
            }
         } else {
            if (trie->iter == iter) {
               trie->iter = new;
            } else {
               struct trie_iter *i = trie->iter;
               while (i->next != iter) i = i->next;
               i->next = new;
            }
         }
         return new;
      }

      // Create new common parent
      struct trie_iter *parent = malloc(sizeof(struct trie_iter));
      parent->trie = trie;
      parent->parent = iter->parent;
      parent->next = iter->next;
      parent->child = iter;
      parent->key = substrn(iter->key, 0, iter_ptr);
      parent->value = NULL;
      iter->parent = parent;
      iter->next = NULL;
      char *str = iter->key;
      iter->key = substr(str, iter_ptr);
      free(str);
      if (parent->parent) {
         if (parent->parent->child == iter) {
            parent->parent->child = parent;
         } else {
            struct trie_iter *i = parent->parent->child;
            while (i->next != iter) i = i->next;
            i->next = parent;
         }
      } else {
         if (trie->iter == iter) {
            trie->iter = parent;
         } else {
            struct trie_iter *i = trie->iter;
            while (i->next != iter) i = i->next;
            i->next = parent;
         }
      }
      iter = parent;
   }

   // Insert new key
   if (key_ptr < key_len) {
      new->key = substr(key, key_ptr);
      if (iter->child) {
         iter = iter->child;
         while (iter->next) iter = iter->next;
         iter->next = new;
         new->parent = iter->parent;
      } else {
         iter->child = new;
         new->parent = iter;
      }
      return new;
   }

   fprintf(stderr, "Trie could not handle case, this should not happen\n");
   free(new);
   return NULL;
}

static void remove_iter(struct trie_iter *iter)
{
   // TODO Possibilities to combine nodes are not considered
   if (iter == NULL) return;
   if (iter->value != NULL) return;
   if (iter->child != NULL) return;

   if (iter->parent == NULL) {
      if (iter->trie->iter == iter) {
         iter->trie->iter = iter->next;
      } else {
         struct trie_iter *i = iter->trie->iter;
         while (i->next != iter) i = i->next;
         i->next = iter->next;
      }
   } else {
      if (iter->parent->child == iter) {
         iter->parent->child = iter->next;
      } else {
         struct trie_iter *i = iter->parent->child;
         while (i->next != iter) i = i->next;
         i->next = iter->next;
      }
   }

   remove_iter(iter->parent);
   free(iter->key);
   free(iter);
}

void *trie_remove(struct trie* trie, const char *key)
{
   struct trie_iter *iter = trie_lookup(trie, key);

   // Key not found
   if (iter == NULL) return NULL;

   // Key found
   void *value = iter->value;
   iter->value = NULL;
   remove_iter(iter);

   return value;
}

struct trie_iter* trie_lookup(struct trie *trie, const char *key)
{
   int iter_ptr = 0, key_ptr = 0;
   struct trie_iter *iter = lookup(trie->iter, key,
                                   &iter_ptr, &key_ptr);

   if (iter == NULL) return NULL;

   const int iter_len = strlen(iter->key);
   const int key_len = strlen(key);
   
   if (iter_ptr == iter_len && key_ptr == key_len && iter->value)
      return iter;
   else
      return NULL;
}

const char *trie_key(struct trie_iter *iter)
{
   return iter->key;
}

void *trie_value(struct trie_iter *iter)
{
   return iter->value;
}

static void print_iter(struct trie_iter *iter, int depth)
{
   int i;

   for (i = 0; i < depth; i++) printf("  ");
   printf("%s\n", iter->key);
   if (iter->child) print_iter(iter->child, depth+1);
   if (iter->next) print_iter(iter->next, depth);
}

void trie_print(struct trie *trie)
{
   print_iter(trie->iter, 0);
}

