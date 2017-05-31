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

#include "value.h"
#include "hpd/common/hpd_common.h"
#include "hpd/common/hpd_map.h"
#include "comm.h"
#include "log.h"
#include "daemon.h"

hpd_error_t value_alloc(hpd_value_t **value, const hpd_module_t *context, const char *body, int len)
{
    hpd_error_t rc;
    HPD_CALLOC((*value), 1, hpd_value_t);
    (*value)->context = context;
    if ((rc = hpd_map_alloc(&(*value)->headers))) {
        free(*value);
        return rc;
    }
    if (body) {
        HPD_STR_CPY((*value)->body, body);
        if (len == HPD_NULL_TERMINATED) (*value)->len = strlen(body);
        else (*value)->len = (size_t) len;
    }
    return HPD_E_SUCCESS;

    alloc_error:
        value_free(*value);
        (*value) = NULL;
        LOG_RETURN_E_ALLOC(context->hpd);
}

hpd_error_t value_vallocf(hpd_value_t **value, const hpd_module_t *context, const char *fmt, va_list vp)
{
    hpd_error_t rc;
    HPD_CALLOC((*value), 1, hpd_value_t);
    (*value)->context = context;
    if ((rc = hpd_map_alloc(&(*value)->headers))) {
        free(*value);
        return rc;
    }
    if (fmt) {
        HPD_VSPRINTF_ALLOC((*value)->body, fmt, vp);
        (*value)->len = strlen((*value)->body);
    }
    return HPD_E_SUCCESS;

    alloc_error:
    value_free(*value);
    (*value) = NULL;
    LOG_RETURN_E_ALLOC(context->hpd);

    vsnprintf_error:
    value_free(*value);
    free((*value)->body);
    LOG_RETURN(context->hpd, HPD_E_UNKNOWN, "vsnprintf error.");
}

hpd_error_t value_copy(hpd_value_t **dst, const hpd_value_t *src)
{
    hpd_error_t rc;
    if ((rc = value_alloc(dst, src->context, src->body, (int) src->len))) return rc;
    const hpd_pair_t *pair;
    hpd_map_foreach(rc, pair, src->headers) {
        const char *k, *v;
        if ((rc = hpd_pair_get(pair, &k, &v))) {
            value_free(*dst);
            return rc;
        }
        if ((rc = value_set_header(*dst, k, v))) {
            value_free(*dst);
            return rc;
        }
    }
    return rc;
}

hpd_error_t value_free(hpd_value_t *value)
{
    hpd_error_t rc = HPD_E_SUCCESS;
    if (value) {
        rc = hpd_map_free(value->headers);
        free(value->body);
    }
    free(value);
    return rc;
}

hpd_error_t value_set_header(hpd_value_t *value, const char *key, const char *val)
{
    return hpd_map_set(value->headers, key, val);
}

hpd_error_t value_set_headers_v(hpd_value_t *value, va_list vp)
{
    hpd_error_t rc;
    const char *key, *val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char *);
        if (!val) LOG_RETURN_E_NULL(value->context->hpd);
        if ((rc = value_set_header(value, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t value_get_body(const hpd_value_t *value, const char **body, size_t *len)
{
    if (body) (*body) = value->body;
    if (len) (*len) = value->len;
    return HPD_E_SUCCESS;
}

hpd_error_t value_get_header(const hpd_value_t *value, const char *key, const char **val)
{
    return hpd_map_get(value->headers, key, val);
}

hpd_error_t value_get_headers_v(const hpd_value_t *value, va_list vp)
{
    hpd_error_t rc;
    const char *key, **val;

    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char **);
        if (!val) LOG_RETURN_E_NULL(value->context->hpd);
        if ((rc = value_get_header(value, key, val))) return rc;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t value_first_header(const hpd_value_t *value, const hpd_pair_t **pair)
{
    return hpd_map_first(value->headers, pair);
}

hpd_error_t value_next_header(const hpd_pair_t **pair)
{
    return hpd_map_next(pair);
}

