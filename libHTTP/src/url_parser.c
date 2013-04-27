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

/// The possible states of the URL Parser
enum url_parser_state {
	S_START,	///< The initial state. From here the parser can either go to S_PROTOCOL, S_SEGMENT or S_ERROR
	S_PROTOCOL,	///< In this state, the parser is parsing the protocol
	S_GARBAGE1,	///< Used for ignoring the first slash after protocol
	S_GARBAGE2, ///< Used for ignoring the second slash after protocol
	S_HOST,		///< The parser is parsing the host of an URL
	S_PREPORT,	///< Used for ignoring the : before a port
	S_PORT,		///< The parser is parsing the port of an URL
	S_SEGMENT,	///< The parser is parsing a path segment of an url
	S_KEY,		///< Here the parser is receiving the first part of a key/value pair
	S_VALUE,	///< The parser goes here after receiving a key, and is now expecting a value. It may go back to key if an & is found
	S_ERROR		///< The error state. The parser will go here if the input char is not valid in an URL, or if it received an invalid char at some point
};

/// An URL Parser instance
/**
 *	<h1>Public Interface</h1>
 *	up_create is used to create an instance of this struct.
 *	The instance has a pointer to a struct containing the user-defined callbacks (see documentation for url_parser_settings).
 *	
 *	<h1>Internals</h1>
 *	The instance holds information about the current state, number of chars parsed etc.
 *	The instance has only a single buffer, which is expanded when chunks are added. 
 *	When a callback is called, it receives a pointer to a specific location in this buffer and a length.
 *	This is done so that only a single buffer is allocated. 
*/
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

/// Create URL parser instance
/**
 *	This method creates an URL Parser instance.
 *	It allocates memory for itself and a copy of the settings struct, and copies the settings struct to this new location.
 *	It also sets all values to default.
 *
 *  @param  settings A pointer to a url_parser_settings struct
 *  @return a pointer to the newly created instance
 */
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

/// Destroy URL parser instance
/**
 *	This method destroys an URL Parser instance, including the buffer and settings struct.
 *
 *  @param  instance A pointer to an url_parser_instance to destroy
 */
void up_destroy(struct url_parser_instance *instance)
{
	if(instance->settings != NULL) {
		free(instance->settings);
	}

	if(instance->buffer != NULL) {
		free(instance->buffer);
	}
	
	if(instance != NULL)
		free(instance);
}

/// Check if a given char is valid in an URL
/**
 *	This method checks if a char is valid in an URL. Only specific chars are valid, others have
 *	to be encoded properly. 
 *
 *  @param  c A char to check
 *  @return 1 if the char is valid in an URL, 0 otherwise
 */
int isLegalURLChar(char c)
{
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '.' || c == '_' || c == '~' || c == ':' ||
	     c == '/' || c == '?' || c == '#' || c == '[' || c == ']' || c == '@' || c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' ||
	     c == ')' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=')
		return 1;

	return 0;
}

/// Parse a chunk of an URL
/**
 *	This method receives a chunk of an URL, and calls the appropriate callbacks.
 *	It increases the size of buffer, copies the new chunk to it and then parses each character and changes state accordingly.
 *	A chunk may of course be a complete URL, all of which will be parsed immediately. 
 *
 *  @param  instance A pointer to an URL Parser instance
 *	@param	chunk A pointer to the chunk (non zero terminated)
 *	@param chunk_size The size of the chunk
 */
int up_add_chunk(void *_instance, void *data,
                  const char* chunk, size_t chunk_size) {
   struct url_parser_instance *instance = _instance;

	// Increase the current buffer so the chunk can be added
	int old_buffer_size = instance->buffer_size;
	instance->buffer_size += chunk_size;

	if(instance->buffer == NULL && instance->settings->on_begin != NULL)
	{
		instance->settings->on_begin();
	}

	instance->buffer = realloc(instance->buffer, instance->buffer_size * (sizeof(char)));
	if(instance->buffer == NULL)
	{
		fprintf(stderr, "Realloc failed in URL parser when allocating space for new URL chunk\n");
		instance->state = S_ERROR;
		return 1;
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

/// Informs the parser that the URL is complete
/**
 *	This method will let the parser know that the URL is complete.
 *	This always results in the on_complete callback being called, but it may also
 *	inflict two others: on_path_segment and on_key_value. This is due to the nature
 *	of the URL parser being able to receive in chunks: it can simply now know if
 *	an URL path is complete without a / at the end. It also cannot know if a value part
 *	of a key is done before being told.
 *
 *  @param  instance A pointer to an URL Parser instance
 */
int up_complete(void *_instance, void *data)
{
   struct url_parser_instance *instance = _instance;

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
