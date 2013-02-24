#include "tests.h"

void init_tests()
{
	init_libcurl();
}

int basic_connection_test(char* url)
{
	char *received = simple_get_request(url);

	printf("%s", received);

	return 1;
}