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

#include <stdlib.h>
#include "hpd_rest.h"
#include "hpd_application_api.h"
#include "hpd_common.h"
#include "http-webserver.h"
#include <curl/curl.h>
#include "json.h"
#include "xml.h"

static hpd_error_t on_create(void **data, hpd_module_t *context);
static hpd_error_t on_destroy(void *data);
static hpd_error_t on_start(void *data, hpd_t *hpd);
static hpd_error_t on_stop(void *data, hpd_t *hpd);
static hpd_error_t on_parse_opt(void *data, const char *name, const char *arg);

hpd_module_def_t hpd_rest = { on_create, on_destroy, on_start, on_stop, on_parse_opt };

typedef struct hpd_rest {
    struct httpws *ws;
    struct httpws_settings ws_set;
    hpd_t *hpd;
} hpd_rest_t;

typedef struct hpd_rest_req {
    hpd_service_id_t *service;
    struct http_request *http_req;
    char *body;
    size_t len;
    hpd_request_t *hpd_request;
} hpd_rest_req_t;

static char *url_encode(const char *decoded)
{
    char *encoded;
    CURL *curl = curl_easy_init();

    if(curl)
        encoded = curl_easy_escape(curl, decoded, 0);
    else
        encoded = NULL;

    curl_easy_cleanup(curl);
    return encoded;
}

static char *url_decode(const char *encoded)
{
    char *decoded;
    CURL *curl = curl_easy_init();
    if(curl)
        decoded = curl_easy_unescape(curl, encoded, 0, NULL);
    else
        decoded = NULL;

    curl_easy_cleanup(curl);
    return decoded;
}

char *hpd_rest_url_create(hpd_service_id_t *service)
{
    // TODO Better error handling - cleanup too !!
    hpd_device_id_t *device;
    hpd_adapter_id_t *adapter;
    if (hpd_service_get_device(service, &device)) return NULL;
    if (hpd_service_get_adapter(service, &adapter)) return NULL;

    // TODO Better error handling - cleanup too !!
    const char *srv_id, *dev_id, *adp_id;
    if (hpd_service_get_id(service, &srv_id)) return NULL;
    if (hpd_device_get_id(device, &dev_id)) return NULL;
    if (hpd_adapter_get_id(adapter, &adp_id)) return NULL;

    // TODO Error handling and cleanup
    hpd_device_id_free(device);
    hpd_adapter_id_free(adapter);

    char *aid = url_encode(adp_id);
    char *did = url_encode(dev_id);
    char *sid = url_encode(srv_id);

    if (!(aid && did && sid))
        return NULL;

    char *uri = malloc((strlen(aid)+strlen(did)+strlen(sid)+3+1)*sizeof(char));
    if( uri == NULL ) return NULL;
    uri[0] = '\0';

    strcat(uri, "/");
    strcat(uri, aid);
    strcat(uri, "/");
    strcat(uri, did);
    strcat(uri, "/");
    strcat(uri, sid);

    free(aid);
    free(did);
    free(sid);

    return uri;
}

// TODO Should probably double-check this one...
static hpd_error_t url_extract(const char *url, char **aid, char **did, char **sid)
{
    const char *aid_p, *did_p, *sid_p;
    size_t aid_l, did_l, sid_l;
    
    if (strlen(url) < 2)
        return HPD_E_ARGUMENT;
    if (url[0] != '/')
        return HPD_E_ARGUMENT;

    aid_p = &url[1];
    if (aid_p[0] == '\0' || aid_p[0] == '/')
        return HPD_E_ARGUMENT;
    for (aid_l = 0; aid_p[aid_l] != '/'; aid_l ++)
        if (aid_p[aid_l] == '\0')
            return HPD_E_ARGUMENT;

    did_p = &aid_p[aid_l+1];
    if (did_p[0] == '\0' || did_p[0] == '/')
        return HPD_E_ARGUMENT;
    for (did_l = 0; did_p[did_l] != '/'; did_l ++)
        if (did_p[did_l] == '\0')
            return HPD_E_ARGUMENT;

    sid_p = &did_p[did_l+1];
    if (sid_p[0] == '\0' || sid_p[0] == '/')
        return HPD_E_ARGUMENT;
    for (sid_l = 0; sid_p[sid_l] != '/' && sid_p[sid_l] != '\0'; sid_l++);

    if (!sid_p[sid_l] == '\0')
        return HPD_E_ARGUMENT;

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
    return  HPD_E_ALLOC;
}

static void reply_malformed_url(struct http_request *req)
{
    struct http_response *res = http_response_create(req, WS_HTTP_400);
    http_response_sendf(res, "Malformed URL");
    http_response_destroy(res);
}

