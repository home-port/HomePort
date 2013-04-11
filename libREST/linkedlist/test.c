#include "linkedList.h"
#include <stdio.h>

void main()
{
   ListElement *head = create("Hello", NULL);
   printf("created a new linked list\n");
      
   insert(head, "there", NULL);
   printf("added element\n");

   get(head, "there");
   printf("get\n");

   removeElement(head, "Hello");
   printf("remove\n");

   destroy(head);
   printf("destroyed\n");

}

