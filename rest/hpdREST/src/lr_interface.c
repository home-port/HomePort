/*Copyright 2011 Aalborg University. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of
  conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list
  of conditions and the following disclaimer in the documentation and/or other materials
  provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed*/

#include "lr_interface.h"
#include "json.h"
#include "xml.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "libREST.h"
#include "map.h"
#include "hpd_application_api.h"

#define string_copy(copy, original) 			\
  do { 							\
    copy = malloc(sizeof(char)*(strlen(original)+1)); 	\
    if(copy == NULL) 					\
    { 							\
      goto cleanup; 					\
    } 							\
    strcpy(copy, original); 				\
  }while(0)

struct lri_req {
    struct lr_request *req;
    char in_hpd;    ///< Bool to check whether a pointer to this struct is present in hpd somewhere
    char *req_str;  ///< String used to store bodies for PUT
};

static void req_free(struct lri_req *req)
{
    if (req != NULL) {
        if (req->req_str) {
            free(req->req_str);
        }
        free(req);
    }
}

static hpd_error_t on_req_free(void *req)
{
    req_free(req);
    return HPD_E_SUCCESS;
}

static struct lri_req *req_new(struct lr_request *req)
{
    struct lri_req *lri_req = malloc(sizeof(struct lri_req));
    lri_req->req = req;
    lri_req->req_str = NULL;
    lri_req->in_hpd = 0;
    return lri_req;
}

static int on_req_destroy(void *srv_data, void **req_data, struct lr_request *req)
{
    struct lri_req *lri_req = (struct lri_req *)(*req_data);
    if (lri_req != NULL) {
        lri_req->req = NULL;
        if (!lri_req->in_hpd) req_free(lri_req);
    }
    return 0;
}

static hpd_error_t sendState(hpd_response_t *res)
{
    hpd_error_t rc;
    struct lri_req *lri;
    if ((rc = hpd_response_get_request_data(res, (void **) &lri))) return rc;

    if (lri == NULL) {
        fprintf(stderr, "[LRI] Unexpected state (lri is null)\n");
        return HPD_E_UNKNOWN;
    }

    /* Connection is closed downstairs, so don't send anything */
    if (lri->req == NULL) {
        req_free(lri);
        return HPD_E_SUCCESS;
    }

    hpd_value_t *value;
    if ((rc = hpd_response_get_value(res, &value))) return rc;
    const char *val;
    size_t len;
    if ((rc = hpd_value_get_body(value, &val, &len))) return rc;
    hpd_status_t code;
    if ((rc = hpd_response_get_status(res, &code))) return rc;

    char *buffer = NULL, *state = NULL;
    map_t *headersIn = lr_request_get_headers(lri->req);

    if (val) {
        buffer = malloc((len+1) * sizeof(char));
        strncpy(buffer, val, len);
        buffer[len] = '\0';
    } else {
#define XX(num, str) case HPD_S_##num: string_copy(buffer, #str); break;
        switch (code) {
            HPD_HTTP_STATUS_CODE_MAP(XX)
            default:
                fprintf(stderr, "[Homeport] Unknown error code\n");
                code = HPD_S_500;
                string_copy(buffer, "500 Internal Server Error: Unknown error code.");
        }
#undef XX
    }

    if (code == HPD_S_200 && val) {
        /*TODO Check header for XML or jSON*/
        char *accept;
        MAP_GET(headersIn, "Accept", accept);
        map_t headers;
        MAP_INIT(&headers);
        if (accept != NULL && strcmp(accept, "application/json") == 0)
        {
            state = jsonGetState(buffer);
            MAP_SET(&headers, "Content-Type", "application/json");
        } else {
            state = xmlGetState(buffer);
            MAP_SET(&headers, "Content-Type", "application/xml");
        }
        lr_sendf(lri->req, WS_HTTP_200, &headers, state);
        MAP_FREE(&headers);
    } else {
        fprintf(stderr, "%s\n", buffer);
        lr_sendf(lri->req, code, NULL, buffer);
    }

    free(state);
    free(buffer);

    cleanup: // TODO Fixme
        return HPD_E_SUCCESS;

    alloc_error: // TODO Fixme: Ensure we are in a good state memory wise
        return HPD_E_ALLOC;
}

static int
getState(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
    hpd_service_id_t *service = (hpd_service_id_t*) srv_data;

    // TODO Error handling?
    hpd_bool_t bool;
    if (hpd_service_has_action(service, HPD_M_GET, &bool)) return 1;
    if (!bool) {
        lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
        return 1;
    }

    struct lri_req *lri_req = req_new(req);
    (*req_data) = lri_req;

    // Keep open: As the adapter may keep a pointer to request, we better insure that it is not close due to a timeout
    lr_request_keep_open(req);
    lri_req->in_hpd = 1;

    // TODO Error handling
    hpd_request_t *request;
    hpd_request_alloc(&request, service, HPD_M_GET, sendState);
    hpd_request_set_data(request, lri_req, on_req_free);
    hpd_request(request);

    // Stop parsing request, we don't need the body anyways
    return 1;
}

