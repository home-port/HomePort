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

#include "httpd_header_parser.h"
#include "hpd_shared_api.h"

enum hp_state {
	S_ERROR = -1,
	S_FIELD,
	S_VALUE,
	S_COMPLETED
};

struct hp {
    const hpd_module_t *context;
	struct hp_settings settings;

	enum hp_state state;

	char* field_buffer;
	size_t field_buffer_size;

	char* value_buffer;
	size_t value_buffer_size;
};

static void hp_reset_buffers(struct hp *instance)
{
		if(instance->field_buffer) free(instance->field_buffer);
		if(instance->value_buffer) free(instance->value_buffer);

		instance -> field_buffer_size = 0;
		instance -> field_buffer = NULL;

		instance -> value_buffer_size = 0;
		instance -> value_buffer = NULL;
}

hpd_error_t hp_create(struct hp **instance, struct hp_settings *settings, const hpd_module_t *context)
{
    if (!context) return HPD_E_NULL;
    if (!instance || !settings) HPD_LOG_RETURN_E_NULL(context);

	(*instance) = malloc(sizeof(struct hp));
    if (!(*instance)) HPD_LOG_RETURN_E_ALLOC(context);
    
    (*instance)->context = context;

	memcpy(&(*instance)->settings, settings, sizeof(struct hp_settings));

	(*instance)->state = S_FIELD;

	(*instance)->field_buffer_size = 0;
    (*instance)->field_buffer = NULL;

	(*instance)->value_buffer_size = 0;
	(*instance)->value_buffer = NULL;

	return HPD_E_SUCCESS;
}

hpd_error_t hp_on_header_field(struct hp *instance, const char *field_chunk, size_t length)
{
    if (!instance)
        return HPD_E_NULL;
    if (!field_chunk)
        HPD_LOG_RETURN_E_NULL(instance->context);

    hpd_error_t rc;
    size_t old_buffer_size = instance->field_buffer_size;

    switch (instance->state) {
        case S_VALUE: {
            hp_string_cb on_field_value_pair = instance->settings.on_field_value_pair;
            if (on_field_value_pair && (rc = on_field_value_pair(instance->settings.data, 
                                                                 instance->field_buffer, instance->field_buffer_size, 
                                                                 instance->value_buffer, instance->value_buffer_size))) {
                        instance->state = S_ERROR;
                        return rc;
                    }
            hp_reset_buffers(instance);
            old_buffer_size = 0;
            instance->state = S_FIELD;
        }
        case S_FIELD: {
            size_t new_len = instance->field_buffer_size + length;
            char *new_buf = realloc(instance->field_buffer, new_len * (sizeof(char)));
            if (!new_buf) HPD_LOG_RETURN_E_ALLOC(instance->context);
            instance->field_buffer = new_buf;
            instance->field_buffer_size = new_len;
            memcpy(instance->field_buffer+old_buffer_size, field_chunk, length);
            return HPD_E_SUCCESS;
        }
        case S_COMPLETED:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Received additional data after hp_on_header_complete().");
        case S_ERROR:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Cannot receive data: In an error state.");
        default:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Unexpected state.");
    }
}

hpd_error_t hp_on_header_value(struct hp *instance, const char *value_chunk, size_t length)
{
    if (!instance || !value_chunk) return  HPD_E_NULL;

    size_t old_buffer_size = instance->value_buffer_size;

    switch (instance->state) {
        case S_FIELD:
            instance->state = S_VALUE;
        case S_VALUE: {
            size_t new_len = instance->value_buffer_size + length;
            char *new_buf = realloc(instance->value_buffer, new_len * (sizeof(char)));
            if (!new_buf) HPD_LOG_RETURN_E_ALLOC(instance->context);
            instance->value_buffer = new_buf;
            instance->value_buffer_size = new_len;
            memcpy(instance->value_buffer+old_buffer_size, value_chunk, length);
            return HPD_E_SUCCESS;
        }
        case S_COMPLETED:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Received additional data after hp_on_header_complete().");
        case S_ERROR:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Cannot receive data: In an error state.");
        default:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Unexpected state.");
    }
}

hpd_error_t hp_on_header_complete(struct hp *instance)
{
    if (!instance) return HPD_E_NULL;

    hpd_error_t rc;

    switch (instance->state) {
        case S_FIELD:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Cannot complete headers: Missing value to last key.");
        case S_VALUE:
            if ((rc = instance->settings.on_field_value_pair(instance->settings.data, instance->field_buffer, instance->field_buffer_size, instance->value_buffer, instance->value_buffer_size))) {
                instance->state = S_ERROR;
                return rc;
            }
            instance->state = S_COMPLETED;
            return HPD_E_SUCCESS;
        case S_COMPLETED:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Headers are already completed.");
        case S_ERROR:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Cannot complete headers: In an error state.");
        default:
            HPD_LOG_RETURN(instance->context, HPD_E_STATE, "Unexpected state.");
    }
}

hpd_error_t hp_destroy(struct hp *instance)
{
    if (!instance) return HPD_E_NULL;

    hp_reset_buffers(instance);
    free(instance);

    return HPD_E_SUCCESS;
}
