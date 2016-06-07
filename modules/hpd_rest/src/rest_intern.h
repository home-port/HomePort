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

#ifndef HOMEPORT_REST_INTERN_H
#define HOMEPORT_REST_INTERN_H

#include "hpd_types.h"

typedef struct hpd_rest hpd_rest_t;

hpd_error_t hpd_rest_url_create(hpd_rest_t *rest, hpd_service_id_t *service, char **url);
hpd_error_t hpd_rest_get_timestamp(const hpd_module_t *context, char *str);

// ALL keys starts with _ to avoid conflicts with adapter provided ones..
static const char * const HPD_REST_KEY_ID = "_id";
static const char * const HPD_REST_KEY_URI = "_uri";
static const char * const HPD_REST_KEY_GET = "_get";
static const char * const HPD_REST_KEY_PUT = "_put";
static const char * const HPD_REST_KEY_PARAMETER = "_parameter";
static const char * const HPD_REST_KEY_SERVICE = "_service";
static const char * const HPD_REST_KEY_DEVICE = "_device";
static const char * const HPD_REST_KEY_URL_ENCODED_CHARSET = "_urlEncodedCharset";
static const char * const HPD_REST_KEY_ADAPTER = "_adapter";
static const char * const HPD_REST_KEY_VALUE = "_value";
static const char * const HPD_REST_KEY_CONFIGURATION = "_configuration";
static const char * const HPD_REST_KEY_TIMESTAMP = "_timestamp";

static const char * const HPD_REST_VAL_TRUE = "1";
static const char * const HPD_REST_VAL_ASCII = "ASCII";

#endif //HOMEPORT_HPD_REST_INTERN_H
