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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "header_parser.h"

// TODO Need to get rid of printfs in a lot of places (this file and others...)

enum hp_state {
	S_ERROR = -1,
	S_FIELD,
	S_VALUE,
	S_COMPLETED
};

struct hp {
	struct hp_settings *settings;

	enum hp_state state;

	char* field_buffer;
	size_t field_buffer_size;

	char* value_buffer;
	size_t value_buffer_size;
};

static void reset_buffers(struct hp *instance)
{
		if(instance->field_buffer) free(instance->field_buffer);
		if(instance->value_buffer) free(instance->value_buffer);

		instance -> field_buffer_size = 0;
		instance -> field_buffer = NULL;

		instance -> value_buffer_size = 0;
		instance -> value_buffer = NULL;
}

hpd_error_t hp_create(struct hp **instance, struct hp_settings *settings)
{
    if (!instance || !settings) return HPD_E_NULL;

	(*instance) = malloc(sizeof(struct hp));
    if (!(*instance)) return HPD_E_ALLOC;

	(*instance)->settings = malloc(sizeof(struct hp_settings));
    if (!(*instance)->settings) {
        fprintf(stderr, "Malloc failed in header parser when allocating space for header parser settings\n");
        free(*instance);
        return HPD_E_ALLOC;
    }

	memcpy((*instance)->settings, settings, sizeof(struct hp_settings));

	(*instance)->state = S_FIELD;

	(*instance)->field_buffer_size = 0;
    (*instance)->field_buffer = NULL;

	(*instance)->value_buffer_size = 0;
	(*instance)->value_buffer = NULL;

	return HPD_E_SUCCESS;
}

hpd_error_t hp_on_header_field(struct hp *instance, const char *field_chunk, size_t length)
{
    if (!instance || field_chunk) return HPD_E_NULL;

    hpd_error_t rc;
    size_t old_buffer_size = instance->field_buffer_size;

    switch (instance->state) {
        case S_VALUE:
            if ((rc = instance->settings->on_field_value_pair(instance->settings->data, instance->field_buffer, instance->field_buffer_size, instance->value_buffer, instance->value_buffer_size))) {
                instance->state = S_ERROR;
                return rc;
            }
            reset_buffers(instance);
            old_buffer_size = 0;
            instance->state = S_FIELD;
        case S_FIELD:
            instance->field_buffer_size += length;
            instance->field_buffer = realloc(instance->field_buffer, instance->field_buffer_size * (sizeof(char)));
            if(instance->field_buffer == NULL) {
                fprintf(stderr, "Realloc failed in URL parser when allocating space for new URL chunk\n");
                return HPD_E_ALLOC;
            }
            memcpy(instance->field_buffer+old_buffer_size, field_chunk, length);
            return HPD_E_SUCCESS;
        case S_COMPLETED:
            fprintf(stderr, "The header parser received additional data after a call to completed\n");
        case S_ERROR: // TODO Error msg ?
        default:
            return HPD_E_STATE;
    }
}

hpd_error_t hp_on_header_value(struct hp *instance, const char *value_chunk, size_t length)
{
    if (!instance || !value_chunk) return  HPD_E_NULL;

    size_t old_buffer_size = instance->value_buffer_size;

    switch (instance->state) {
        case S_FIELD:
            instance->state = S_VALUE;
        case S_VALUE:
            instance->value_buffer_size += length;
            instance->value_buffer = realloc(instance->value_buffer, instance->value_buffer_size * (sizeof(char)));
            if(instance->value_buffer == NULL) {
                fprintf(stderr, "Realloc failed in URL parser when allocating space for new URL chunk\n");
                return HPD_E_ALLOC;
            }
            memcpy(instance->value_buffer+old_buffer_size, value_chunk, length);
            return HPD_E_SUCCESS;
        case S_COMPLETED:
            fprintf(stderr, "The header parser received additional data after a call to completed\n");
        case S_ERROR: // TODO Error msg ?
        default:
            return HPD_E_STATE;
    }
}

hpd_error_t hp_on_header_complete(struct hp *instance)
{
    if (!instance) return HPD_E_NULL;

    hpd_error_t rc;

    switch (instance->state) {
        case S_FIELD:
            fprintf(stderr, "The header parser was missing a value when the call to completed was made\n");
            return HPD_E_STATE;
        case S_VALUE:
            if ((rc = instance->settings->on_field_value_pair(instance->settings->data, instance->field_buffer, instance->field_buffer_size, instance->value_buffer, instance->value_buffer_size))) {
                instance->state = S_ERROR;
                return rc;
            }
            instance->state = S_COMPLETED;
            return HPD_E_SUCCESS;
        case S_COMPLETED: // TODO Error msg ?
        case S_ERROR: // TODO Error msg ?
        default: // TODO Error msg ?
            return HPD_E_STATE;
    }
}

hpd_error_t hp_destroy(struct hp *instance)
{
    if (!instance) return HPD_E_NULL;

    if(instance->settings != NULL) free(instance->settings);
    reset_buffers(instance);
    free(instance);

    return HPD_E_SUCCESS;
}
