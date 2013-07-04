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
enum up_state {
   S_START,    ///< The initial state. From here the parser can either go to S_PROTOCOL, S_SEGMENT or S_ERROR
   S_PROTOCOL, ///< In this state, the parser is parsing the protocol
   S_SLASH1,   ///< Used for ignoring the first slash after protocol
   S_SLASH2,   ///< Used for ignoring the second slash after protocol
   S_HOST,     ///< The parser is parsing the host of an URL
   S_PREPORT,  ///< Used for ignoring the : before a port
   S_PORT,     ///< The parser is parsing the port of an URL
   S_SEGMENT,  ///< The parser is parsing a path segment of an url
   S_KEY,      ///< Here the parser is receiving the first part of a key/value pair
   S_VALUE,    ///< The parser goes here after receiving a key, and is now expecting a value. It may go back to key if an & is found
   S_ERROR     ///< The error state. The parser will go here if the input char is not valid in an URL, or if it received an invalid char at some point
};

/// An URL Parser instance
/**
 *   <h1>Public Interface</h1>
 *   up_create is used to create an instance of this struct.
 *   The instance has a pointer to a struct containing the user-defined
 *   callbacks (see documentation for url_parser_settings).
 *   
 *   <h1>Internals</h1>
 *   The instance holds information about the current state, number of
 *   chars parsed etc.  The instance has only a single buffer, which is
 *   expanded when chunks are added.  When a callback is called, it
 *   receives a pointer to a specific location in this buffer and a
 *   length.  This is done so that only a single buffer is allocated. 
*/
struct up {
   struct up_settings *settings;
   void *data;

   int state;

   char *buffer;
   size_t protocol;
   size_t protocol_l;
   size_t host;
   size_t host_l;
   size_t port;
   size_t port_l;
   size_t path;
   size_t path_l;
   size_t key_value;
   size_t end;
   size_t last_key;
   size_t last_key_l;
   size_t last_value;
   size_t last_value_l;
   size_t last_path;
   size_t last_path_l;
   size_t parser;
   size_t insert;

   //unsigned int chars_parsed;
   //unsigned int last_returned;
   //int path_start;
   //size_t tmp_key_begin;
   //size_t tmp_key_size;
};

/// Create URL parser instance
/**
 *   This method creates an URL Parser instance.  It allocates memory
 *   for itself and a copy of the settings struct, and copies the
 *   settings struct to this new location.  It also sets all values to
 *   default.
 *
 *  @param  settings A pointer to a url_parser_settings struct
 *  @return a pointer to the newly created instance
 */
struct up *up_create(
      struct up_settings *settings, void *data)
{
   struct up *instance = malloc(sizeof(struct up));

   // Store settings
   instance->settings = malloc(sizeof(struct up_settings));
   if(instance->settings == NULL)
   {
      fprintf(stderr, "Malloc failed in URL parser when allocating "
                      "space for URL parser settings\n");
      return NULL;
   }
   memcpy(instance->settings, settings, sizeof(struct up_settings));

   // Set state
   instance->state = S_START;
   instance->buffer = NULL;

   // Set pointers
   instance->protocol = 0;
   instance->protocol_l = 0;
   instance->host = 0;
   instance->host_l = 0;
   instance->port = 0;
   instance->port_l = 0;
   instance->path = 0;
   instance->path_l = 0;
   instance->key_value = 0;
   instance->end = 0;
   instance->last_key = 0;
   instance->last_key_l = 0;
   instance->last_value = 0;
   instance->last_value_l = 0;
   instance->last_path = 0;
   instance->last_path_l = 0;
   instance->parser = 0;
   instance->insert = 0;

   //instance->chars_parsed = 0;
   //instance->last_returned = 0;
   //instance->path_start = -1;

   // Store data
   instance->data = data;

   return instance;
}

/// Destroy URL parser instance
/**
 *  This method destroys an URL Parser instance, including the buffer
 *  and settings struct.
 *
 *  @param  instance A pointer to an url_parser_instance to destroy
 */
