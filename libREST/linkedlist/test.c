#include "linkedList.h"
#include <stdio.h>

void main()
{
   LinkedList *linkedlist = create();
   printf("created a new linked list\n");
      
   insert(linkedlist, "Hello", NULL);
   insert(linkedlist, "there", NULL);
   printf("added element\n");

   get(linkedlist, "there");
   printf("get\n");

   removeElement(linkedlist, "there");
   printf("remove\n");
   
   destroy(linkedlist);
   printf("destroyed\n");
}

