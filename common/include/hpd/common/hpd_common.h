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

#ifndef HOMEPORT_HPD_COMMON_H
#define HOMEPORT_HPD_COMMON_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocates and zeros a structure.
 *
 * CAST is for c++ compatibility (tests).
 */
#define HPD_CALLOC(PTR, NUM, CAST) do { \
    (PTR) = (CAST *) calloc((NUM), sizeof(CAST)); \
    if(!(PTR)) goto alloc_error; \
} while(0)

/**
 * CAST is for c++ compatibility (tests).
 */
#define HPD_REALLOC(PTR, NUM, CAST) do { \
    void *_tmp = realloc((PTR), (NUM)*sizeof(CAST)); \
    if(!_tmp) goto alloc_error; \
    (PTR) = (CAST *) _tmp; \
} while(0)

#define HPD_CPY_ALLOC(DST, SRC, CAST) do { \
    (DST) = (CAST *) malloc(sizeof(CAST)); \
    if(!(DST)) goto alloc_error; \
    memcpy((DST), (SRC), sizeof(CAST)); \
} while(0)

#define HPD_STR_CPY(DST, SRC) do { \
    HPD_REALLOC((DST), (strlen((SRC))+1), char); \
    strcpy((DST), (SRC)); \
} while(0)

#define HPD_STR_N_CPY(DST, SRC, LEN) do { \
    HPD_REALLOC((DST), ((LEN)+1), char); \
    strncpy((DST), (SRC), (LEN)); \
    (DST)[LEN] = '\0'; \
} while(0)

#define HPD_SPRINTF_ALLOC(DST, FMT, ...) do { \
    size_t _len; \
    if ((_len = snprintf(NULL, 0, (FMT), ##__VA_ARGS__)) < 0) goto snprintf_error; \
    if (!((DST) = calloc(_len+1, sizeof(char)))) goto alloc_error; \
    if (snprintf((DST), _len+1, (FMT), ##__VA_ARGS__) < 0) { free((DST)); goto snprintf_error; } \
} while (0)

#define HPD_VSPRINTF_ALLOC(DST, FMT, VP) do { \
    size_t _len; \
    va_list _vp; \
    va_copy(_vp, (VP)); \
    if ((_len = vsnprintf(NULL, 0, (FMT), _vp)) < 0) { \
        va_end(_vp); \
        goto vsnprintf_error; \
    } \
    va_end(_vp); \
    if (!((DST) = calloc(_len+1, sizeof(char)))) goto alloc_error; \
    if (vsnprintf((DST), _len+1, (FMT), (VP)) < 0) { free((DST)); goto vsnprintf_error; } \
} while (0)

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_HPD_COMMON_H
