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

#ifndef HOMEPORT_VALUE_H
#define HOMEPORT_VALUE_H

#include "hpd_types.h"
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

hpd_error_t value_alloc(hpd_value_t **value, const char *body, int len);
hpd_error_t value_copy(hpd_value_t **dst, const hpd_value_t *src);
hpd_error_t value_free(hpd_value_t *value);
hpd_error_t value_set_header(hpd_value_t *value, const char *key, const char *val);
hpd_error_t value_set_headers_v(hpd_value_t *value, va_list vp);
hpd_error_t value_get_body(const hpd_value_t *value, const char **body, size_t *len);
hpd_error_t value_get_header(const hpd_value_t *value, const char *key, const char **val);
hpd_error_t value_get_headers_v(const hpd_value_t *value, va_list vp);
hpd_error_t value_first_header(hpd_value_t *value, hpd_pair_t **pair);
hpd_error_t value_next_header(hpd_pair_t **pair);

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_VALUE_H
