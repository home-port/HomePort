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

#include "hpd_rest_intern.h"
#include "hpd_rest.h"
#include "hpd_application_api.h"
#include "hpd_common.h"
#include "httpd.h"
#include <curl/curl.h>
#include "json.h"
#include "xml.h"

static hpd_error_t on_create(void **data, hpd_module_t *context);
static hpd_error_t on_destroy(void *data);
static hpd_error_t on_start(void *data, hpd_t *hpd);
static hpd_error_t on_stop(void *data, hpd_t *hpd);
static hpd_error_t on_parse_opt(void *data, const char *name, const char *arg);

hpd_module_def_t hpd_rest = { on_create, on_destroy, on_start, on_stop, on_parse_opt };

typedef enum content_type {
    CONTENT_UNKNOWN = -1,
    CONTENT_NONE,
    CONTENT_XML,
    CONTENT_JSON,
} content_type_t;

struct hpd_rest {
    hpd_httpd_t *ws;
    hpd_httpd_settings_t ws_set;
    hpd_t *hpd;
    hpd_module_t *context;
    CURL *curl;
};

typedef struct hpd_rest_req {
    hpd_rest_t *rest;

    // The HTTP request and data from it
    hpd_httpd_request_t *http_req;
    hpd_httpd_method_t http_method;
    const char *url;
    hpd_map_t *headers;
    char *body;
    size_t len;

    // Request data converted to hpd notation
    hpd_method_t hpd_method;
    hpd_service_id_t *service;
    hpd_request_t *hpd_request;

    // The HTTP response (if we sent any yet)
    hpd_httpd_response_t *http_res;
} hpd_rest_req_t;

static content_type_t media_type_to_enum(const char *haystack)
{
    char *str;

    if (haystack == NULL)
        return CONTENT_NONE;
    if (    strcmp(haystack, "application/xml") == 0 ||
            strncmp(haystack, "application/xml;", 16) == 0 ||
            ((str = strstr(haystack, ",application/xml")) && (str[16] == ';' || str[16] == '\0')))
        return CONTENT_XML;
    if (    strcmp(haystack, "application/json") == 0 ||
            strncmp(haystack, "application/json;", 17) == 0 ||
            ((str = strstr(haystack, ",application/json")) && (str[17] == ';' || str[17] == '\0')))
        return CONTENT_JSON;
    return CONTENT_UNKNOWN;
}

static hpd_error_t url_encode(hpd_rest_t *rest, const char *decoded, char **encoded)
{
    (*encoded) = curl_easy_escape(rest->curl, decoded, 0);
    if (!(*encoded)) HPD_LOG_RETURN(rest->context, HPD_E_UNKNOWN, "Curl failed encoding url.");
    return HPD_E_SUCCESS;
}

