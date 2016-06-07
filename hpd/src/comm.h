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

#ifndef HOMEPORT_COMM_H
#define HOMEPORT_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hpd_map.h"

typedef struct hpd_listeners hpd_listeners_t;

TAILQ_HEAD(hpd_listeners, hpd_listener);

struct hpd_listener {
    // Navigational members
    TAILQ_ENTRY(hpd_listener) HPD_TAILQ_FIELD;
    hpd_t *hpd;
    // Data members
    hpd_value_f on_change;
    hpd_device_f on_attach;
    hpd_device_f on_detach;
    // User data
    void *data;
    hpd_free_f on_free;
};

struct hpd_request {
    hpd_service_id_t  *service;
    hpd_method_t    method;
    hpd_value_t    *value;
    // Callback and data for returning the response to sender
    hpd_response_f  on_response; // Nullable
    hpd_free_f      on_free;
    void       *data;
};

struct hpd_response {
    hpd_request_t  *request;
    hpd_status_t    status;
    hpd_value_t    *value;
};

struct hpd_value {
    hpd_map_t  *headers;
    char       *body;
    size_t      len;
};

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_COMM_H
