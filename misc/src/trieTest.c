#include "trie.h"

int main()
{
   printf("creating tree\n");
   TrieNode* root = create_trie();
   printf("inserting key\n");

   ListElement *element = insert_trie_key(root,"abe\0");
   char *c = "Hello this is abe\0";
   set_listElement_value(element, c);

   ListElement *newnew = insert_trie_key(root, "fisk\0");
   char *c2 = "FiskFisk\0";
   set_listElement_value(newnew, c2);

   ListElement *newAbe = insert_trie_key(root, "abekat\0");
   char *c3 = "this is abekat\0";
   set_listElement_value(newAbe, c3);

   ListElement *inTheTrie = lookup_trieNode(root, "abe\0");
   char *out = (char*)get_listElement_value(inTheTrie);

   printf("Lookup of abe: %s\n",out);

   /*
   ListElement *v = lookup_trieNode(root, "abekat\0");
   if(v == NULL)
      printf("abekat is NOT in the trie\n");
   else
      printf("Houston, we have a problem\n");

   ListElement *newElement = insert_trie_key(root,"abekat");
   */


   ListElement *found = lookup_trieNode(root, "fisk\0");
   char *out2 = (char*)get_listElement_value(found);

   printf("Lookup of fisk: %s\n",out2);

   ListElement *ff = lookup_trieNode(root,"abekat\0");

   if(ff == NULL)
   {
      printf("lookup of abekat returned null :(\n");
   }
   else
   {
   char *o2 = (char*)get_listElement_value(ff);
   printf("Lookup of abekat: %s\n", o2);
}

   // INSERTION OF ABEKAT


   /*
   char* c = "hello\0";
   set_trieNode_value(root, c);
   TrieNode* lol = lookup_trieNode(root, "abe\0");
   char* p = (char*)get_trieNode_value(lol);
   printf("lol: %s\n",p);
   */

   return 0;
}
