#include <stdio.h>
#include "hashtable.h"

struct nlist* table[10];

void main()
{
   install("a", "apple", table);
   install("b", "banana", table);

   char *fruit = lookup("a", table)->defn;
   printf("fruit: %s \nfruit: %s\n", fruit,lookup("b", table)->defn);

   if(lookup("g",table)==NULL)
      printf("no grapes, as wanted\n");
   else
      printf("we found grapes: fault!!\n");
}
