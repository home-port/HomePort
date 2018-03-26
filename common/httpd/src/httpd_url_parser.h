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

#ifndef HOMEPORT_HTTPD_URL_PARSER_H
#define HOMEPORT_HTTPD_URL_PARSER_H

#include "hpd-0.6/hpd_types.h"

typedef hpd_error_t (*up_string_cb)(void *data, const char* parsedSegment, size_t segment_length);
typedef hpd_error_t (*up_pair_cb)(void *data, const char* key, size_t key_length, const char* value, size_t value_length);
typedef hpd_error_t (*up_void_cb)(void *data);

struct up;

/**
 * Settings struct for the URL Parser.
 *
 *  Please initialise this struct as following, to ensure that all
 *  settings have acceptable default values:
 *  \code
 *  struct url_parser_settings *settings = URL_PARSER_SETTINGS_DEFAULT;
 *  \endcode
 *
 *  The settings hold a series of callbacks of type either up_string_cb,
 *  up_pair_cb or up_void_cb.  Strings received in up_pair_cb and
 *  up_string_cb are never null-terminated, and they always have a
 *  length.  The callbacks are called when the URL parser finishes
 *  parsing a specific chunk.  The callback with the last part of a
 *  segment (e.g. the c in /a/b/c) might first be called when
 *  up_complete has been called, as it cannot know if c is the last
 *  character of the segment unless it terminates with a /.
 *
 */

struct up_settings {
    up_void_cb on_begin;
    up_string_cb on_protocol;
    up_string_cb on_host;
    up_string_cb on_port;
    up_string_cb on_path_segment;
    up_string_cb on_path_complete;
    up_pair_cb on_key_value;
    up_string_cb on_complete;
};

#define UP_SETTINGS_DEFAULT {\
	.on_begin = NULL, .on_protocol = NULL, .on_host = NULL, \
	.on_port = NULL, .on_path_segment = NULL, .on_path_complete = NULL, \
	.on_key_value = NULL, .on_complete = NULL }

hpd_error_t up_create(struct up **instance, struct up_settings *settings, const hpd_module_t *context, void *data);
hpd_error_t up_destroy(struct up *instance);

hpd_error_t up_add_chunk(struct up *instance, const char *chunk, size_t chunk_size);
hpd_error_t up_complete(struct up *instance);

#endif
