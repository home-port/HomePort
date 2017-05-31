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

#include <hpd/common/hpd_serialize_shared.h>
#include <curl/curl.h>
#include <hpd/hpd_shared_api.h>
#include <stdlib.h>
#include <string.h>

hpd_error_t hpd_serialize_url_encode(const hpd_module_t *context, const char *decoded, char **encoded)
{
    CURL *curl;
    if (!(curl = curl_easy_init()))
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Could not initialise curl.");
    
    (*encoded) = curl_easy_escape(curl, decoded, 0);
    if (!(*encoded)) {
        curl_easy_cleanup(curl);
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Curl failed encoding url.");
    }

    curl_easy_cleanup(curl);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_serialize_url_decode(const hpd_module_t *context, const char *encoded, char **decoded)
{
    CURL *curl;
    if (!(curl = curl_easy_init()))
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Could not initialise curl.");

    (*decoded) = curl_easy_unescape(curl, encoded, 0, NULL);
    if (!(*decoded)) {
        curl_easy_cleanup(curl);
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Curl failed decoding url.");
    }

    curl_easy_cleanup(curl);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_serialize_url_create(const hpd_module_t *context, const hpd_service_id_t *service, char **url)
{
    hpd_error_t rc;

    const char *adp_id, *dev_id, *srv_id;
    if ((rc = hpd_service_id_get_service_id_str(service, &srv_id))) goto id_error;
    if ((rc = hpd_service_id_get_device_id_str(service, &dev_id))) goto id_error;
    if ((rc = hpd_service_id_get_adapter_id_str(service, &adp_id))) goto id_error;

    char *aid = NULL, *did = NULL, *sid = NULL;
    if ((rc = hpd_serialize_url_encode(context, adp_id, &aid))) goto encode_error;
    if ((rc = hpd_serialize_url_encode(context, dev_id, &did))) goto encode_error;
    if ((rc = hpd_serialize_url_encode(context, srv_id, &sid))) goto encode_error;

    (*url) = malloc((strlen(aid)+strlen(did)+strlen(sid)+3+1)*sizeof(char));
    if (!(*url)) {
        rc = HPD_E_ALLOC;
        goto encode_error;
    }
    (*url)[0] = '\0';

    strcat((*url), "/");
    strcat((*url), aid);
    strcat((*url), "/");
    strcat((*url), did);
    strcat((*url), "/");
    strcat((*url), sid);

    free(aid);
    free(did);
    free(sid);

    return HPD_E_SUCCESS;

    id_error:
    return rc;

    encode_error:
    free(aid);
    free(did);
    free(sid);
    return rc;
}