static void reply_internal_server_error(struct http_request *req)
{
    struct http_response *res = http_response_create(req, WS_HTTP_500);
    http_response_sendf(res, "Internal Server Error");
    http_response_destroy(res);
}

static void reply_not_found(struct http_request *req)
{
    struct http_response *res = http_response_create(req, WS_HTTP_404);
    http_response_sendf(res, "Not Found");
    http_response_destroy(res);
}

static void reply_unsupported_media_type(struct http_request *req)
{
    struct http_response *res = http_response_create(req, WS_HTTP_415);
    http_response_sendf(res, "Unsupported Media Type");
    http_response_destroy(res);
}

static void reply_bad_request(struct http_request *req)
{
    struct http_response *res = http_response_create(req, WS_HTTP_400);
    http_response_sendf(res, "Bad Request");
    http_response_destroy(res);
}

static void reply_method_not_allowed(struct http_request *req)
{
    struct http_response *res = http_response_create(req, WS_HTTP_405);
    http_response_sendf(res, "Method Not Allowed");
    http_response_destroy(res);
}

static hpd_error_t on_free(void *data)
{
    hpd_rest_req_t *rest_req = data;

    if (rest_req->http_req) {
        // TODO Have to check whether there has been responded or not...
        //reply_internal_server_error(rest_req->http_req); // TODO Best response?
        rest_req->hpd_request = NULL;
    } else {
        hpd_service_id_free(rest_req->service); // TODO Error check
        free(rest_req->body);
        free(rest_req);
    }

    return HPD_E_SUCCESS;
}

static int on_req_destroy(struct httpws *ins, struct http_request *req, void* ws_ctx, void** req_data)
{
    hpd_rest_req_t *rest_req = *req_data;

    if (!rest_req) return 0;

    if (rest_req->hpd_request) {
        rest_req->http_req = NULL;
    } else {
        hpd_service_id_free(rest_req->service); // TODO Error check
        free(rest_req->body);
        free(rest_req);
    }

    return 0;
}

// TODO Double check this...
static hpd_error_t on_response(hpd_response_t *res)
{
    hpd_error_t rc;
    hpd_rest_req_t *rest_req;
    hpd_response_get_request_data(res, (void **) &rest_req); // TODO Error check

    if (rest_req->http_req) {
        hpd_value_t *value;
        if ((rc = hpd_response_get_value(res, &value))) return rc;
        const char *val;
        size_t len;
        if ((rc = hpd_value_get_body(value, &val, &len))) return rc;
        hpd_status_t status;
        if ((rc = hpd_response_get_status(res, &status))) return rc;

        map_t *headersIn = http_request_get_headers(rest_req->http_req);

        char *buffer = NULL;
        if (val) {
            buffer = malloc((len+1) * sizeof(char));
            strncpy(buffer, val, len);
            buffer[len] = '\0';
        } else {
#define XX(num, str) case HPD_S_##num: HPD_STR_CPY(buffer, #str); break;
            switch (status) {
                HPD_HTTP_STATUS_CODE_MAP(XX)
                default:
                    fprintf(stderr, "[Homeport] Unknown error code\n");
                    status = HPD_S_500;
                    HPD_STR_CPY(buffer, "500 Internal Server Error: Unknown error code.");
            }
#undef XX
        }

        if (status == HPD_S_200 && val) {
            /*TODO Check header for XML or jSON*/
            char *accept;
            MAP_GET(headersIn, "Accept", accept);
            char *state;
            struct http_response *response = http_response_create(rest_req->http_req, status);
            if (accept != NULL && strcmp(accept, "application/json") == 0)
            {
                state = jsonGetState(buffer);
                http_response_add_header(response, "Content-Type", "application/json");
            } else {
                state = xmlGetState(buffer);
                http_response_add_header(response, "Content-Type", "application/xml");
            }
            // TODO Consider headers to add
#ifdef LR_ORIGIN
            http_response_add_header(response, "Access-Control-Allow-Origin", "*");
#endif
            http_response_sendf(response, "%s", state);
            http_response_destroy(response);
            free(state);
        } else {
            fprintf(stderr, "%s\n", buffer);
            struct http_response *response = http_response_create(rest_req->http_req, status);
            // TODO Consider headers to add
#ifdef LR_ORIGIN
            http_response_add_header(response, "Access-Control-Allow-Origin", "*");
#endif
            http_response_sendf(response, "%s", buffer);
            http_response_destroy(response);
        }

        free(buffer);
    }

    return HPD_E_SUCCESS;

    alloc_error:
        // TODO Error handling
        return HPD_E_ALLOC;
}