void up_destroy(struct up *instance)
{
   if(instance != NULL) {

      if(instance->settings != NULL) {
         free(instance->settings);
      }

      if(instance->buffer != NULL) {
         free(instance->buffer);
      }
      
      free(instance);
   }
}

/// Check if a given char is valid in an URL
/**
 *   This method checks if a char is valid in an URL. Only specific chars
 *   are valid, others have to be encoded properly. 
 *
 *  @param  c A char to check
 *  @return 1 if the char is valid in an URL, 0 otherwise
 */
int isLegalURLChar(char c)
{
   if ((c >= '0' && c <= '9') || 
       (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
       c == '-' || c == '.' || c == '_' || c == '~' || c == ':' ||
       c == '/' || c == '?' || c == '#' || c == '[' || c == ']' ||
       c == '@' || c == '!' || c == '$' || c == '&' || c == '\'' ||
       c == '(' || c == ')' || c == '*' || c == '+' || c == ',' ||
       c == ';' || c == '=')
      return 1;

   return 0;
}

/// Parse a chunk of an URL
/**
 *   This method receives a chunk of an URL, and calls the appropriate
 *   callbacks.  It increases the size of buffer, copies the new chunk to
 *   it and then parses each character and changes state accordingly.  A
 *   chunk may of course be a complete URL, all of which will be parsed
 *   immediately. 
 *
 *  @param  instance A pointer to an URL Parser instance
 *   @param   chunk A pointer to the chunk (non zero terminated)
 *   @param chunk_size The size of the chunk
 */
int up_add_chunk(void *_instance, const char* chunk, size_t len)
{
   struct up *up = _instance;
   const struct up_settings *settings = up->settings;
   char *buffer;

   // Add chunk to buffer
   //size_t old_buffer_size = up->size;
   up->end += len;
   buffer = realloc(up->buffer, up->end*sizeof(char));
   if(buffer == NULL) {
      fprintf(stderr, "Realloc failed in URL parser when allocating"
                      "space for new URL chunk\n");
      up->state = S_ERROR;
      return 1;
   }
   up->buffer = buffer;
   memcpy(&buffer[up->insert], chunk, len);
   up->insert += len;

   // Parse the new chunk in buffer
   for(;up->parser < up->end; up->parser++)
   {
      char c = up->buffer[up->parser];

      // Check if it is a valid URL char. If not, print an error message
      // and set parser to error state
      if (!isLegalURLChar(c))
      {
         fprintf(stderr, "The URL parser received an invalid "
                         "character: %c\n", c);
         up->state = S_ERROR;
      }

      switch(up->state)
      {
         case S_START:
            if (settings->on_begin != NULL) {
               settings->on_begin(up->data);
            }
            if (c == '/') {
               up->state = S_SEGMENT;
               up->path = up->parser;
               up->path_l++;
               up->last_path = up->parser+1;
            } else {
               up->state = S_PROTOCOL;
               up->protocol = up->parser;
               up->protocol_l++;
            }
            break;
         case S_PROTOCOL:
            if (c == ':') {
               if(settings->on_protocol != NULL) {
                  settings->on_protocol(up->data,
                                        &up->buffer[up->protocol],
                                        up->parser-up->protocol);
               }
               up->state = S_SLASH1;
            } else {
               up->protocol_l++;
            }
            break;
         case S_SLASH1:
            if(c == '/') {
               up->state = S_SLASH2;
            } else {
               up->state = S_ERROR;
            }
            break;
         case S_SLASH2:
            if(c == '/') {
               up->state = S_HOST;
               up->host = up->parser + 1;
            } else {
               up->state = S_ERROR;
            }
            break;
         case S_HOST:
            if(c == ':' || c == '/') {
               if (settings->on_host != NULL) {
                  up->settings->on_host(up->data,
                                        &up->buffer[up->host],
                                        up->host_l);
               }
               if (c == ':') up->state = S_PREPORT;
               if (c == '/') {
                  up->state = S_SEGMENT;
                  up->path = up->parser;
                  up->path_l++;
                  up->last_path = up->parser+1;
               }
               break;
            } else {
               up->host_l++;
            }
            break;
         case S_PREPORT:
            if(c == '/')
               up->state = S_ERROR;
            up->state = S_PORT;
            up->port = up->parser;
            up->port_l++;
            break;
         case S_PORT:
            if (c == '/')
            {
               if (settings->on_port != NULL) {
                  settings->on_port(up->data,
                                    &up->buffer[up->port],
                                    up->port_l);
               }
               up->state = S_SEGMENT;
               up->path = up->parser;
               up->path_l++;
               up->last_path = up->parser+1;
            } else {
               up->port_l++;
            }
            break;
         case S_SEGMENT:
            if (c == '/' || c == '?') {
               if (settings->on_path_segment != NULL) {
                  settings->on_path_segment(up->data,
                                            &up->buffer[up->last_path],
                                            up->last_path_l);
               }
               up->last_path = up->parser+1;
               up->last_path_l = 0;
            } else {
               up->last_path_l++;
            }
            if (c == '?') {
               if (settings->on_path_complete != NULL) {
                  settings->on_path_complete(up->data,
                                             &up->buffer[up->path],
                                             up->path_l);
               }
               up->state = S_KEY;
               up->key_value = up->parser+1;
               up->last_key = up->parser+1;
            } else {
               up->path_l++;
            }
            break;
         case S_KEY:
            if (c == '=') {
               up->state = S_VALUE;
               up->last_value = up->parser+1;
               up->last_value_l = 0;
            } else {
               up->last_key_l++;
            }
            break;
         case S_VALUE:
            if (c == '&') {
               if (settings->on_key_value != NULL) {
                  settings->on_key_value(up->data,
                                         &up->buffer[up->last_key], 
                                         up->last_key_l,
                                         &up->buffer[up->last_value],
                                         up->last_value_l);
               }
               up->state = S_KEY;
               up->last_key = up->parser+1;
               up->last_key_l = 0;
            } else {
               up->last_value_l++;
            }
            break;

         case S_ERROR:
               fprintf(stderr,"The URL parser has reached an error state\n");
               return 1;
            break;
      }
   }

   return 0;
}

/// Informs the parser that the URL is complete
/**
 *   This method will let the parser know that the URL is complete.
 *   This always results in the on_complete callback being called, but it
 *   may also inflict two others: on_path_segment and on_key_value. This
 *   is due to the nature of the URL parser being able to receive in
 *   chunks: it can simply now know if an URL path is complete without a /
 *   at the end. It also cannot know if a value part of a key is done
 *   before being told.
 *
 *  @param  instance A pointer to an URL Parser instance
 */
int up_complete(void *_instance)
{
   struct up *up = _instance;
   const struct up_settings *settings = up->settings;

   // Check if we need to send a last chunk and that we are in a valid
   // end state
   switch(up->state)
   {
      case S_SEGMENT:
         if (settings->on_path_segment != NULL) {
            settings->on_path_segment(up->data, 
                                      &up->buffer[up->last_path],
                                      up->last_path_l);
         }
         if (settings->on_path_complete != NULL) {
            settings->on_path_complete(up->data, 
                                       &up->buffer[up->path],
                                       up->path_l);
         }
         break;
      case S_VALUE:
         if (settings->on_key_value != NULL) {
            settings->on_key_value(up->data, 
                                   &up->buffer[up->last_key],
                                   up->last_key_l,
                                   &up->buffer[up->last_value],
                                   up->last_value_l);
         }
         break;

      case S_HOST:
            if (settings->on_host != NULL) {
               settings->on_host(up->data,
                                 &up->buffer[up->host],
                                 up->host_l);
            }
         break;
      case S_PORT:
            if (settings->on_port != NULL) {
               settings->on_port(up->data,
                                 &up->buffer[up->port],
                                 up->port_l);
            }
         break;
      default:
         fprintf(stderr, "An error has happened in the URL parser. End "
                         "state: %d\n",up->state);
         return 1;
   }

   if(settings->on_complete != NULL) {
      settings->on_complete(
            up->data,
            up->buffer,
            up->parser);
   }
   return 0;
}
