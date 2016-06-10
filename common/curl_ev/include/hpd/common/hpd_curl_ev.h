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

#ifndef HOMEPORT_HPD_CURL_EV_H
#define HOMEPORT_HPD_CURL_EV_H

#include <stddef.h>
#include <hpd/hpd_types.h>

typedef struct hpd_curl_ev_handle hpd_curl_ev_handle_t;

typedef size_t (*hpd_curl_ev_f)(char *buffer, size_t size, size_t nmemb, void *userdata);
typedef void (*hpd_curl_ev_free_f)(void *userdata);
typedef void (*hpd_curl_ev_done_f)(void *userdata, int curl_code);

hpd_error_t hpd_curl_ev_init(hpd_curl_ev_handle_t **handle, const hpd_module_t *context);
hpd_error_t hpd_curl_ev_cleanup(hpd_curl_ev_handle_t *handle);

hpd_error_t hpd_curl_ev_add_handle(hpd_curl_ev_handle_t *handle);
hpd_error_t hpd_curl_ev_remove_handle(hpd_curl_ev_handle_t *handle);

hpd_error_t hpd_curl_ev_set_header_callback(hpd_curl_ev_handle_t *handle, hpd_curl_ev_f on_header);
hpd_error_t hpd_curl_ev_set_body_callback(hpd_curl_ev_handle_t *handle, hpd_curl_ev_f on_body);
hpd_error_t hpd_curl_ev_set_done_callback(hpd_curl_ev_handle_t *handle, hpd_curl_ev_done_f on_done);

hpd_error_t hpd_curl_ev_set_custom_request(hpd_curl_ev_handle_t *handle, const char *request);
hpd_error_t hpd_curl_ev_set_data(hpd_curl_ev_handle_t *handle, void *data, hpd_curl_ev_free_f on_free);
hpd_error_t hpd_curl_ev_set_postfields(hpd_curl_ev_handle_t *handle, const void *data, size_t len);
hpd_error_t hpd_curl_ev_set_url(hpd_curl_ev_handle_t *handle, const char *url);
hpd_error_t hpd_curl_ev_set_verbose(hpd_curl_ev_handle_t *handle, long int bool);

hpd_error_t hpd_curl_ev_add_header(hpd_curl_ev_handle_t *handle, const char *header);

#endif
