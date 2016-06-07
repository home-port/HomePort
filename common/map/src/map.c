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

#include "hpd_types.h"
#include "hpd_map.h"
#include "hpd_common.h"

TAILQ_HEAD(hpd_map, hpd_pair);

struct hpd_pair {
    TAILQ_ENTRY(hpd_pair) HPD_TAILQ_FIELD; //< Tailq members
    char *k; //< Key (not null)
    char *v; //< Value
};

hpd_error_t hpd_map_alloc(hpd_map_t **map)
{
    if (!map) return HPD_E_NULL;

    HPD_CALLOC(*map, 1, hpd_map_t);
    TAILQ_INIT(*map);
    return HPD_E_SUCCESS;

    alloc_error:
        return HPD_E_ALLOC;
}

hpd_error_t hpd_map_first(hpd_map_t *map, hpd_pair_t **pair)
{
    if (!map || !pair) return HPD_E_NULL;

    (*pair) = TAILQ_FIRST(map);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_map_next(hpd_pair_t **pair)
{
    if (!pair || !(*pair)) return HPD_E_NULL;

    (*pair) = TAILQ_NEXT(*pair, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_map_remove(hpd_map_t *map, hpd_pair_t *pair)
{
    if (!map || !pair) return HPD_E_NULL;

    TAILQ_REMOVE(map, pair, HPD_TAILQ_FIELD);
    free(pair->k);
    free(pair->v);
    free(pair);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_map_free(hpd_map_t *map)
{
    if (!map) return HPD_E_NULL;

    hpd_error_t rc = HPD_E_SUCCESS;
    hpd_pair_t *pair, *tmp;
    TAILQ_FOREACH_SAFE(pair, map, HPD_TAILQ_FIELD, tmp) {
        rc = hpd_map_remove(map, pair);
    }
    free(map);
    return rc;
}

hpd_error_t hpd_map_get(hpd_map_t *map, const char *k, const char **v)
{
    if (!map || !k || !v) return HPD_E_NULL;
    
    hpd_pair_t *attr;
    (*v) = NULL;
    TAILQ_FOREACH(attr, map, HPD_TAILQ_FIELD) {
        if (strcmp(attr->k, k) == 0) {
            (*v) = attr->v;
            return HPD_E_SUCCESS;
        }
    }
    return HPD_E_NOT_FOUND;
}

hpd_error_t hpd_map_get_n(hpd_map_t *map, const char *k, size_t k_len, const char **v)
{
    if (!map || !k || !v) return HPD_E_NULL;

    hpd_pair_t *attr;
    (*v) = NULL;
    TAILQ_FOREACH(attr, map, HPD_TAILQ_FIELD) {
        if (strncmp(attr->k, k, k_len) == 0) {
            (*v) = attr->v;
            return HPD_E_SUCCESS;
        }
    }
    (*v) = NULL;
    return HPD_E_NOT_FOUND;
}

static hpd_error_t insert(hpd_map_t *map, const char *k, const char *v)
{
    hpd_pair_t *attr = NULL;
    HPD_CALLOC(attr, 1, hpd_pair_t);
    HPD_STR_CPY(attr->k, k);
    HPD_STR_CPY(attr->v, v);
    TAILQ_INSERT_TAIL(map, attr, tailq);
    return HPD_E_SUCCESS;

    alloc_error:
        if (attr) {
            free(attr->k);
            free(attr->v);
            free(attr);
        }
    return HPD_E_ALLOC;
}

static hpd_error_t replace(hpd_pair_t *attr, const char *v) {
    HPD_STR_CPY(attr->v, v);
    return HPD_E_SUCCESS;

    alloc_error:
        return HPD_E_ALLOC;
}

static hpd_error_t insert_n(hpd_map_t *map, const char *k, size_t k_len, const char *v, size_t v_len)
{
    hpd_pair_t *attr = NULL;
    HPD_CALLOC(attr, 1, hpd_pair_t);
    HPD_STR_N_CPY(attr->k, k, k_len);
    HPD_STR_N_CPY(attr->v, v, v_len);
    TAILQ_INSERT_TAIL(map, attr, tailq);
    return HPD_E_SUCCESS;

    alloc_error:
    if (attr) {
        free(attr->k);
        free(attr->v);
        free(attr);
    }
    return HPD_E_ALLOC;
}

static hpd_error_t replace_n(hpd_pair_t *attr, const char *v, size_t v_len) {
    HPD_STR_N_CPY(attr->v, v, v_len);
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

hpd_error_t hpd_map_set(hpd_map_t *map, const char *k, const char *v)
{
    if (!map || !k) return HPD_E_NULL;

    hpd_error_t rc;
    hpd_pair_t *attr = NULL;
    TAILQ_FOREACH(attr, map, HPD_TAILQ_FIELD)
        if (strcmp(attr->k, k) == 0)
            break;

    if (v == NULL) {
        if (attr && (rc = hpd_map_remove(map, attr))) return rc;
        return HPD_E_SUCCESS;
    } else if (!attr) {
        return insert(map, k, v);
    } else {
        return replace(attr, v);
    }
}

hpd_error_t hpd_map_set_n(hpd_map_t *map, const char *k, size_t k_len, const char *v, size_t v_len)
{
    if (!map || !k) return HPD_E_NULL;

    hpd_error_t rc;
    hpd_pair_t *attr = NULL;
    TAILQ_FOREACH(attr, map, HPD_TAILQ_FIELD)
        if (strncmp(attr->k, k, k_len) == 0)
            break;
    
    if (v == NULL) {
        if (attr && (rc = hpd_map_remove(map, attr))) return rc;
        return HPD_E_SUCCESS;
    } else if (!attr) {
        return insert_n(map, k, k_len, v, v_len);
    } else {
        return replace_n(attr, v, v_len);
    }
}

hpd_error_t hpd_map_v_matches(hpd_map_t *map, va_list vp)
{
    if (!map) return HPD_E_NULL;

    const char *key, *val, *val2;
    while ((key = va_arg(vp, const char *))) {
        val = va_arg(vp, const char *);
        if (!val) return HPD_E_NULL;
        hpd_map_get(map, key, &val2);
        if (!val2 || strcmp(val, val2) != 0) return HPD_E_NOT_FOUND;
    }
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_pair_get(const hpd_pair_t *pair, const char **key, const char **value)
{
    if (!pair || (!key && !value)) return HPD_E_NULL;

    if (key) (*key) = pair->k;
    if (value) (*value) = pair->v;
    return HPD_E_SUCCESS;
}
