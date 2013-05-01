#include "trie.h"

int main()
{
   printf("creating tree\n");
   TrieNode* root = create_trie();
   printf("inserting key\n");


   ListElement *newnew = insert_trie_key(root, "fisk\0");
   char *c2 = "FiskFisk\0";
   set_listElement_value(newnew, c2);

   ListElement *newAbe = insert_trie_key(root, "abekat\0");
   char *c3 = "this is abekat\0";
   set_listElement_value(newAbe, c3);

   ListElement *newAbe2 = insert_trie_key(root, "abenar\0");
   char *c32 = "this is a weird type of monkey\0";
   set_listElement_value(newAbe2, c32);

      ListElement *element = insert_trie_key(root,"abe\0");
   char *c = "Hello this is abe\0";
   set_listElement_value(element, c);

   ListElement *fish = insert_trie_key(root, "fiskekutter\0");
   char *thxForAllTheFish = "poppeye the sailor man!\0";
   set_listElement_value(fish, thxForAllTheFish);

   ListElement *aab = insert_trie_key(root, "ab\0");
   char *aab2 = "A football club!\0";
   set_listElement_value(aab, aab2);

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

   inTheTrie = lookup_trieNode(root, "ab\0");
   out = (char*)get_listElement_value(inTheTrie);
   printf("Lookup of ab: %s\n",out);


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

   ListElement * lollol = lookup_trieNode(root,"abenar\0");
   char* out123 = (char*)get_listElement_value(lollol);
   printf("Lookup of abenar: %s\n",out123);

   printf("Whos the strongest man in the world? %s\n",(char*)(get_listElement_value(lookup_trieNode(root,"fiskekutter\0"))));

   printf("removing abekat\n");
   remove_trie_key(root,"abekat\0");
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