// TODO Clean up on errors !
static int on_req_url_cmpl(struct httpws *ins, struct http_request *req,
                           void* ws_ctx, void** req_data)
{
    struct hpd_rest *rest = ws_ctx;
    const char *url = http_request_get_url(req);

    if (!rest->hpd) {
        // TODO Best error type ?
        reply_internal_server_error(req);
        return 1;
    }

    if (strcmp(url, "/devices") == 0) {
        map_t *headersIn = http_request_get_headers(req);
        char *accept;
        char *body;

        MAP_GET(headersIn, "Accept", accept);

        /** Defaults to XML */
        // TODO: There's a double check on application/json somewhere else, extract method!
        if( accept != NULL && strcmp(accept, "application/json") == 0 ) {
            body = jsonGetConfiguration(rest->hpd);
        } else {
            body = xmlGetConfiguration(rest->hpd);
        }

        struct http_response *res = http_response_create(req, WS_HTTP_200);
        http_response_sendf(res, "%s", body);
        http_response_destroy(res);

        free(body);

        return 1;
    }
    
    char *aid_encoded, *did_encoded, *sid_encoded;
    if (url == NULL) {
        reply_malformed_url(req);
        return 1;
    }
    switch (url_extract(url, &aid_encoded, &did_encoded, &sid_encoded)) {
        case HPD_E_ALLOC:
            reply_internal_server_error(req);
            return 1;
        case HPD_E_ARGUMENT:
            reply_malformed_url(req);
            return 1;
        case HPD_E_SUCCESS:
            break;
        default:
            reply_internal_server_error(req);
            return 1;
    }

    char *aid_decoded, *did_decoded, *sid_decoded;
    aid_decoded = url_decode(aid_encoded);
    did_decoded = url_decode(did_encoded);
    sid_decoded = url_decode(sid_encoded);
    free(aid_encoded);
    free(did_encoded);
    free(sid_encoded);

    hpd_service_id_t *service;
    hpd_service_id_alloc(&service, rest->hpd, aid_decoded, did_decoded, sid_decoded); // TODO Error handling
    free(aid_decoded);
    free(did_decoded);
    free(sid_decoded);

    hpd_method_t method;
    switch(http_request_get_method(req))
    {
#ifdef LR_ORIGIN
        case HTTP_OPTIONS:
            method = HPD_M_NONE;
            break;
#endif
        case HTTP_GET:
            method = HPD_M_GET;
            break;
        case HTTP_DELETE:
            reply_method_not_allowed(req);
            return 1;
        case HTTP_POST:
            reply_method_not_allowed(req);
            return 1;
        case HTTP_PUT:
            method = HPD_M_PUT;
            break;
        default:
            reply_method_not_allowed(req);
            return 1;
    }
    if (method > HPD_M_NONE) {
        hpd_bool_t bool;
        switch (hpd_service_has_action(service, method, &bool)) {
            case HPD_E_NOT_FOUND:
                reply_not_found(req);
                break;
        };
        if (!bool) {
            reply_method_not_allowed(req);
            return 1;
        }
    }

    hpd_rest_req_t *rest_req;
    HPD_CALLOC(rest_req, 1, hpd_rest_req_t);
    rest_req->service = service;
    rest_req->http_req = req;
    *req_data = rest_req;

    return 0;

    alloc_error:
        reply_internal_server_error(req);
        return 1;
}

static int on_req_hdr_cmpl(
        struct httpws *ins, struct http_request *req,
        void *ws_ctx, void **req_data)
{
#ifdef LR_ORIGIN
    hpd_error_t rc;
    hpd_rest_req_t *rest_req = *req_data;
    hpd_service_id_t *service = rest_req->service;

    struct http_response *res;
    char methods[23];
    methods[0] = '\0';

    switch(http_request_get_method(req))
    {
        case HTTP_OPTIONS:
            res = http_response_create(req, WS_HTTP_200);
            http_response_add_header(res, "Access-Control-Allow-Origin", "*");
            hpd_action_t *action;
            hpd_service_foreach_action(rc, action, service) {
                hpd_method_t method;
                hpd_action_get_method(action, &method); // TODO Error handling
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
                        // TODO Shouldn't be here...
                        break;
                }
            }
            switch (rc) {
                case HPD_E_NOT_FOUND:
                    // TODO Do I need to clean up rest_req or will destroy always be called from downstairs?
                    reply_not_found(req);
                    return 1;
            }
            http_response_add_header(res, "Access-Control-Allow-Methods", methods);
            // TODO Having so many specified headers here is not really thaaaat good
            http_response_add_header(res, "Access-Control-Allow-Headers", "Content-Type, Cache-Control, Accept, X-Requested-With");
            http_response_sendf(res, "OK");
            http_response_destroy(res);
            return 1;
        default:
            return 0;
    }
#else
    return 0;
#endif
}

