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

#ifndef HOMEPORT_COMMON_H
#define HOMEPORT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define HPD_TAILQ_FIELD tailq

/**
 * Allocates and zeros a structure.
 *
 * CAST is for c++ compatibility (tests).
 */
#define CALLOC(PTR, NUM, CAST) do { \
    PTR = (CAST *) calloc((NUM), sizeof(CAST)); \
    if(!PTR) goto alloc_error; \
} while(0)

/**
 * CAST is for c++ compatibility (tests).
 */
#define REALLOC(PTR, NUM, CAST) do { \
    PTR = (CAST *) realloc(PTR, (NUM)*sizeof(CAST)); \
    if(!PTR) goto alloc_error; \
} while(0)

#define STR_CPY(DST, SRC) do { \
    REALLOC(DST, (strlen(SRC)+1), char); \
    strcpy(DST, SRC); \
} while(0)

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_COMMON_H