static int
setState(void *srv_data, void **req_data_in, struct lr_request *req, const char *body, size_t len)
{
    hpd_service_id_t *service = srv_data;
    struct lri_req *lri_req = *req_data_in;

    // TODO Error handling?
    hpd_bool_t bool;
    if (hpd_service_has_action(service, HPD_M_PUT, &bool)) return 1;
    if (!bool) {
        lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
        return 1;
    }

    // Receive data
    if (body) {
        if (!lri_req) {
            lri_req = req_new(req);
            (*req_data_in) = lri_req;
        }

        if (!lri_req->req_str) {
            lri_req->req_str = malloc((len+1)*sizeof(char));
            lri_req->req_str[0] = '\0';
        } else {
            len += strlen(lri_req->req_str);
            lri_req->req_str = realloc(lri_req->req_str, (len+1)*sizeof(char));
        }

        if (!lri_req->req_str) {
            fprintf(stderr, "Failed to allocate memory\n");
            lr_sendf(req, WS_HTTP_500, NULL, "Internal server error");
            return 1;
        }
        strncat(lri_req->req_str, body, len);
        lri_req->req_str[len] = '\0';
        return 0;
    }

    map_t *headersIn = lr_request_get_headers(req);
    char *contentType;
    MAP_GET(headersIn, "Content-Type", contentType);
    char *value;
    if(lri_req == NULL || lri_req->req_str == NULL)
    {
        lr_sendf(req, WS_HTTP_400, NULL, "400 Bad Request");
        return 1;
    }
    if( contentType == NULL ||
        strcmp(contentType, "application/xml") == 0 ||
        strncmp(contentType, "application/xml;", 16) == 0 )
    {
        value = xmlParseState(lri_req->req_str);
    }
    else if( strcmp( contentType, "application/json" ) == 0 ||
             strncmp(contentType, "application/json;", 17) == 0 )
    {
        value = (char *) jsonParseState(lri_req->req_str);
    }
    else
    {
        lr_sendf(req, WS_HTTP_415, NULL, "415 Unsupported Media Type");
        return 0;
    }

    if (!value) {
        lr_sendf(req, WS_HTTP_400, NULL, "400 Bad Request");
        return 1;
    }

    // Call callback
    // Keep open: As the adapter may keep a pointer to request, we better insure that it is not close due to a timeout
    lr_request_keep_open(req);
    lri_req->in_hpd = 1;

    // TODO Error handling
    hpd_request_t *request;
    hpd_request_alloc(&request, service, HPD_M_PUT, sendState);
    hpd_request_set_data(request, lri_req, on_req_free);
    hpd_value_t *val;
    hpd_value_alloc(&val, value, HPD_NULL_TERMINATED);
    hpd_request_set_value(request, val);
    hpd_request(request);

    free(value);

    return 0;
}

char *lri_url_encode(const char *id)
{
    char *res;
    CURL *curl = curl_easy_init();

    if(curl)
        res = curl_easy_escape(curl, id, 0);
    else
        res = NULL;

    curl_easy_cleanup(curl);
    return res;
}

char *lri_url_decode(const char *id)
{
    char *res;
    CURL *curl = curl_easy_init();
    if(curl)
        res = curl_easy_unescape(curl, id, 0, NULL);
    else
        res = NULL;

    curl_easy_cleanup(curl);
    return res;
}

char *lri_alloc_uri(hpd_service_id_t *service)
{
    // TODO Better error handling
    hpd_device_id_t *device;
    hpd_adapter_id_t *adapter;
    if (hpd_service_get_device(service, &device)) return NULL;
    if (hpd_service_get_adapter(service, &adapter)) return NULL;

    // TODO Better error handling
    const char *srv_id, *dev_id, *adp_id;
    if (hpd_service_get_id(service, &srv_id)) return NULL;
    if (hpd_device_get_id(device, &dev_id)) return NULL;
    if (hpd_adapter_get_id(adapter, &adp_id)) return NULL;

    char *aid = lri_url_encode(adp_id);
    char *did = lri_url_encode(dev_id);
    char *sid = lri_url_encode(srv_id);

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

int
lri_registerService(struct lr *lr, hpd_service_id_t *service)
{
    char *uri = lri_alloc_uri(service);
    int rc;
    hpd_service_id_t *s = lr_lookup_service(lr, uri);
    if (s) {
        printf("A similar service is already registered in the unsecure server\n");
        free(uri);
        return HPD_E_UNKNOWN;
    }

    // TODO Prefix printf statements and print to stderr on errors !! (Entire hpdREST module)
    printf("[rest] Registering service %s...\n", uri);
    rc = lr_register_service(lr,
                             uri,
                             getState, NULL, setState, NULL,
                             on_req_destroy, service);
    free(uri);
    if(rc) {
        printf("Failed to register non secure service\n");
        return HPD_E_UNKNOWN;
    }

    return HPD_E_SUCCESS;
}

int
lri_unregisterService(struct lr *lr, char* uri)
{
    hpd_service_id_t *s = lr_lookup_service(lr, uri);
    if( s == NULL )
        return HPD_E_UNKNOWN;

    s = lr_unregister_service(lr, uri);
    if( s == NULL )
        return HPD_E_UNKNOWN;

    return HPD_E_SUCCESS;
}

int
lri_getConfiguration(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
    hpd_t *homeport = srv_data;
    map_t *headersIn =  lr_request_get_headers( req );
    char *accept;
    char *res;

    MAP_GET(headersIn, "Accept", accept);

    /** Defaults to XML */
    if( accept != NULL && strcmp(accept, "application/json") == 0 )
    {
        res = jsonGetConfiguration(homeport);
    }
    else
    {
        res = xmlGetConfiguration(homeport);
    }
//  else
//  {
//    lr_sendf(req, WS_HTTP_406, NULL, NULL);
//    return 0;
//  }
    lr_sendf(req, WS_HTTP_200, NULL, "%s", res);

    free(res);
    return 0;
}


