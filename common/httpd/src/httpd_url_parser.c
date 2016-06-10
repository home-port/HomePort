/*
 * Copyright 2011 Aalborg University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 */

#include <string.h>
#include <stdlib.h>

#include "httpd_url_parser.h"
#include "hpd/hpd_shared_api.h"

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
struct up {
    const hpd_module_t *context;
    struct up_settings *settings; ///< Settings
    void *data;                   ///< User data

    enum up_state state;          ///< State
    char *buffer;                 ///< URL Buffer

    size_t protocol;              ///< Protocol start
    size_t protocol_l;            ///< Protocol length
    size_t host;                  ///< Host start
    size_t host_l;                ///< Host length
    size_t port;                  ///< Port start
    size_t port_l;                ///< Port length
    size_t path;                  ///< Path start
    size_t path_l;                ///< Path length
    size_t key_value;             ///< Arguments start
    size_t end;                   ///< Length of full URL

    size_t last_key;              ///< Last seen key start
    size_t last_key_l;            ///< Last seen key length
    size_t last_value;            ///< Last seen value start
    size_t last_value_l;          ///< Last seen value length
    size_t last_path;             ///< Last seen path segment start
    size_t last_path_l;           ///< Last seen path segment length

    size_t parser;                ///< Location of parser
    size_t insert;                ///< Location to insert new chunks
};

/**
 * Create URL parser instance.
 *
 *  This method creates an URL Parser instance.  It allocates memory
 *  for itself and copy the settings struct.  It also sets all values
 *  to default.
 *
 *  The instance should be destroyed using up_destroy when it is no
 *  longer needed.
 *
 *  \param  settings  A pointer to a url_parser_settings struct
 *  \param  data      Pointer to user data, will be supplied to
 *                    callbacks
 *
 *  \return  a pointer to the newly created instance
 */
hpd_error_t up_create(struct up **instance, struct up_settings *settings, const hpd_module_t *context, void *data)
{
    if (!context) return HPD_E_NULL;
    if (!instance || !settings) HPD_LOG_RETURN_E_NULL(context);

    (*instance) = malloc(sizeof(struct up));
    if (!(*instance)) return HPD_E_ALLOC;

    (*instance)->context = context;

    // Store settings
    (*instance)->settings = malloc(sizeof(struct up_settings));
    if (!(*instance)->settings) {
        free(*instance);
        HPD_LOG_RETURN_E_ALLOC(context);
    }
    memcpy((*instance)->settings, settings, sizeof(struct up_settings));

    // Set state
    (*instance)->state = S_START;
    (*instance)->buffer = NULL;

    // Set pointers
    (*instance)->protocol = 0;
    (*instance)->protocol_l = 0;
    (*instance)->host = 0;
    (*instance)->host_l = 0;
    (*instance)->port = 0;
    (*instance)->port_l = 0;
    (*instance)->path = 0;
    (*instance)->path_l = 0;
    (*instance)->key_value = 0;
    (*instance)->end = 0;
    (*instance)->last_key = 0;
    (*instance)->last_key_l = 0;
    (*instance)->last_value = 0;
    (*instance)->last_value_l = 0;
    (*instance)->last_path = 0;
    (*instance)->last_path_l = 0;
    (*instance)->parser = 0;
    (*instance)->insert = 0;

    // Store data
    (*instance)->data = data;

    return HPD_E_SUCCESS;
}

/**
 * Destroy URL parser instance.
 *
 *  This method destroys an URL Parser instance, including the buffer
 *  and settings struct.
 *
 *  \param  instance  A pointer to an url_parser_instance to destroy
 */
hpd_error_t up_destroy(struct up *instance)
{
    if (!instance) return HPD_E_NULL;

    if(instance->settings) free(instance->settings);
    if(instance->buffer) free(instance->buffer);
    free(instance);

    return HPD_E_SUCCESS;
}

/**
 * Check if a given char is valid in an URL.
 *
 *  This method checks if a char is valid in an URL. Only specific chars
 *  are valid, others have to be encoded properly. 
 *
 *  @param  c A char to check
 *  @return 1 if the char is valid in an URL, 0 otherwise
 */
