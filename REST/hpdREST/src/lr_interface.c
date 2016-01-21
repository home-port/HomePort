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
#include "datamanager.h"
#include "homeport.h"
#include "hpd_error.h"
#include "json.h"
#include "xml.h"
#include <stdlib.h>
#include "libREST.h"

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

static struct lri_req *req_new(struct lr_request *req)
{
    struct lri_req *lri_req = malloc(sizeof(struct lri_req));
    lri_req->req = req;
    lri_req->req_str = NULL;
    lri_req->in_hpd = 0;
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

static void
sendState(Service *service, void *in, ErrorCode code, const char *val, size_t len)
{
    struct lri_req *lri = in;

    if (lri == NULL) {
        fprintf(stderr, "[LRI] Unexpected state (lri is null)\n");
        return;
    }

    /* Connection is closed downstairs, so don't send anything */
    if (lri->req == NULL)
        return req_free(lri);

    char *buffer = NULL, *state = NULL;
    struct lm *headersIn = lr_request_get_headers(lri->req);
    struct lm *headers = NULL;

    if (val) {
        buffer = malloc((len+1) * sizeof(char));
        strncpy(buffer, val, len);
        buffer[len] = '\0';
    } else {
#define XX(num, str) case ERR_##num: buffer = #str; break;
        switch (code) {
            HTTP_STATUS_CODE_MAP(XX)
            default:
                fprintf(stderr, "[Homeport] Unknown error code\n");
                code = ERR_500;
                buffer = "500 Internal Server Error: Unknown error code.";
        }
#undef XX
    }

    if (code == ERR_200 && val) {
        /*TODO Check header for XML or jSON*/
        char *accept = lm_find( headersIn, "Accept" );
        if (accept != NULL && strcmp(accept, "application/json") == 0)
        {
            state = jsonGetState(buffer);
            headers =  lm_create();
            lm_insert(headers, "Content-Type", "application/json");
        } else {
            state = xmlGetState(buffer);
            headers =  lm_create();
            lm_insert(headers, "Content-Type", "application/xml");
        }
        lr_sendf(lri->req, WS_HTTP_200, headers, state);
        lm_destroy(headers);
    } else {
        fprintf(stderr, "%s\n", buffer);
        lr_sendf(lri->req, code, NULL, buffer);
    }

    free(state);
    free(buffer);
}

static int
getState(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
    Service *service = (Service*) srv_data;

    if(service->getFunction == NULL) {
        lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
        return 1;
    }

    struct lri_req *lri_req = req_new(req);
    (*req_data) = lri_req;

    // Keep open: As the adapter may keep a pointer to request, we better insure that it is not close due to a timeout
    lr_request_keep_open(req);
    lri_req->in_hpd = 1;
    homePortGet(service, sendState, lri_req);

    // Stop parsing request, we don't need the body anyways
    return 1;
}

static int
setState(void *srv_data, void **req_data_in, struct lr_request *req, const char *body, size_t len)
{
    Service *service = srv_data;
    struct lri_req *lri_req = *req_data_in;

    if(service->putFunction == NULL) {
        lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
        return 1;
    }

    // Receive data
    if (body) {
        if (!lri_req) {
            lri_req = req_new(req);
            (*req_data_in) = lri_req;
        }
        if (lri_req->req_str) len += strlen(lri_req->req_str);
        lri_req->req_str = realloc(lri_req->req_str, (len+1)*sizeof(char));
        if (!lri_req->req_str) {
            fprintf(stderr, "Failed to allocate memory\n");
            lr_sendf(req, WS_HTTP_500, NULL, "Internal server error");
            return 1;
        }
        strncat(lri_req->req_str, body, len);
        lri_req->req_str[len] = '\0';
        return 0;
    }

    struct lm *headersIn = lr_request_get_headers(req);
    char *contentType = lm_find(headersIn, "Content-Type");
    char *value;
    int freeValue = 1;
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
        freeValue = 0;
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
    homePortSet(service, value, strlen(value), sendState, req);

    if(freeValue) free(value);

    return 0;
}

int
lri_registerService(struct lr *lr, Service *service)
{
    char *uri = service->uri;
    int rc;
    Service *s = lr_lookup_service(lr, uri);
    if (s) {
        printf("A similar service is already registered in the unsecure server\n");
        return HPD_E_SERVICE_ALREADY_REGISTER;
    }

    // TODO Prefix printf statements and print to stderr on errors !! (Entire hpdREST module)
    printf("[rest] Registering service %s...\n", uri);
    rc = lr_register_service(lr,
                             uri,
                             getState, NULL, setState, NULL,
                             on_req_destroy, service);
    if(rc) {
        printf("Failed to register non secure service\n");
        return HPD_E_MHD_ERROR;
    }

    return HPD_E_SUCCESS;
}

int
lri_unregisterService(struct lr *lr, char* uri)
{
    Service *s = lr_lookup_service(lr, uri);
    if( s == NULL )
        return HPD_E_SERVICE_NOT_REGISTER;

    s = lr_unregister_service(lr, uri);
    if( s == NULL )
        return HPD_E_MHD_ERROR;

    return HPD_E_SUCCESS;
}

int
lri_getConfiguration(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
    HomePort *homeport = (HomePort*) srv_data;
    struct lm *headersIn =  lr_request_get_headers( req );
    char *accept;
    char *res;

    accept = lm_find( headersIn, "Accept" );

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
    lr_sendf(req, WS_HTTP_200, NULL, res);

    free(res);
    return 0;
}


