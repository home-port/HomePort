#include "../include/trie.h"

int main()
{
   printf("creating tree\n");
   TrieNode* root = trie_create();
   printf("inserting key\n");


   ListElement *newnew = trie_insert_key(root, "fisk\0");
   char *c2 = "FiskFisk\0";
   set_listElement_value(newnew, c2);

   ListElement *newAbe = trie_insert_key(root, "abekat\0");
   char *c3 = "this is abekat\0";
   set_listElement_value(newAbe, c3);

   ListElement *newAbe2 = trie_insert_key(root, "abenar\0");
   char *c32 = "this is a weird type of monkey\0";
   set_listElement_value(newAbe2, c32);

      ListElement *element = trie_insert_key(root,"abe\0");
   char *c = "Hello this is abe\0";
   set_listElement_value(element, c);

   ListElement *fish = trie_insert_key(root, "fiskekutter\0");
   char *thxForAllTheFish = "poppeye the sailor man!\0";
   set_listElement_value(fish, thxForAllTheFish);
   
   ListElement *aab = trie_insert_key(root, "ab\0");
   char *aab2 = "A football club!\0";
   set_listElement_value(aab, aab2);
   
   ListElement *inTheTrie = trie_lookup_node(root, "abe\0");
   printf("helllllllllllllo\n");
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

   inTheTrie = trie_lookup_node(root, "ab\0");
   out = (char*)get_listElement_value(inTheTrie);
   printf("Lookup of ab: %s\n",out);


   ListElement *found = trie_lookup_node(root, "fisk\0");
   char *out2 = (char*)get_listElement_value(found);

   printf("Lookup of fisk: %s\n",out2);

   ListElement *ff = trie_lookup_node(root,"abekat\0");

   if(ff == NULL)
   {
      printf("lookup of abekat returned null :(\n");
   }
   else
   {
   char *o2 = (char*)get_listElement_value(ff);
   printf("Lookup of abekat: %s\n", o2);
   }

   ListElement * lollol = trie_lookup_node(root,"abenar\0");
   char* out123 = (char*)get_listElement_value(lollol);
   printf("Lookup of abenar: %s\n",out123);

   printf("Whos the strongest man in the world?%s\n",(char*)(get_listElement_value(trie_lookup_node(root,"fiskekutter\0"))));

   printf("removing abekat\n");
   trie_remove_key(root,"abekat\0");
   // INSERTION OF ABEKAT
   trie_destroy(root);

   /*
   char* c = "hello\0";
   set_trieNode_value(root, c);
   TrieNode* lol = lookup_trieNode(root, "abe\0");
   char* p = (char*)get_trieNode_value(lol);
   printf("lol: %s\n",p);
   */

   return 0;
}