static int up_isLegalURLChar(char c)
{
    if ((c >= '0' && c <= '9') ||
        (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        c == '-' || c == '.' || c == '_' || c == '~' || c == ':' ||
        c == '/' || c == '?' || c == '#' || c == '[' || c == ']' ||
        c == '@' || c == '!' || c == '$' || c == '&' || c == '\'' ||
        c == '(' || c == ')' || c == '*' || c == '+' || c == ',' ||
        c == ';' || c == '=' || c == '%')
        return 1;

    return 0;
}

#define UP_CALL(X, ...) do { \
    hpd_error_t rc; \
    if (settings->X != NULL) { \
        if ((rc = settings->X(instance->data, ##__VA_ARGS__))) { \
            instance->state = S_ERROR; \
            return rc; \
        } \
    } \
} while(0)

/**
 * Parse a chunk of an URL.
 *
 *  This method receives a chunk of an URL, and calls the appropriate
 *  callbacks.  It increases the size of buffer, copies the new chunk to
 *  it and then parses each character and changes state accordingly.  A
 *  chunk may of course be a complete URL, all of which will be parsed
 *  immediately. 
 *
 *  @param  instance    A pointer to an URL Parser instance
 *  @param  chunk       A pointer to the chunk (non zero terminated)
 *  @param  chunk_size  The size of the chunk
 */
hpd_error_t up_add_chunk(struct up *instance, const char *chunk, size_t len)
{
    if (!instance || !chunk) return HPD_E_NULL;

    const struct up_settings *settings = instance->settings;
    char *buffer;

    // Add chunk to buffer
    instance->end += len;
    buffer = realloc(instance->buffer, instance->end * sizeof(char));
    if (!buffer) {
        instance->state = S_ERROR;
        HPD_LOG_RETURN_E_ALLOC(instance->context);
        return HPD_E_ALLOC;
    }
    instance->buffer = buffer;
    memcpy(&buffer[instance->insert], chunk, len);
    instance->insert += len;

    // Parse the new chunk in buffer
    for(; instance->parser < instance->end; instance->parser++)
    {
        char c = instance->buffer[instance->parser];

        // Check if it is a valid URL char. If not, print an error message
        // and set parser to error state
        if (!up_isLegalURLChar(c))
        {
            instance->state = S_ERROR;
            HPD_LOG_RETURN(instance->context, HPD_E_ARGUMENT, "Invalid character ('%c') in URL.", c);
        }

        switch(instance->state)
        {
            case S_START:
                UP_CALL(on_begin);
                if (c == '/') {
                    instance->state = S_SEGMENT;
                    instance->path = instance->parser;
                    instance->path_l++;
                    instance->last_path = instance->parser + 1;
                } else {
                    instance->state = S_PROTOCOL;
                    instance->protocol = instance->parser;
                    instance->protocol_l++;
                }
                break;
            case S_PROTOCOL:
                if (c == ':') {
                    UP_CALL(on_protocol, &instance->buffer[instance->protocol], instance->parser - instance->protocol);
                    instance->state = S_SLASH1;
                } else {
                    instance->protocol_l++;
                }
                break;
            case S_SLASH1:
                if(c == '/') {
                    instance->state = S_SLASH2;
                } else {
                    instance->state = S_ERROR;
                    HPD_LOG_RETURN(instance->context, HPD_E_ARGUMENT, "URL Parse error.");
                }
                break;
            case S_SLASH2:
                if(c == '/') {
                    instance->state = S_HOST;
                    instance->host = instance->parser + 1;
                } else {
                    instance->state = S_ERROR;
                    HPD_LOG_RETURN(instance->context, HPD_E_ARGUMENT, "URL Parse error.");
                }
                break;
            case S_HOST:
                if(c == ':' || c == '/') {
                    UP_CALL(on_host, &instance->buffer[instance->host], instance->host_l);
                    if (c == ':') instance->state = S_PREPORT;
                    if (c == '/') {
                        instance->state = S_SEGMENT;
                        instance->path = instance->parser;
                        instance->path_l++;
                        instance->last_path = instance->parser + 1;
                    }
                    break;
                } else {
                    instance->host_l++;
                }
                break;
            case S_PREPORT:
                if(c == '/') {
                    instance->state = S_ERROR;
                    HPD_LOG_RETURN(instance->context, HPD_E_ARGUMENT, "URL Parse error.");
                }
                instance->state = S_PORT;
                instance->port = instance->parser;
                instance->port_l++;
                break;
            case S_PORT:
                if (c == '/')
                {
                    UP_CALL(on_port, &instance->buffer[instance->port], instance->port_l);
                    instance->state = S_SEGMENT;
                    instance->path = instance->parser;
                    instance->path_l++;
                    instance->last_path = instance->parser + 1;
                } else {
                    instance->port_l++;
                }
                break;
            case S_SEGMENT:
                if (c == '/' || c == '?') {
                    UP_CALL(on_path_segment, &instance->buffer[instance->last_path], instance->last_path_l);
                    instance->last_path = instance->parser + 1;
                    instance->last_path_l = 0;
                } else {
                    instance->last_path_l++;
                }
                if (c == '?') {
                    UP_CALL(on_path_complete, &instance->buffer[instance->path], instance->path_l);
                    instance->state = S_KEY;
                    instance->key_value = instance->parser + 1;
                    instance->last_key = instance->parser + 1;
                } else {
                    instance->path_l++;
                }
                break;
            case S_KEY:
                if (c == '=') {
                    instance->state = S_VALUE;
                    instance->last_value = instance->parser + 1;
                    instance->last_value_l = 0;
                } else {
                    instance->last_key_l++;
                }
                break;
            case S_VALUE:
                if (c == '&') {
                    UP_CALL (on_key_value, &instance->buffer[instance->last_key], instance->last_key_l, &instance->buffer[instance->last_value], instance->last_value_l);
                    instance->state = S_KEY;
                    instance->last_key = instance->parser + 1;
                    instance->last_key_l = 0;
                } else {
                    instance->last_value_l++;
                }
                break;

            case S_ERROR:
                HPD_LOG_RETURN(instance->context, HPD_E_STATE, "URL Parser in error state.");
            default:
                HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Unexpected state.");
        }
    }

    return HPD_E_SUCCESS;
}

/**
 * Informs the parser that the URL is complete.
 *
 *  This method will let the parser know that the URL is complete.
 *  This always results in the on_complete callback being called, but it
 *  may also inflict two others: on_path_segment and on_key_value. This
 *  is due to the nature of the URL parser being able to receive in
 *  chunks: it can simply now know if an URL path is complete without a
 *  slash at the end. It also cannot know if a value part of a key is
 *  done before being told.
 *
 *  @param  instance A pointer to an URL Parser instance
 */
hpd_error_t up_complete(struct up *instance)
{
    if (!instance) return HPD_E_NULL;

    const struct up_settings *settings = instance->settings;

    // Check if we need to send a last chunk and that we are in a valid
    // end state
    switch(instance->state)
    {
        case S_SEGMENT:
            UP_CALL(on_path_segment, &instance->buffer[instance->last_path], instance->last_path_l);
            UP_CALL (on_path_complete, &instance->buffer[instance->path], instance->path_l);
            break;
        case S_VALUE:
            UP_CALL (on_key_value, &instance->buffer[instance->last_key], instance->last_key_l, &instance->buffer[instance->last_value], instance->last_value_l);
            break;
        case S_HOST:
            UP_CALL(on_host, &instance->buffer[instance->host], instance->host_l);
            break;
        case S_PORT:
            UP_CALL(on_port, &instance->buffer[instance->port], instance->port_l);
            break;
        default:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Unexpected state.");
    }

    UP_CALL(on_complete, instance->buffer, instance->parser);
    return HPD_E_SUCCESS;
}

