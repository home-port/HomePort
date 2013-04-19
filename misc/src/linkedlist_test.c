#include "linkedlist.h"
#include "unit_test.h"
#include <stdio.h>

TEST_START("linkedlist.c")

TEST(linkedlist)
   LinkedList *linkedlist = create();
   insert(linkedlist, "Hello", NULL);
   insert(linkedlist, "there", NULL);
   get(linkedlist, "there");
   removeElement(linkedlist, "there");   
   destroy(linkedlist);
TSET()

TEST_END()

