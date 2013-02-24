#include <stdio.h>
#include "tests.h"

int main()
{
	init_tests();

	printf("Running webserver tests:\n");

	int r = basic_connection_test("http://localhost");
	printf("\tBasic connection test: %i\n",r);

	return 0;
}