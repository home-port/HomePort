// url_parser.c

/*  Copyright 2013 Aalborg University. All rights reserved.
*   
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*  
*  2. Redistributions in binary form must reproduce the above copyright
*  notice, this list of conditions and the following disclaimer in the
*  documentation and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
*  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
*  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
*  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*  
*  The views and conclusions contained in the software and
*  documentation are those of the authors and should not be interpreted
*  as representing official policies, either expressed.
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "url_parser.h"

enum url_parser_state {
	S_START,
	S_PROTOCOL,
	S_GARBAGE1,
	S_GARBAGE2,
	S_HOST,
	S_PREPORT,
	S_PORT,
	S_SEGMENT,
	S_KEY,
	S_VALUE,
	S_ERROR
};

struct url_parser_instance {
	struct url_parser_settings *settings;

	int state;
	unsigned int chars_parsed;
	unsigned int last_returned;
	int tmp_key_begin;
	int tmp_key_size;

	unsigned buffer_size;
	char* buffer;
};

struct url_parser_instance *up_create(struct url_parser_settings *settings)
{
	struct url_parser_instance *instance;

	instance = malloc(sizeof(struct url_parser_instance));

	instance -> settings = malloc(sizeof(struct url_parser_settings));

	if(instance->settings == NULL)
	{
		fprintf(stderr, "Malloc failed in URL parser when allocating space for URL parser settings\n");
		return NULL;
	}

	memcpy(instance->settings, settings, sizeof(struct url_parser_settings));

	instance -> state = S_START;
	instance -> buffer_size = 0;
	instance -> chars_parsed = 0;
	instance -> last_returned = 0;
	instance -> buffer = NULL;

	return instance;
}

void up_destroy(struct url_parser_instance *instance)
{
	if(instance->settings != NULL) {
		free(instance->settings);
	}

	if(instance->buffer != NULL) {
		free(instance->buffer);
	}
	
	free(instance);
}

int isLegalURLChar(char c)
{
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '.' || c == '_' || c == '~' || c == ':' ||
	     c == '/' || c == '?' || c == '#' || c == '[' || c == ']' || c == '@' || c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' ||
	     c == ')' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=')
		return 1;

	return 0;
}

void up_add_chunk(struct url_parser_instance *instance, const char* chunk, int chunk_size) {
	// Increase the current buffer so the chunk can be added
	int old_buffer_size = instance->buffer_size;
	instance->buffer_size += chunk_size;

	instance->buffer = realloc(instance->buffer, instance->buffer_size * (sizeof(char)));
	if(instance->buffer == NULL)
	{
		fprintf(stderr, "Realloc failed in URL parser when allocating space for new URL chunk\n");
		instance->state = S_ERROR;
		return;
	}

	memcpy(instance->buffer+old_buffer_size, chunk, chunk_size);


	// Parse the new chunk in buffer
	unsigned int i;
	for(i = instance->chars_parsed; i < instance->buffer_size; i++)
	{
		char c = instance->buffer[i];

		//printf("current char: %c, state: %d\n", c, instance->state);

		// Check if it is a valid URL char. If not, print an error message and set parser to error state
		if (!isLegalURLChar(c))
		{
			fprintf(stderr, "The URL parser received an invalid character: %c\n", c);
			instance->state = S_ERROR;
		}

		switch(instance->state)
		{
			case S_START:
				if(c == '/')
				{
					instance->state = S_SEGMENT;
				}
				else
				{
					instance->state = S_PROTOCOL;
				}
				break;
			case S_PROTOCOL:
				if(c == ':')
				{
					if(instance->settings->on_protocol != NULL)
					{
						instance->settings->on_protocol(instance->buffer,instance->chars_parsed);
					}
					instance->last_returned = instance->chars_parsed;
					instance->state = S_GARBAGE1;
					break;
				}
				break;
			case S_GARBAGE1:
				if(c == '/')
				{
					instance->state = S_GARBAGE2;
				}
				else
				{
					instance->state = S_ERROR;
				}
				break;
			case S_GARBAGE2:
				if(c == '/')
				{
					instance->state = S_HOST;
				}
				else
				{
					instance->state = S_ERROR;
				}
				break;
			case S_HOST:
				if(c == ':' || c == '/')
				{
					if(instance->settings->on_host != NULL)
					{
						instance->settings->on_host(&instance->buffer[instance->last_returned+3],(i-(instance->last_returned)-3));
					}
					instance->last_returned = instance->chars_parsed;
					instance->state = S_PREPORT;
					break;
				}
				break;

			case S_PREPORT:
				if(c == '/')
					instance->state = S_ERROR;
				instance->state = S_PORT;
				break;

			case S_PORT:
				if(c == '/')
				{
					if(instance->settings->on_port != NULL)
					{
						instance->settings->on_port(&instance->buffer[instance->last_returned+1], (i-(instance->last_returned)-1));
					}
					instance->last_returned = instance->chars_parsed;
					instance -> state = S_SEGMENT;
				}
				break;
			case S_SEGMENT:
				if(c == '/' || c == '?')
				{
					if(instance->settings->on_path_segment != NULL)
					{
						instance->settings->on_path_segment(&instance->buffer[instance->last_returned+1], (i-(instance->last_returned)-1));
					}
					instance->last_returned = instance->chars_parsed;
				}
				if(c == '?')
				{
					instance->state = S_KEY;
				}
				break;
			case S_KEY:
				if(c == '=')
				{
					if(instance->settings->on_key_value != NULL)
					{
						//instance->tmp_key = &instance->buffer[instance->last_returned+1];
						instance->tmp_key_begin = instance->last_returned+1;
						instance->tmp_key_size = (i-(instance->last_returned)-1);
					}
					instance->last_returned = instance->chars_parsed;
					instance->state = S_VALUE;
				}
				break;
			case S_VALUE:
				if(c=='&')
				{
					if(instance->settings->on_key_value != NULL)
					{
						instance->settings->on_key_value(&instance->buffer[instance->tmp_key_begin], instance->tmp_key_size,
														 &instance->buffer[instance->last_returned+1],
														 (i-(instance->last_returned)-1));
					}
					instance->last_returned = instance->chars_parsed;
					instance->state = S_KEY;
				}
				break;

			case S_ERROR:
					fprintf(stderr,"The URL parser has reached an error state\n");
				break;
		}
		instance->chars_parsed++;
	}
}

void up_complete(struct url_parser_instance *instance)
{
	// Check if we need to send a last chunk and that we are in a valid end state
	switch(instance->state)
	{
		case S_SEGMENT:
			if(instance->settings->on_path_segment != NULL)
			{
				instance->settings->on_path_segment(&instance->buffer[instance->last_returned+1],
													(instance->buffer_size-instance->last_returned-1));
			}
			instance->last_returned = instance->chars_parsed;
			break;

		case S_VALUE:
			if(instance->settings->on_key_value != NULL)
			{
				instance->settings->on_key_value(&instance->buffer[instance->tmp_key_begin],
												 instance->tmp_key_size,
												 &instance->buffer[instance->last_returned+1],
												 (instance->buffer_size-(instance->last_returned)-1));
			}
				instance->last_returned = instance->chars_parsed;
			break;

		case S_HOST:
				// It is valid to call complete after a host has been found
			break;
		
		case S_PORT:
				// It is valid to call complete after a port has been found for the host
			break;

		default:
			fprintf(stderr, "An error has happened in the URL parser. End state: %d\n",instance->state);
	}

	if(instance->settings->on_complete != NULL && instance->buffer != NULL) {
		instance->settings->on_complete(instance->buffer, instance->buffer_size);
	}
}