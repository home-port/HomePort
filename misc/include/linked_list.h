// linked_list.h

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

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <stdio.h>

struct ll;
struct ll_iter {
   struct ll *list;
   struct ll_iter *next;
   struct ll_iter *prev;
   void *data;
};

struct ll {
   struct ll_iter *head;
   struct ll_iter *last; // Last inserted element
   struct ll_iter *tail;
};

#define ll_create(LIST) \
   { \
      LIST = malloc(sizeof(struct ll)); \
      if (LIST) { \
         LIST->head = NULL; \
         LIST->tail = NULL; \
      } \
   }

#define ll_destroy(LIST) \
   { \
      if (LIST->head != NULL) { \
         while (LIST->head->next != NULL) { \
            LIST->head = LIST->head->next; \
            free(LIST->head->prev); \
         } \
         free(LIST->head); \
      } \
      free(LIST); \
   }

#define ll_insert(LIST, ITER, DATA) \
   do { \
      LIST->last = malloc(sizeof(struct ll_iter)); \
      if (!LIST->last) break; \
      LIST->last->list = LIST; \
      LIST->last->data = DATA; \
      LIST->last->prev = ITER; \
      if (ITER) { \
         if (ITER->next) \
            ITER->next->prev = LIST->last; \
         LIST->last->next = ITER->next; \
         ITER->next = LIST->last; \
         if (ITER == LIST->tail) \
            LIST->tail = LIST->last; \
      } else { \
         LIST->last->next = LIST->head; \
         LIST->head = LIST->last; \
         if (!LIST->tail) \
            LIST->tail = LIST->last; \
      } \
      if (LIST->last->next) \
         LIST->last->next->prev = LIST->last; \
   } while (0);


#define ll_remove(ITER) \
   { \
      if (ITER->prev != NULL) \
         ITER->prev->next = ITER->next; \
      if (ITER->next != NULL) \
         ITER->next->prev = ITER->prev; \
      if (ITER->list->head == ITER) \
         ITER->list->head = ITER->next; \
      if (ITER->list->tail == ITER) \
         ITER->list->tail = ITER->prev; \
      free(ITER); \
   }

#define ll_head(LIST) LIST->head
#define ll_tail(LIST) LIST->tail
#define ll_next(ITER) ITER->next
#define ll_prev(ITER) ITER->prev
#define ll_data(ITER) ITER->data

#define ll_find(ITER, LIST, DATA) \
   { \
      for (ITER = ll_head(LIST); \
           ITER != NULL && ll_data(ITER) != DATA; \
           ITER = ll_next(ITER) ); \
   }

#endif









