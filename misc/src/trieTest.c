#include "trie.h"

void main()
{
   printf("creating tree\n");
   TrieNode* root = create_trie();
   printf("inserting key\n");
   root=insert_trie_key(root,"abe\0");
   //root=insert_trie_key(root,"abekat");
   char* c = "hello\0";
   set_trieNode_value(root, c);
   TrieNode* lol = lookup_trieNode(root, "abe\0");
   char* p = (char*)get_trieNode_value(lol);
   printf("lol: %s\n",p);
}