static int on_req_body(struct httpws *ins, struct http_request *req,
                   void* ws_ctx, void** req_data,
                   const char* chunk, size_t len)
{
    hpd_rest_req_t *rest_req = *req_data;

    HPD_REALLOC(rest_req->body, rest_req->len + len, char);
    strncpy(&rest_req->body[rest_req->len], chunk, len);
    rest_req->len += len;
    return 0;

    alloc_error:
        // TODO Error handling
        return 1;
}

static int on_req_cmpl(struct httpws *ins, struct http_request *req, void* ws_ctx, void** req_data)
{
    hpd_rest_req_t *rest_req = *req_data;
    hpd_service_id_t *service = rest_req->service;

    hpd_method_t method;
    switch(http_request_get_method(req)) {
        case HTTP_GET:
            method = HPD_M_GET;
            break;
        case HTTP_PUT:
            method = HPD_M_PUT;
            break;
        default:
            return 1;
    }

    hpd_value_t *value = NULL;
    if (rest_req->body) {
        map_t *headersIn = http_request_get_headers(req);
        char *contentType;
        MAP_GET(headersIn, "Content-Type", contentType);
        char *v;

        if (contentType == NULL ||
            strcmp(contentType, "application/xml") == 0 ||
            strncmp(contentType, "application/xml;", 16) == 0) {
            v = xmlParseState(rest_req->body);
        } else if (strcmp( contentType, "application/json" ) == 0 ||
                   strncmp(contentType, "application/json;", 17) == 0) {
            v = (char *) jsonParseState(rest_req->body);
        } else {
            reply_unsupported_media_type(req);
            return 1;
        }

        if (!v) {
            reply_bad_request(req);
            return 1;
        }

        hpd_value_alloc(&value, v, HPD_NULL_TERMINATED); // TODO Check for errors!
    }

    hpd_request_alloc(&rest_req->hpd_request, service, method, on_response); // TODO Check for errors!
    if (value) hpd_request_set_value(rest_req->hpd_request, value); // TODO Check for errors!
    // TODO Set free function, but be aware of two-way free !
    hpd_request_set_data(rest_req->hpd_request, rest_req, on_free); // TODO Check for errors!
    hpd_request(rest_req->hpd_request); // TODO Check for errors!

    http_request_keep_open(rest_req->http_req);
    return 0;
}

static hpd_error_t on_create(void **data, hpd_module_t *context)
{
    hpd_error_t rc;
    if ((rc = hpd_module_add_option(context, "port", "port", 0, "Listener port for rest server."))) return rc;

    struct httpws_settings ws_set = HTTPWS_SETTINGS_DEFAULT;
    ws_set.on_req_url_cmpl = on_req_url_cmpl;
    ws_set.on_req_hdr_cmpl = on_req_hdr_cmpl;
    ws_set.on_req_body = on_req_body;
    ws_set.on_req_cmpl = on_req_cmpl;
    ws_set.on_req_destroy = on_req_destroy;

    hpd_rest_t *rest;
    HPD_CALLOC(rest, 1, hpd_rest_t);
    rest->ws_set = ws_set;

    rest->ws_set.ws_ctx = rest;
    (*data) = rest;

    return HPD_E_SUCCESS;

    alloc_error:
    return HPD_E_ALLOC;
}

static hpd_error_t on_destroy(void *data)
{
    free(data);
    return HPD_E_SUCCESS;
}

// TODO Not the best error handling (probably goes for the entire file)...
// TODO Definitely needs cleanup on errors !!!
static hpd_error_t on_start(void *data, hpd_t *hpd)
{
    hpd_error_t rc;
    hpd_rest_t *rest = data;

    hpd_ev_loop_t *loop;
    if ((rc = hpd_get_loop(hpd, &loop))) return rc;

    if (!(rest->ws = httpws_create(&rest->ws_set, loop))) return HPD_E_UNKNOWN;

    if (httpws_start(rest->ws)) return HPD_E_UNKNOWN;

    rest->hpd = hpd;

    return HPD_E_SUCCESS;
}

static hpd_error_t on_stop(void *data, hpd_t *hpd)
{
    hpd_rest_t *rest = data;

    httpws_stop(rest->ws);
    httpws_destroy(rest->ws);
    rest->ws = NULL;

    rest->hpd = NULL;

    return HPD_E_SUCCESS;
}

static hpd_error_t on_parse_opt(void *data, const char *name, const char *arg)
{
    hpd_rest_t *rest = data;

    if (strcmp(name, "port") == 0) {
        // TODO Lazy error handling:
        rest->ws_set.port = (enum ws_port) atoi(arg);
    } else {
        return HPD_E_ARGUMENT;
    }

    return HPD_E_SUCCESS;
}
