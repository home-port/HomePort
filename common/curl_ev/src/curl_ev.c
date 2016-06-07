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

#include "curl_ev_intern.h"
#include "hpd/common/hpd_common.h"
#include "hpd/hpd_shared_api.h"

static size_t curl_ev_on_header(char *buffer, size_t size, size_t nmemb, void *userdata)
{
    hpd_curl_ev_handle_t *handle = userdata;
    if (handle->on_header) return handle->on_header(buffer, size, nmemb, handle->data);
    else return size*nmemb;
}

static size_t curl_ev_on_body(char *buffer, size_t size, size_t nmemb, void *userdata)
{
    hpd_curl_ev_handle_t *handle = userdata;
    if (handle->on_body) return handle->on_body(buffer, size, nmemb, handle->data);
    else return size*nmemb;
}

hpd_error_t hpd_curl_ev_init(hpd_curl_ev_handle_t **handle, const hpd_module_t *context)
{
    if (!context) return HPD_E_NULL;
    if (!handle) HPD_LOG_RETURN_E_NULL(context);

    CURLcode cc;

    HPD_CALLOC(*handle, 1, hpd_curl_ev_handle_t);

    (*handle)->context = context;
    if (!((*handle)->handle = curl_easy_init())) goto init_error;
    if ((cc = curl_easy_setopt((*handle)->handle, CURLOPT_HEADERFUNCTION, curl_ev_on_header))) goto curl_error;
    if ((cc = curl_easy_setopt((*handle)->handle, CURLOPT_HEADERDATA, *handle))) goto curl_error;
    if ((cc = curl_easy_setopt((*handle)->handle, CURLOPT_WRITEFUNCTION, curl_ev_on_body))) goto curl_error;
    if ((cc = curl_easy_setopt((*handle)->handle, CURLOPT_WRITEDATA, *handle))) goto curl_error;

    return HPD_E_SUCCESS;

    curl_error:
        curl_easy_cleanup((*handle)->handle);
        free(handle);
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Curl returned an error [code: %i]", cc);

    init_error:
        free(handle);
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Curl init error");
        
    alloc_error:
        HPD_LOG_RETURN_E_ALLOC(context);
}

hpd_error_t hpd_curl_ev_cleanup(hpd_curl_ev_handle_t *handle)
{
    if (!handle) return HPD_E_NULL;
    if (handle->curl_ev) HPD_LOG_RETURN(handle->context, HPD_E_STATE, "Handle is still attached");

    if (handle->headers) curl_slist_free_all(handle->headers);
    if (handle->handle) curl_easy_cleanup(handle->handle);
    if (handle->on_free) handle->on_free(handle->data);
    free(handle);
    
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_curl_ev_set_header_callback(hpd_curl_ev_handle_t *handle, hpd_curl_ev_f on_header)
{
    if (!handle) return HPD_E_NULL;
    handle->on_header = on_header;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_curl_ev_set_body_callback(hpd_curl_ev_handle_t *handle, hpd_curl_ev_f on_body)
{
    if (!handle) return HPD_E_NULL;
    handle->on_body = on_body;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_curl_ev_set_done_callback(hpd_curl_ev_handle_t *handle, hpd_curl_ev_done_f on_done)
{
    if (!handle) return HPD_E_NULL;
    handle->on_done = on_done;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_curl_ev_set_custom_request(hpd_curl_ev_handle_t *handle, const char *request)
{
    if (!handle) return HPD_E_NULL;
    CURLcode cc;
    if ((cc = curl_easy_setopt(handle->handle, CURLOPT_CUSTOMREQUEST, request)))
        HPD_LOG_RETURN(handle->context, HPD_E_UNKNOWN, "Curl failed [code: %i]", cc);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_curl_ev_set_data(hpd_curl_ev_handle_t *handle, void *data, hpd_curl_ev_free_f on_free)
{
    if (!handle) return HPD_E_NULL;
    handle->data = data;
    handle->on_free = on_free;
    return HPD_E_SUCCESS;
}

/**
 * HPD_E_UNKNOWN: handle may be in an inconsistent state, and should not be added before a call to this function succeded.
 */
hpd_error_t hpd_curl_ev_set_postfields(hpd_curl_ev_handle_t *handle, const void *data, size_t len)
{
    if (!handle) return HPD_E_NULL;
    CURLcode cc;
    if ((cc = curl_easy_setopt(handle->handle, CURLOPT_POSTFIELDSIZE, len)))
        HPD_LOG_RETURN(handle->context, HPD_E_UNKNOWN, "Curl failed [code: %i]", cc);
    if ((cc = curl_easy_setopt(handle->handle, CURLOPT_COPYPOSTFIELDS, data)))
        HPD_LOG_RETURN(handle->context, HPD_E_UNKNOWN, "Curl failed [code: %i]", cc);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_curl_ev_set_url(hpd_curl_ev_handle_t *handle, const char *url)
{
    if (!handle) return HPD_E_NULL;
    CURLcode cc;
    if ((cc = curl_easy_setopt(handle->handle, CURLOPT_URL, url)))
        HPD_LOG_RETURN(handle->context, HPD_E_UNKNOWN, "Curl failed [code: %i]", cc);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_curl_ev_set_verbose(hpd_curl_ev_handle_t *handle, long int bool)
{
    if (!handle) return HPD_E_NULL;
    CURLcode cc;
    if ((cc = curl_easy_setopt(handle->handle, CURLOPT_VERBOSE, bool)))
        HPD_LOG_RETURN(handle->context, HPD_E_UNKNOWN, "Curl failed [code: %i]", cc);

    return HPD_E_SUCCESS;
}

/**
 * HPD_E_UNKNOWN: handle may be in an inconsistent state, and should not be added before a call to this function succeded.
 */
hpd_error_t hpd_curl_ev_add_header(hpd_curl_ev_handle_t *handle, const char *header)
{
    if (!handle) return HPD_E_NULL;
    CURLcode cc;
    struct curl_slist *headers = curl_slist_append(handle->headers, header);
    if (!headers) HPD_LOG_RETURN(handle->context, HPD_E_UNKNOWN, "Curl failed");
    handle->headers = headers;
    if ((cc = curl_easy_setopt(handle->handle, CURLOPT_HTTPHEADER, handle->headers)))
        HPD_LOG_RETURN(handle->context, HPD_E_UNKNOWN, "Curl failed [code: %i]", cc);
    return HPD_E_SUCCESS;
}
