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

#include <hpd/hpd_types.h>
#include <hpd/common/hpd_queue.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hpd_map hpd_map_t;
typedef struct hpd_pair hpd_pair_t;

hpd_error_t hpd_map_alloc(hpd_map_t **map);
hpd_error_t hpd_map_first(hpd_map_t *map, hpd_pair_t **pair);
hpd_error_t hpd_map_next(hpd_pair_t **pair);
hpd_error_t hpd_map_remove(hpd_map_t *map, hpd_pair_t *pair);
hpd_error_t hpd_map_free(hpd_map_t *map);
hpd_error_t hpd_map_get(hpd_map_t *map, const char *k, const char **v);
hpd_error_t hpd_map_get_n(hpd_map_t *map, const char *k, size_t k_len, const char **v);
hpd_error_t hpd_map_set(hpd_map_t *map, const char *k, const char *v);
hpd_error_t hpd_map_set_n(hpd_map_t *map, const char *k, size_t k_len, const char *v, size_t v_len);
hpd_error_t hpd_map_v_matches(hpd_map_t *map, va_list vp);
hpd_error_t hpd_pair_get(const hpd_pair_t *pair, const char **key, const char **value);

#define hpd_map_foreach(RC, PAIR, MAP) for ( \
    (RC) = hpd_map_first((MAP), &(PAIR)); \
    !(RC) && (PAIR); \
    (RC) = hpd_map_next(&(PAIR)))

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_HPD_MAP_H
