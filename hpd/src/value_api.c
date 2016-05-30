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

#include "hpd_api.h"
#include "value.h"
#include "log.h"

hpd_error_t hpd_value_alloc(hpd_value_t **value, const char *body, int len)
{
    if (!value) LOG_RETURN_E_NULL();
    if (len < 0 && len != HPD_NULL_TERMINATED)
        LOG_RETURN(HPD_E_ARGUMENT, "len must be >= 0 or HPD_NULL_TERMINATED.", __func__);
    return value_alloc(value, body, len);
}

hpd_error_t hpd_value_copy(hpd_value_t **dst, const hpd_value_t *src)
{
    if (!dst || ! src) LOG_RETURN_E_NULL();
    return value_copy(dst, src);
}

hpd_error_t hpd_value_free(hpd_value_t *value)
{
    if (!value) LOG_RETURN_E_NULL();
    return value_free(value);
}

hpd_error_t hpd_value_set_header(hpd_value_t *value, const char *key, const char *val)
{
    if (!value || !key) LOG_RETURN_E_NULL();
    if (key[0] == '_') LOG_RETURN(HPD_E_ARGUMENT, "Keys starting with '_' is reserved for generated headers");
    return value_set_header(value, key, val);
}

hpd_error_t hpd_value_set_headers(hpd_value_t *value, ...)
{
    if (!value) LOG_RETURN_E_NULL();

    va_list vp;
    va_start(vp, value);
    hpd_error_t rc = value_set_headers_v(value, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_value_get_body(const hpd_value_t *value, const char **body, size_t *len)
{
    if (!value || (!body && !len)) LOG_RETURN_E_NULL();
    return value_get_body(value, body, len);
}

hpd_error_t hpd_value_get_header(const hpd_value_t *value, const char *key, const char **val)
{
    if (!value || !key || !val) LOG_RETURN_E_NULL();
    return value_get_header(value, key, val);
}

hpd_error_t hpd_value_get_headers(const hpd_value_t *value, ...)
{
    if (!value) LOG_RETURN_E_NULL();

    va_list vp;
    va_start(vp, value);
    hpd_error_t rc = value_get_headers_v(value, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_value_first_header(hpd_value_t *value, hpd_pair_t **pair)
{
    if (!value || !pair) LOG_RETURN_E_NULL();
    return value_first_header(value, pair);
}

hpd_error_t hpd_value_next_header(hpd_pair_t **pair)
{
    if (!pair || !*pair) LOG_RETURN_E_NULL();
    return value_next_header(pair);
}
