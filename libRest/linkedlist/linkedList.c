// linkedList.c

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

#include "linkedList.h"

struct ListElement *create(char* key, void* val)
{
   ListElement *headElement = malloc(sizeof(*headElement));
   if(headElement == NULL)
   {
	   return NULL;	
   }
   headElement->next = NULL;
   headElement->val = val;
   headElement->key = key;

   return headElement;
}

void insert(ListElement *head, char *key, void *val)
{
   ListElement *element = malloc(sizeof(*element));
   ListElement *tempElement = head;

   while(tempElement->next != NULL);

   tempElement->next = element;
   element->next = NULL;
   element->val = val;
   element->key = key;
}

void* get(ListElement *head, char *key)
{
   ListElement *tempElement = head->next;
   while(tempElement != NULL)
   {
      if(tempElement->key == key)
      {
         return tempElement->val;
      }
   }
   return NULL;
}

void destroy(ListElement *head)
{
   while(head->next != NULL)
   {
      //moving head down the list as elements are freed
      ListElement *target = head;
      head = head->next;
      free(target);
   }
   free(head);
}

void removeElement(ListElement *head, char *key)
{  
   ListElement *target = head;
   ListElement *prev = NULL;
   while(target->next != NULL)
   {
      if(target->key == key)
      {
         if(prev != NULL)
            prev->next = target->next;
         free(target);
      }
   }
}
