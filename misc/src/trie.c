// tree.c

/*Copyright 2013 Aalborg University. All rights reserved.
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

#include "trie.h"

TrieNode* create_trie()
{
   TrieNode* root = malloc(sizeof(TrieNode));
   if(root == NULL)
   {
      fprintf(stderr,"malloc failed for creating a root of trie");
      return NULL;
   }
   root->value = NULL;
   root->children = NULL;

   return root;
}

void* get_trieNode_value(TrieNode* node)
{
   return node->value;
}

void set_trieNode_value(TrieNode* node, void* value)
{
   node->value = value;
}

TrieNode* insert_trie_key(TrieNode* root, char* key)
{
   if(root->children == NULL)
   {
      LinkedList* list = create_linkedList();
      insert_listElement(list, key, NULL);
      root->children = list;
      return root;
   }
   unsigned int current_pos = 0, tmp_pos = 0;
   TrieNode* treeElement = root;
   ListElement* listElement = treeElement->childrent->head;
   char* listkey = ListElement->key;

   while(listElement!= NULL)
   {
      tmp_pos = 0;
      if(listkey[tmp_pos] == key[current_pos])
      {

      }
      listElement = listElement->next;
      if(listElement != NULL)
         listkey = listElement->key;
   }

   insert_listElememt(treeElement->children,key, node);
   
   TrieNode* newNode = malloc(sizeof(TrieNode));
}


TrieNode* lookup_trieNode(TrieNode* root, char* key)
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
               '\0' && listkey[current_pos] == key[current_pos]){
            current_pos++;
            tmp_pos++;
         }
         if(listkey[tmp_pos] == '\0'&& key[current_pos] == '\0')
         {
            return treeElement;
         }
         else if(listkey[tmp_pos] == '\0')
         {
            treeElement = listElement->node;
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
