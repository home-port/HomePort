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

char* substring_from(char* key, int from)
{
   char *partialKey = malloc(strlen(key)+1-from);
   strcpy(partialKey,&key[from]);
   return partialKey;
}

char* substring_from_to(char* key, int from, int to)
{
   char *partialKey = malloc(strlen(key)+1-(to-from));
   strncpy(partialKey, &key[from], (to-from));
   partialKey[(to-from)+1] = '\0';
   return partialKey;
}

TrieNode* create_trie()
{
   TrieNode* root = malloc(sizeof(TrieNode));
   if(root == NULL)
   {
      fprintf(stderr,"malloc failed for creating a root of trie\n");
      return NULL;
   }
   root->children = NULL;

   return root;
}

ListElement* insert_trie_key(TrieNode* root, char* key)
{
   printf("Inserting %s\n",key);

   key = substring_from(key, 0); // allocate our own copy of the key
   if(root->children == NULL)
   {
      LinkedList* list = create_linkedList();
      ListElement* element = insert_listElement(list, key, NULL);
      root->children = list;
      return element;
   }

   unsigned int current_pos = 0, tmp_pos = 0;
   TrieNode* treeElement = root;
   ListElement* listElement = treeElement->children->head;
   char* listkey = listElement->key;

   while(listElement!= NULL)
   {
      tmp_pos = 0;
      if(listkey[tmp_pos] == key[current_pos])
      {
         tmp_pos++;
         current_pos++;

         while(listkey[tmp_pos] != '\0'&& key[current_pos]!=
               '\0' && listkey[current_pos] == key[current_pos]){
            current_pos++;
            tmp_pos++;
         }

         if(listkey[tmp_pos] == '\0'&& key[current_pos] == '\0')
         {
            if(listElement->value == NULL)
            {
               return  listElement;
            }
            else
            {
               // Element is already in trie
               return NULL;
            }
         }
         else if(listkey[tmp_pos] == '\0')
         {
            if(listElement->node == NULL)
            {
               TrieNode *node = malloc(sizeof(TrieNode));
               if(node == NULL)
               {
                  fprintf(stderr, "malloc failed when creating a trie node\n");
                  return NULL;
               }

               node->children = create_linkedList();
               ListElement *element = insert_listElement(node->children, substring_from(key, current_pos), NULL);

               listElement->node = node;

               return element;
            }
            else
            {
               ListElement *element = insert_listElement(listElement->node->children, substring_from(key, current_pos), NULL);
               return element;
            }
         }
         else if (key[current_pos] == '\0')
         {
            char *matchingPart = substring_from_to(listkey,0, tmp_pos);
            char *nonMatchingPart = substring_from_to(listkey, tmp_pos, strlen(listkey));

            TrieNode *node = malloc(sizeof(TrieNode));
            if(node == NULL)
            {
               fprintf(stderr, "malloc failed when creating a trie node\n");
               return NULL;
            }

            node->children = create_linkedList();
            ListElement *element = insert_listElement(node->children, nonMatchingPart, listElement->node);
            element->value = listElement->value;

            listElement->key = matchingPart;
            listElement->value = NULL;
            listElement->node = node;

            return listElement;
         }
         else //Neither the key or listkey is done, they simply differ
         {
            char *matchingPart = substring_from_to(listkey,0, tmp_pos);
            char *nonMatchingPart = substring_from_to(listkey, tmp_pos, strlen(listkey));

            TrieNode *node = malloc(sizeof(TrieNode));
            if(node == NULL)
            {
               fprintf(stderr, "malloc failed when creating a trie node\n");
               return NULL;
            }

            node->children = create_linkedList();
            ListElement *element = insert_listElement(node->children, nonMatchingPart, listElement->node);
            element->value = listElement->value;

            listElement->value = NULL;
            listElement->key = matchingPart;
            listElement->node = node;

            element = insert_listElement(node->children, substring_from_to(key, current_pos, strlen(key)),NULL);

            return element;
         }

      }
      listElement = listElement->next;
      if(listElement != NULL)
         listkey = listElement->key;
   }

   if(treeElement->children != NULL)
   {
      ListElement *element = insert_listElement(treeElement->children, substring_from(key, current_pos), NULL);
      return element;
   }

   //insert_listElement(treeElement->children,key, NULL);
   //TrieNode* newNode = malloc(sizeof(TrieNode));
}

ListElement* lookup_trieNode(TrieNode* root, char* key)
{
   if(root->children == NULL)
      return NULL;

   unsigned int current_pos = 0, tmp_pos = 0;
   TrieNode* treeElement = root;
   ListElement* listElement = root->children->head;
   char* listkey = listElement->key;
   
   while(listElement!=NULL)
   {
      tmp_pos = 0;

      if(listkey[tmp_pos] == key[current_pos])
      {
         current_pos++;
         tmp_pos++;
         while(listkey[tmp_pos] != '\0'&& key[current_pos]!=
               '\0' && listkey[tmp_pos] == key[current_pos]){
            current_pos++;
            tmp_pos++;

         }
         if(listkey[tmp_pos] == '\0'&& key[current_pos] == '\0')
         {
            return listElement;
         }
         else if(listkey[tmp_pos] == '\0')
         {
            treeElement = listElement->node;
            if(treeElement == NULL || treeElement->children == NULL)
               return NULL;

            listElement = treeElement->children->head;
            listkey = listElement->key;
            continue;
         }
         else 
         {
            return NULL;
         }
      }
      listElement = listElement->next;
      if(listElement != NULL)
         listkey = listElement->key;
   }
   return NULL;
}

void remove_trie_key(TrieNode* root, char* key)
{
   if(root->children == NULL)
      return;
         
   unsigned int current_pos = 0, tmp_pos = 0;
   TrieNode* treeElement = root;
   ListElement *parent = NULL, *listElement = root->children->head;
   char* listkey = listElement->key;
   
   while(listElement!=NULL)
   {
      tmp_pos = 0;

      if(listkey[tmp_pos] == key[current_pos])
      {
         current_pos++;
         tmp_pos++;
         while(listkey[tmp_pos] != '\0'&& key[current_pos]!=
               '\0' && listkey[tmp_pos] == key[current_pos]){
            current_pos++;
            tmp_pos++;

         }
         if(listkey[tmp_pos] == '\0'&& key[current_pos] == '\0')
         {
            //case: key is a subset of another key
            if(listkey->node != NULL)
            {
               //key has only one child, colaps
               if(listkey->node->children->next != NULL)
               {
                  listkey->value = listkey->node->children->head->value;
                  lsitkey->key = strcat()

               }
            }
            remove_listElement(treeElement->children,listkey);
            printf("colaps trie\n");
            // case: only one child left and parent is not end of
            // another key
            if(treeElement->children->head->next == NULL && parent->value != NULL)
            {
               parent->key == strcat(parent->key,
                     treeElement->children->head->key);

            }
            return; 
         }
         else if(listkey[tmp_pos] == '\0')
         {
            treeElement = listElement->node;
            if(treeElement == NULL || treeElement->children == NULL)
               return;

            listElement = treeElement->children->head;
            listkey = listElement->key;
            continue;
         }
         else 
         {
            return;
         }
      }
      parent = listElement;
      listElement = listElement->next;
      if(listElement != NULL)
         listkey = listElement->key;
   }
   return;     
}