static hpd_error_t url_decode(hpd_rest_t *rest, const char *encoded, char **decoded)
{
    (*decoded) = curl_easy_unescape(rest->curl, encoded, 0, NULL);
    if (!(*decoded)) HPD_LOG_RETURN(rest->context, HPD_E_UNKNOWN, "Curl failed decoding url.");
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_rest_url_create(hpd_rest_t *rest, hpd_service_id_t *service, char **url)
{
    hpd_error_t rc;

    hpd_adapter_id_t *adapter = NULL;
    hpd_device_id_t *device = NULL;
    if ((rc = hpd_service_get_adapter(service, &adapter))) goto id_error;
    if ((rc = hpd_service_get_device(service, &device))) goto id_error;

    const char *adp_id, *dev_id, *srv_id;
    if ((rc = hpd_service_get_id(service, &srv_id))) goto id_error;
    if ((rc = hpd_device_get_id(device, &dev_id))) goto id_error;
    if ((rc = hpd_adapter_get_id(adapter, &adp_id))) goto id_error;

    if ((rc = hpd_device_id_free(device))) {
        device = NULL;
        goto id_error;
    }
    device = NULL;
    if ((rc = hpd_adapter_id_free(adapter))) {
        adapter = NULL;
        goto id_error;
    }
    adapter = NULL;

    char *aid = NULL, *did = NULL, *sid = NULL;
    if ((rc = url_encode(rest, adp_id, &aid))) goto encode_error;
    if ((rc = url_encode(rest, dev_id, &did))) goto encode_error;
    if ((rc = url_encode(rest, srv_id, &sid))) goto encode_error;

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
    hpd_adapter_id_free(adapter);
    hpd_device_id_free(device);
    return rc;

    encode_error:
    free(aid);
    free(did);
    free(sid);
    return rc;
}

// TODO Should probably double-check this one...
static hpd_error_t url_extract(hpd_rest_t *rest, const char *url, char **aid, char **did, char **sid)
{
    const char *aid_p, *did_p, *sid_p;
    size_t aid_l, did_l, sid_l;

    if (strlen(url) < 2)
        HPD_LOG_RETURN(rest->context, HPD_E_ARGUMENT, "URL Parse error.");
    if (url[0] != '/')
        HPD_LOG_RETURN(rest->context, HPD_E_ARGUMENT, "URL Parse error.");

    aid_p = &url[1];
    if (aid_p[0] == '\0' || aid_p[0] == '/')
        HPD_LOG_RETURN(rest->context, HPD_E_ARGUMENT, "URL Parse error.");
    for (aid_l = 0; aid_p[aid_l] != '/'; aid_l ++)
        if (aid_p[aid_l] == '\0')
            HPD_LOG_RETURN(rest->context, HPD_E_ARGUMENT, "URL Parse error.");

    did_p = &aid_p[aid_l+1];
    if (did_p[0] == '\0' || did_p[0] == '/')
        HPD_LOG_RETURN(rest->context, HPD_E_ARGUMENT, "URL Parse error.");
    for (did_l = 0; did_p[did_l] != '/'; did_l ++)
        if (did_p[did_l] == '\0')
            HPD_LOG_RETURN(rest->context, HPD_E_ARGUMENT, "URL Parse error.");

    sid_p = &did_p[did_l+1];
    if (sid_p[0] == '\0' || sid_p[0] == '/')
        HPD_LOG_RETURN(rest->context, HPD_E_ARGUMENT, "URL Parse error.");
    for (sid_l = 0; sid_p[sid_l] != '/' && sid_p[sid_l] != '\0'; sid_l++);

    if (!sid_p[sid_l] == '\0')
        HPD_LOG_RETURN(rest->context, HPD_E_ARGUMENT, "URL Parse error.");

    *aid = NULL;
    *did = NULL;
    *sid = NULL;
    HPD_STR_N_CPY(*aid, aid_p, aid_l);
    HPD_STR_N_CPY(*did, did_p, did_l);
    HPD_STR_N_CPY(*sid, sid_p, sid_l);

    return HPD_E_SUCCESS;

    alloc_error:
    free(*aid);
    free(*did);
    free(*sid);
    return HPD_E_ALLOC;
}

static hpd_error_t reply(hpd_httpd_request_t *req, enum hpd_status status, hpd_rest_req_t *rest_req,
                         hpd_module_t *context)
{
    hpd_error_t rc;

    if (rest_req && rest_req->http_res)
        HPD_LOG_RETURN(rest_req->rest->context, HPD_E_STATE, "Already replied to request.");

    // Check status code
    switch (status) {
#define XX(NUM, STR) case HPD_S_##NUM: break;
        HPD_HTTP_STATUS_CODE_MAP(XX)
#undef XX
        default:
            if (context) HPD_LOG_WARN(context, "Unknown HTTP Status code.");
            status = HPD_S_500;
    }

    hpd_httpd_response_t *res;
    if ((rc = hpd_httpd_response_create(&res, req, status))) return rc;

    switch (status) {
#define XX(NUM, STR) case HPD_S_##NUM: rc = hpd_httpd_response_sendf(res, #STR); break;
        HPD_HTTP_STATUS_CODE_MAP(XX)
#undef XX
    }

    if (rc) {
        hpd_httpd_response_destroy(res);
        return rc;
    }

    if (rest_req) rest_req->http_res = res;
    return hpd_httpd_response_destroy(res);
}

static hpd_error_t reply_internal_server_error(hpd_httpd_request_t *req, hpd_rest_req_t *rest_req, hpd_module_t *context)
{
    return reply(req, HPD_S_500, rest_req, context);
}

static hpd_error_t reply_not_found(hpd_httpd_request_t *req, hpd_rest_req_t *rest_req, hpd_module_t *context)
{
    return reply(req, HPD_S_404, rest_req, context);
}

static hpd_error_t reply_unsupported_media_type(hpd_httpd_request_t *req, hpd_rest_req_t *rest_req, hpd_module_t *context)
{
    return reply(req, HPD_S_415, rest_req, context);
}

static hpd_error_t reply_bad_request(hpd_httpd_request_t *req, hpd_rest_req_t *rest_req, hpd_module_t *context)
{
    return reply(req, HPD_S_400, rest_req, context);
}

static hpd_error_t reply_method_not_allowed(hpd_httpd_request_t *req, hpd_rest_req_t *rest_req, hpd_module_t *context)
{
    return reply(req, HPD_S_405, rest_req, context);
}

static hpd_error_t reply_devices(hpd_rest_req_t *rest_req)
{
    hpd_error_t rc, rc2;
    hpd_httpd_request_t *http_req = rest_req->http_req;
    hpd_rest_t *rest = rest_req->rest;
    hpd_module_t *context = rest->context;

    if (rest_req->http_res) HPD_LOG_RETURN(context, HPD_E_STATE, "Response already sent.");

    // Get Accept header
    const char *accept;
    if ((rc = hpd_map_get(rest_req->headers, "Accept", &accept))) return rc;

    // Create body
    char *body;
    switch (media_type_to_enum(accept)) {
        case CONTENT_NONE:
        case CONTENT_XML:
            body = xmlGetConfiguration(rest, rest->hpd);
            break;
        case CONTENT_JSON:
            body = jsonGetConfiguration(rest, rest->hpd);
            break;
        case CONTENT_UNKNOWN:
        default:
            if ((rc = reply_unsupported_media_type(http_req, rest_req, context))) {
                HPD_LOG_ERROR(context, "Failed to send unsupported media type response (code: %d).", rc);
            }
            return HPD_E_SUCCESS;
    }

    // Send response
    if ((rc = hpd_httpd_response_create(&rest_req->http_res, http_req, HPD_S_200))) goto create_error;
    if ((rc = hpd_httpd_response_sendf(rest_req->http_res, "%s", body))) goto response_error;
    if ((rc = hpd_httpd_response_destroy(rest_req->http_res))) goto create_error;

    free(body);

    return rc;

    response_error:
        if ((rc2 = hpd_httpd_response_destroy(rest_req->http_res)))
            HPD_LOG_ERROR(context, "Failed to destroy response (code: %d).", rc2);
        rest_req->http_res = NULL;
    create_error:
        free(body);
        if ((rc2 = reply_internal_server_error(http_req, rest_req, context)))
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
    return rc;
}

#ifdef HPD_REST_ORIGIN
static hpd_error_t reply_options(hpd_rest_req_t *rest_req)
{
    hpd_error_t rc, rc2;
    hpd_httpd_request_t *http_req = rest_req->http_req;
    hpd_module_t *context = rest_req->rest->context;

    if (rest_req->http_res) HPD_LOG_RETURN(context, HPD_E_STATE, "Response already sent.");
    
    // Construct methods list
    char methods[23];
    methods[0] = '\0';
    if (strcmp(rest_req->url, "/devices") == 0) {
        strcat(methods, "GET");
    } else {
        hpd_action_t *action;
        hpd_service_foreach_action(rc, action, rest_req->service) {
            hpd_method_t method;
            if ((rc = hpd_action_get_method(action, &method))) {
                if ((rc2 = reply_internal_server_error(http_req, rest_req, context))) {
                    HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
                }
                return rc;
            }
            switch (method) {
                case HPD_M_GET:
                    if (strlen(methods) > 0) strcat(methods, ", ");
                    strcat(methods, "GET");
                    break;
                case HPD_M_PUT:
                    if (strlen(methods) > 0) strcat(methods, ", ");
                    strcat(methods, "PUT");
                    break;
                case HPD_M_NONE:
                case HPD_M_COUNT:
                    HPD_LOG_ERROR(context, "Unexpected method.");
                    if ((rc2 = reply_internal_server_error(http_req, rest_req, context))) {
                        HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
                    }
                    return rc;
            }
        }
        switch (rc) {
            case HPD_E_SUCCESS:
                break;
            case HPD_E_NOT_FOUND:
                if ((rc2 = reply_not_found(http_req, rest_req, context))) {
                    HPD_LOG_ERROR(context, "Failed to send not found response (code: %d).", rc2);
                    return rc2;
                }
                return HPD_E_SUCCESS;
            default:
                HPD_LOG_ERROR(context, "Failed to loop over actions (code: %d).", rc);
                if ((rc2 = reply_internal_server_error(http_req, rest_req, context))) {
                    HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
                }
                return rc;
        }
    }

    if ((rc = hpd_httpd_response_create(&rest_req->http_res, http_req, HPD_S_200))) goto create_error;
    if ((rc = hpd_httpd_response_add_header(rest_req->http_res, "Access-Control-Allow-Origin", "*"))) goto response_error;
    if ((rc = hpd_httpd_response_add_header(rest_req->http_res, "Access-Control-Allow-Methods", methods))) goto response_error;
    if ((rc = hpd_httpd_response_add_header(rest_req->http_res, "Access-Control-Allow-Headers", "Content-Type, Cache-Control, Accept, X-Requested-With"))) goto response_error;
    if ((rc = hpd_httpd_response_sendf(rest_req->http_res, "OK"))) goto response_error;
    if ((rc = hpd_httpd_response_destroy(rest_req->http_res))) goto create_error;
    return HPD_E_SUCCESS;

    response_error:
        if ((rc2 = hpd_httpd_response_destroy(rest_req->http_res)))
            HPD_LOG_ERROR(context, "Failed to destroy response (code: %d).", rc2);
        rest_req->http_res = NULL;
    create_error:
        if ((rc2 = reply_internal_server_error(http_req, rest_req, context)))
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        return rc;
}
#endif

static hpd_error_t on_free(void *data)
{
    hpd_error_t rc = HPD_E_SUCCESS;
    hpd_rest_req_t *rest_req = data;

    if (rest_req->http_req) {
        if (!rest_req->http_res) {
            rc = reply_internal_server_error(rest_req->http_req, rest_req, rest_req->rest->context);
        }
        rest_req->hpd_request = NULL;
        return rc;
    } else {
        rc = hpd_service_id_free(rest_req->service);
        free(rest_req->body);
        free(rest_req);
        return rc;
    }
}

static hpd_httpd_return_t on_req_destroy(hpd_httpd_t *ins, hpd_httpd_request_t *req, void* ws_ctx, void** req_data)
{
    hpd_error_t rc;
    hpd_rest_req_t *rest_req = *req_data;

    if (!rest_req) return HPD_HTTPD_R_CONTINUE;

    if (rest_req->hpd_request) {
        rest_req->http_req = NULL;
        return HPD_HTTPD_R_CONTINUE;
    } else {
        if (rest_req->service && (rc = hpd_service_id_free(rest_req->service)))
            HPD_LOG_ERROR(rest_req->rest->context, "Failed to free service id.");
        free(rest_req->body);
        free(rest_req);
        return rc ? HPD_HTTPD_R_STOP : HPD_HTTPD_R_CONTINUE;
    }
}

static hpd_error_t on_response(hpd_response_t *res)
{
    hpd_error_t rc, rc2;

    // Get rest request
    hpd_rest_req_t *rest_req;
    if ((rc = hpd_response_get_request_data(res, (void **) &rest_req))) return rc;
    
    // Make life easier
    hpd_httpd_request_t *http_req = rest_req->http_req;
    hpd_module_t *context = rest_req->rest->context;
    
    // Check if closed from downstairs
    if (!http_req) return HPD_E_SUCCESS;

    // Get data from hpd
    hpd_value_t *value;
    const char *val;
    size_t len;
    hpd_status_t status;
    if ((rc = hpd_response_get_value(res, &value)) ||
        (rc = hpd_value_get_body(value, &val, &len)) ||
        (rc = hpd_response_get_status(res, &status))) {
        if ((rc2 = reply_internal_server_error(http_req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        return rc;
    }
    
    // Check val from hpd
    if (!val) {
        if ((rc2 = reply(http_req, status, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send status response (code: %d).", rc2);
        }
        return HPD_E_ARGUMENT;
    }

    // Get data from httpd
    const char *accept;
    content_type_t accept_type;
    switch ((rc = hpd_map_get(rest_req->headers, "Accept", &accept))) {
        case HPD_E_SUCCESS:
            break;
        case HPD_E_NOT_FOUND:
            accept = NULL;
            break;
        default:
            if ((rc2 = reply_internal_server_error(http_req, rest_req, context))) {
                HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
            }
            return rc;
    }
    switch ((accept_type = media_type_to_enum(accept))) {
        case CONTENT_NONE:break;
        case CONTENT_XML:break;
        case CONTENT_JSON:break;
        case CONTENT_UNKNOWN:
            if ((rc2 = reply_unsupported_media_type(http_req, rest_req, context))) {
                HPD_LOG_ERROR(context, "Failed to send unsupported media type response (code: %d).", rc2);
            }
            return HPD_E_SUCCESS;
    }

    // Null terminate val
    char *body = NULL;
    body = malloc((len+1) * sizeof(char));
    if (!body) {
        if ((rc2 = reply_internal_server_error(http_req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        HPD_LOG_RETURN_E_ALLOC(context);
    }
    strncpy(body, val, len);
    body[len] = '\0';

    if (rest_req->http_res) {
        HPD_LOG_ERROR(context, "Response already sent.");
        rc = HPD_E_STATE;
        goto error_free_body;
    }
    
    // Create response
    if ((rc = hpd_httpd_response_create(&rest_req->http_res, http_req, status))) goto error_send_res;
    hpd_httpd_response_t *http_res = rest_req->http_res;
    char *state = NULL;
    switch (accept_type) {
        case CONTENT_NONE:
        case CONTENT_XML:
            state = xmlGetState(body); // TODO Check error
            if ((rc = hpd_httpd_response_add_header(http_res, "Content-Type", "application/xml"))) goto error_free_state;
            break;
        case CONTENT_JSON:
            state = jsonGetState(body); // TODO Check error
            if ((rc = hpd_httpd_response_add_header(http_res, "Content-Type", "application/json"))) goto error_free_state;
            break;
        default:
            HPD_LOG_ERROR(context, "Should definitely not be here.");
            goto error_free_res;
    }
#ifdef HPD_REST_ORIGIN
    if ((rc = hpd_httpd_response_add_header(http_res, "Access-Control-Allow-Origin", "*"))) goto error_free_state;
#endif
    if ((rc = hpd_httpd_response_sendf(http_res, "%s", state))) goto error_free_state;
    rc = hpd_httpd_response_destroy(http_res);

    // Clean up
    free(state);
    free(body);

    return rc;

    error_free_state:
        free(state);
    error_free_res:
        if ((rc2 = hpd_httpd_response_destroy(http_res))) {
            HPD_LOG_ERROR(context, "Failed to destroy response (code: %d).", rc2);
        }
    error_send_res:
        rest_req->http_res = NULL;
        if ((rc2 = reply_internal_server_error(http_req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
    error_free_body:
        free(body);
        return rc;
}

static hpd_httpd_return_t on_req_begin(hpd_httpd_t *httpd, hpd_httpd_request_t *req, void* httpd_ctx, void** req_data)
{
    hpd_error_t rc;
    struct hpd_rest *rest = httpd_ctx;
    hpd_module_t *context = rest->context;
    
    hpd_rest_req_t *rest_req;
    HPD_CALLOC(rest_req, 1, hpd_rest_req_t);
    rest_req->rest = rest;
    rest_req->http_req = req;
    *req_data = rest_req;

    return HPD_HTTPD_R_CONTINUE;
    
    alloc_error:
    if ((rc = reply_internal_server_error(req, NULL, rest->context))) {
        HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc);
    };
    return HPD_HTTPD_R_STOP;
}

static hpd_httpd_return_t on_req_url_cmpl(hpd_httpd_t *ins, hpd_httpd_request_t *req, void* httpd_ctx, void** req_data)
{
    hpd_error_t rc, rc2;
    hpd_rest_req_t *rest_req = *req_data;
    struct hpd_rest *rest = httpd_ctx;
    hpd_module_t *context = rest->context;

    // Get url
    if ((rc = hpd_httpd_request_get_url(req, &rest_req->url))) {
        HPD_LOG_ERROR(context, "Failed to get url (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }
    if (!rest_req->url) {
        if ((rc2 = reply_not_found(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send not found response (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }

    // If /devices its okay
    if (strcmp(rest_req->url, "/devices") == 0) return HPD_HTTPD_R_CONTINUE;

    // Get components
    char *aid_encoded, *did_encoded, *sid_encoded;
    switch ((rc = url_extract(rest, rest_req->url, &aid_encoded, &did_encoded, &sid_encoded))) {
        case HPD_E_ALLOC:
            HPD_LOG_ERROR(context, "Unable to allocate memory.");
            if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
                HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
            }
            return HPD_HTTPD_R_STOP;
        case HPD_E_ARGUMENT:
            if ((rc2 = reply_not_found(req, rest_req, context))) {
                HPD_LOG_ERROR(context, "Failed to send not found response (code: %d).", rc2);
            }
            return HPD_HTTPD_R_STOP;
        case HPD_E_SUCCESS:
            break;
        default:
            HPD_LOG_ERROR(context, "Unexpected error (code: %d).", rc);
            if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
                HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
            }
            return HPD_HTTPD_R_STOP;
    }

    // Decode IDs
    char *aid_decoded, *did_decoded, *sid_decoded;
    if ((rc = url_decode(rest, aid_encoded, &aid_decoded))) {
        HPD_LOG_ERROR(context, "Failed to decode id (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }
    if ((rc = url_decode(rest, did_encoded, &did_decoded))) {
        HPD_LOG_ERROR(context, "Failed to decode id (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }
    if ((rc = url_decode(rest, sid_encoded, &sid_decoded))) {
        HPD_LOG_ERROR(context, "Failed to decode id (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }
    free(aid_encoded);
    free(did_encoded);
    free(sid_encoded);

    // Create service ID
    if ((rc = hpd_service_id_alloc(&rest_req->service, rest->hpd, aid_decoded, did_decoded, sid_decoded))) {
        HPD_LOG_ERROR(context, "Failed to allocate id (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }
    free(aid_decoded);
    free(did_decoded);
    free(sid_decoded);

    return HPD_HTTPD_R_CONTINUE;
}

static hpd_method_t method_to_method(hpd_httpd_method_t method)
{
    switch(method)
    {
        case HPD_HTTPD_M_GET:
            return HPD_M_GET;
        case HPD_HTTPD_M_PUT:
            return HPD_M_PUT;
        case HPD_HTTPD_M_OPTIONS:
        case HPD_HTTPD_M_UNKNOWN:
        default:
            return HPD_M_NONE;
    }
}

static hpd_httpd_return_t on_req_hdr_cmpl(hpd_httpd_t *ins, hpd_httpd_request_t *req, void *httpd_ctx, void **req_data)
{
    hpd_error_t rc, rc2;
    hpd_rest_req_t *rest_req = *req_data;
    struct hpd_rest *rest = httpd_ctx;
    hpd_module_t *context = rest->context;

    // Get headers for later
    if ((rc = hpd_httpd_request_get_headers(req, &rest_req->headers))) {
        HPD_LOG_ERROR(context, "Failed to get headers (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }

    // Get method
    if ((rc = hpd_httpd_request_get_method(req, &rest_req->http_method))) {
        HPD_LOG_ERROR(context, "Failed to get method (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }
    rest_req->hpd_method = method_to_method(rest_req->http_method);

    // Act on method
    switch (rest_req->http_method) {
        case HPD_HTTPD_M_GET:
        case HPD_HTTPD_M_PUT: {
            if (strcmp(rest_req->url, "/devices") == 0) {
                if ((rc = reply_devices(rest_req))) {
                    HPD_LOG_ERROR(context, "Failed to reply with devices list (code: %d).", rc);
                    if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
                        HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
                    }
                }
                return HPD_HTTPD_R_STOP;
            }

            hpd_bool_t found;
            switch (hpd_service_has_action(rest_req->service, rest_req->hpd_method, &found)) {
                case HPD_E_SUCCESS:
                    break;
                case HPD_E_NOT_FOUND:
                    if ((rc2 = reply_not_found(req, rest_req, context))) {
                        HPD_LOG_ERROR(context, "Failed to send not found response (code: %d).", rc2);
                    }
                    return HPD_HTTPD_R_STOP;
                default:
                    if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
                        HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
                    }
                    return HPD_HTTPD_R_STOP;
            };
            if (!found) {
                if ((rc2 = reply_method_not_allowed(req, rest_req, context))) {
                    HPD_LOG_ERROR(context, "Failed to send method not allowed response (code: %d).", rc2);
                }
                return HPD_HTTPD_R_STOP;
            }
            return HPD_HTTPD_R_CONTINUE;
        }
#ifdef HPD_REST_ORIGIN
        case HPD_HTTPD_M_OPTIONS: {
            if ((rc = reply_options(rest_req))) {
                HPD_LOG_ERROR(context, "Failed to reply with devices list (code: %d).", rc);
                if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
                    HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
                }
            }
            return HPD_HTTPD_R_STOP;
        }
#endif
        case HPD_HTTPD_M_UNKNOWN:
        default:
            if ((rc2 = reply_method_not_allowed(req, rest_req, context))) {
                HPD_LOG_ERROR(context, "Failed to send method not allowed response (code: %d).", rc2);
            }
            return HPD_HTTPD_R_STOP;
    }
}

static hpd_httpd_return_t on_req_body(hpd_httpd_t *ins, hpd_httpd_request_t *req, void* httpd_ctx, void** req_data, const char* chunk, size_t len)
{
    hpd_error_t rc2;
    hpd_rest_req_t *rest_req = *req_data;
    hpd_module_t *context = rest_req->rest->context;

    HPD_REALLOC(rest_req->body, rest_req->len + len, char);
    strncpy(&rest_req->body[rest_req->len], chunk, len);
    rest_req->len += len;
    return HPD_HTTPD_R_CONTINUE;

    alloc_error:
    HPD_LOG_ERROR(context, "Unable to allocate memory.");
    if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
        HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
    }
    return HPD_HTTPD_R_STOP;
}

static hpd_httpd_return_t on_req_cmpl(hpd_httpd_t *ins, hpd_httpd_request_t *req, void* httpd_ctx, void** req_data)
{
    hpd_error_t rc, rc2;
    hpd_rest_req_t *rest_req = *req_data;
    hpd_module_t *context = rest_req->rest->context;
    hpd_service_id_t *service = rest_req->service;

    // Construct value
    hpd_value_t *value = NULL;
    if (rest_req->body) {
        // Get content type
        const char *content_type;
        switch ((rc = hpd_map_get(rest_req->headers, "Content-Type", &content_type))) {
            case HPD_E_SUCCESS:
                break;
            case HPD_E_NOT_FOUND:
                content_type = NULL;
                break;
            default:
                HPD_LOG_ERROR(context, "Failed to get content-type (code: %d).", rc);
                if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
                    HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
                }
                return HPD_HTTPD_R_STOP;
        }

        // Construct actual value
        const char *val;
        switch (media_type_to_enum(content_type)) {
            case CONTENT_NONE:
            case CONTENT_XML:
                val = xmlParseState(rest_req->body);
                break;
            case CONTENT_JSON:
                val = jsonParseState(rest_req->body);
                break;
            case CONTENT_UNKNOWN:
            default:
                if ((rc2 = reply_unsupported_media_type(req, rest_req, context))) {
                    HPD_LOG_ERROR(context, "Failed to send unsupported media type response (code: %d).", rc2);
                }
                return HPD_HTTPD_R_STOP;
        }

        // Check for parse errors
        // TODO Does this become obsolete then updating xml and json layers?
        if (!val) {
            if ((rc2 = reply_bad_request(req, rest_req, context))) {
                HPD_LOG_ERROR(context, "Failed to send bad request response (code: %d).", rc2);
            }
            return HPD_HTTPD_R_STOP;
        }

        // Allocate hpd value
        if ((rc = hpd_value_alloc(&value, val, HPD_NULL_TERMINATED))) {
            HPD_LOG_ERROR(context, "Unable to allocate value (code: %d).", rc);
            if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
                HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
            }
            return HPD_HTTPD_R_STOP;
        }
    }

    // Send hpd request
    if ((rc = hpd_request_alloc(&rest_req->hpd_request, service, rest_req->hpd_method, on_response))) {
        HPD_LOG_ERROR(context, "Unable to allocate request (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }
    if (value && (rc = hpd_request_set_value(rest_req->hpd_request, value))) {
        HPD_LOG_ERROR(context, "Unable to set value (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        if ((rc2 = hpd_request_free(rest_req->hpd_request))) {
            HPD_LOG_ERROR(context, "Failed to free request (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }
    if ((rc = hpd_request_set_data(rest_req->hpd_request, rest_req, on_free))) {
        HPD_LOG_ERROR(context, "Unable to set data (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        if ((rc2 = hpd_request_free(rest_req->hpd_request))) {
            HPD_LOG_ERROR(context, "Failed to free request (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }
    if ((rc = hpd_request(rest_req->hpd_request))) {
        HPD_LOG_ERROR(context, "Unable to send request (code: %d).", rc);
        if ((rc2 = reply_internal_server_error(req, rest_req, context))) {
            HPD_LOG_ERROR(context, "Failed to send internal server error response (code: %d).", rc2);
        }
        if ((rc2 = hpd_request_free(rest_req->hpd_request))) {
            HPD_LOG_ERROR(context, "Failed to free request (code: %d).", rc2);
        }
        return HPD_HTTPD_R_STOP;
    }

    if ((rc = hpd_httpd_request_keep_open(rest_req->http_req))) {
        HPD_LOG_WARN(context, "Failed to keep the connection open, hoping for the best... (code: %d).", rc);
    }
    return HPD_HTTPD_R_CONTINUE;
}

static hpd_error_t on_create(void **data, hpd_module_t *context)
{
    hpd_error_t rc;

    if ((rc = hpd_module_add_option(context, "port", "port", 0, "Listener port for rest server."))) return rc;

    hpd_httpd_settings_t ws_set = HPD_HTTPD_SETTINGS_DEFAULT;
    ws_set.on_req_begin = on_req_begin;
    ws_set.on_req_url_cmpl = on_req_url_cmpl;
    ws_set.on_req_hdr_cmpl = on_req_hdr_cmpl;
    ws_set.on_req_body = on_req_body;
    ws_set.on_req_cmpl = on_req_cmpl;
    ws_set.on_req_destroy = on_req_destroy;

    hpd_rest_t *rest;
    HPD_CALLOC(rest, 1, hpd_rest_t);
    rest->ws_set = ws_set;
    rest->ws_set.httpd_ctx = rest;
    rest->context = context;

    rest->curl = curl_easy_init();
    if (!rest->curl) {
        on_destroy(rest);
        HPD_LOG_RETURN(rest->context, HPD_E_UNKNOWN, "Could not initialise curl.");
    }

    (*data) = rest;
    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

static hpd_error_t on_destroy(void *data)
{
    hpd_rest_t *rest = data;
    if (rest) {
        if (rest->curl) curl_easy_cleanup(rest->curl);
        free(rest);
    }
    return HPD_E_SUCCESS;
}

static hpd_error_t on_start(void *data, hpd_t *hpd)
{
    hpd_error_t rc, rc2;
    hpd_rest_t *rest = data;
    hpd_module_t *context = rest->context;

    HPD_LOG_INFO(context, "Starting REST server on port %d.", rest->ws_set.port);

    rest->hpd = hpd;

    hpd_ev_loop_t *loop;
    if ((rc = hpd_get_loop(hpd, &loop))) return rc;

    if ((rc = hpd_httpd_create(&rest->ws, &rest->ws_set, context, loop))) return rc;
    if ((rc = hpd_httpd_start(rest->ws))) {
        if ((rc2 = hpd_httpd_destroy(rest->ws))) {
            HPD_LOG_ERROR(context, "Failed to destroy httpd (code: %d).", rc2);
        }
        return rc;
    }

    return HPD_E_SUCCESS;
}

static hpd_error_t on_stop(void *data, hpd_t *hpd)
{
    hpd_error_t rc, rc2;
    hpd_rest_t *rest = data;
    hpd_module_t *context = rest->context;

    HPD_LOG_INFO(rest->context, "Stopping REST server.");

    rc = hpd_httpd_stop(rest->ws);
    if ((rc2 = hpd_httpd_destroy(rest->ws))) {
        if (rc) HPD_LOG_ERROR(context, "Failed to destroy httpd (code: %d).", rc2);
        else rc = rc2;
    }
    
    rest->ws = NULL;
    rest->hpd = NULL;

    return rc;
}

static hpd_error_t on_parse_opt(void *data, const char *name, const char *arg)
{
    hpd_rest_t *rest = data;

    if (strcmp(name, "port") == 0) {
        hpd_tcpd_port_t port = (hpd_tcpd_port_t) atoi(arg);
        if (port <= HPD_P_SYSTEM_PORTS_START || port > HPD_P_DYNAMIC_PORTS_END) return HPD_E_ARGUMENT;
        rest->ws_set.port = port;
    } else {
        return HPD_E_ARGUMENT;
    }

    return HPD_E_SUCCESS;
}
