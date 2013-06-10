#include "../include/linkedlist.h"
#include "unit_test.h"
#include <stdio.h>

TEST_START("linkedlist.c")

TEST(linkedlist)
   LinkedList *linkedlist = create_linkedList();
   insert_listElement(linkedlist, "Hello", NULL);
   insert_listElement(linkedlist, "there", NULL);
   get_value(linkedlist, "there");
   remove_listElement(linkedlist, "there");   
   destroy_linkedList(linkedlist);
TSET()

TEST_END()

