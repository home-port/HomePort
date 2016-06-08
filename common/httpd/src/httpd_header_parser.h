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
 * THIS SOFTWARE IS PROVidED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
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

#ifndef HOMEPORT_HTTPD_HEADER_PARSER_H
#define HOMEPORT_HTTPD_HEADER_PARSER_H

#include "hpd/hpd_types.h"

typedef hpd_error_t (*hp_string_cb)(void* data, const char* field, size_t field_length, const char* value, size_t value_length);

struct hp;

struct hp_settings {
	hp_string_cb on_field_value_pair;
	void* data;
};

#define HP_SETTINGS_DEFAULT {\
	.on_field_value_pair = NULL, \
	.data = NULL }

hpd_error_t hp_create(struct hp **instance, struct hp_settings *settings, const hpd_module_t *context);
hpd_error_t hp_destroy(struct hp*);

hpd_error_t hp_on_header_field(struct hp *instance, const char *field_chunk, size_t length);
hpd_error_t hp_on_header_value(struct hp *instance, const char *value_chunk, size_t length);
hpd_error_t hp_on_header_complete(struct hp *instance);

#endif
