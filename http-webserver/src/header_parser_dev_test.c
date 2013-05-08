#include <stdio.h>
#include "header_parser.h"

void on_field_value(const char* field, size_t field_length, const char* value, size_t value_length)
{
	printf("Key-value: %.*s", (int)field_length, field); 
	printf(" : %.*s \n", (int)value_length, value); 
}

int main()
{
	printf("Header Parser Test\n");

	struct header_parser_settings settings = HEADER_PARSER_SETTINGS_DEFAULT;
	settings.on_field_value_pair = &on_field_value;

	struct header_parser_instance *inst = hp_create(&settings);

	hp_on_header_field(inst, "cat",3);
	hp_on_header_value(inst, "yes",3);

	hp_on_header_field(inst, "dog",3);
	hp_on_header_value(inst, "yes, this is dog",16);

	hp_on_header_complete(inst);

	hp_destroy(inst);

	return 1;
}