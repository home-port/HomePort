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

#ifndef HOMEPORT_HPD_MAP_H
#define HOMEPORT_HPD_MAP_H

#include "hpd_common.h"
#include "hpd_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hpd_map map_t;
typedef struct hpd_pair hpd_pair_t;

TAILQ_HEAD(hpd_map, hpd_pair);

struct hpd_pair {
    TAILQ_ENTRY(hpd_pair) HPD_TAILQ_FIELD; //< Tailq members
    char *k;                 //< Key (not null)
    char *v;                 //< Value
};

#define MAP_INIT(MAP) TAILQ_INIT((MAP))

#define MAP_FOREACH(VAR, HEAD) HPD_TAILQ_FOREACH((VAR), (HEAD))

#define MAP_REMOVE(MAP, PAIR) do { \
    TAILQ_REMOVE((MAP), (PAIR), HPD_TAILQ_FIELD); \
    free((PAIR)->k); \
    free((PAIR)->v); \
    free((PAIR)); \
} while(0)

#define MAP_FREE(MAP) do { \
    hpd_pair_t *attr, *tmp; \
    HPD_TAILQ_FOREACH_SAFE(attr, (MAP), tmp) { \
        MAP_REMOVE((MAP), attr); \
    } \
} while(0)

#define MAP_GET(MAP, K, V) do { \
    hpd_pair_t *attr; \
    (V) = NULL; \
    MAP_FOREACH(attr, (MAP)) \
        if (strcmp(attr->k, (K)) == 0) { \
            (V) = attr->v; \
            break; \
        } \
} while(0)

#define MAP_GET_LEN(MAP, K, K_LEN, V) do { \
    hpd_pair_t *attr; \
    (V) = NULL; \
    MAP_FOREACH(attr, (MAP)) \
        if (strncmp(attr->k, (K), (K_LEN)) == 0) { \
            (V) = attr->v; \
            break; \
        } \
} while(0)

// TODO Memory left in a inconsistent state if this fails with HPD_E_ALLOC
#define MAP_SET(MAP, K, V) do { \
    hpd_pair_t *attr = NULL; \
    MAP_FOREACH(attr, (MAP)) \
        if (strcmp(attr->k, (K)) == 0) \
            break; \
    if ((V) == NULL) { \
        if (attr) MAP_REMOVE((MAP), attr); \
    } else if (!attr) { \
        HPD_CALLOC(attr, 1, hpd_pair_t); \
        HPD_STR_CPY(attr->k, (K)); \
        HPD_STR_CPY(attr->v, (V)); \
        TAILQ_INSERT_TAIL((MAP), attr, tailq); \
    } else { \
        HPD_STR_CPY(attr->v, (V)); \
    } \
} while(0)

// TODO Memory left in a inconsistent state if this fails with HPD_E_ALLOC
#define MAP_SET_LEN(MAP, K, K_LEN, V, V_LEN) do { \
    hpd_pair_t *attr = NULL; \
    MAP_FOREACH(attr, (MAP)) \
        if (strncmp(attr->k, (K), (K_LEN)) == 0) \
            break; \
    if ((V) == NULL) { \
        if (attr) MAP_REMOVE((MAP), attr); \
    } else if (!attr) { \
        HPD_CALLOC(attr, 1, hpd_pair_t); \
        HPD_STR_N_CPY(attr->k, (K), (K_LEN)); \
        HPD_STR_N_CPY(attr->v, (V), (V_LEN)); \
        TAILQ_INSERT_TAIL((MAP), attr, tailq); \
    } else { \
        HPD_STR_N_CPY(attr->v, (V), (V_LEN)); \
    } \
} while(0)

#define MAP_MATCHES_VA(MAP, VP) do { \
    va_list _vp; \
    const char *key, *val, *val2; \
    va_copy(_vp, (VP)); \
    while ((key = va_arg(_vp, const char *))) { \
        val = va_arg(_vp, const char *); \
        if (!val) goto null_error; \
        MAP_GET((MAP), key, val2); \
        if (!val2 || strcmp(val, val2) != 0) goto mismatch; \
    } \
    goto match; \
} while(0)

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_HPD_MAP_H
