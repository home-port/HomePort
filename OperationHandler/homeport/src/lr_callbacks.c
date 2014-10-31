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

#include "lr_callbacks.h"
#include "homeport.h"
#include "json.h"
#include "xml.h"
#include "hpd_service.h"
#include "libREST.h"
#include <stdlib.h>

#define MHD_MAX_BUFFER_SIZE 10

int
lrcb_getConfiguration(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
  HomePort *homeport = (HomePort*) srv_data;
  struct lm *headersIn =  lr_request_get_headers( req );
  char *accept;
  char *res;

  accept = lm_find( headersIn, "Accept" );

  /** Defaults to XML */
  if( strcmp(accept, "application/json") == 0 )
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

int
lrcb_getState(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
  Service *service = (Service*) srv_data;

  if(service->getFunction == NULL)
  {
    lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
    return 1;
  }

  // Keep open: As the adapter may keep a pointer to request, we better insure that it is not close due to a timeout
  lr_request_keep_open(req);
  service->getFunction(service, req);

  // Stop parsing request, we don't need the body anyways
  return 1;
}

int 
lrcb_setState(void *srv_data, void **req_data, struct lr_request *req, const char *body, size_t len)
{
  Service *service = srv_data;
  char *req_str = *req_data;
  char *str=NULL;

  if(service->putFunction == NULL)
  {
    lr_sendf(req, WS_HTTP_405, NULL, "405 Method Not Allowed");
    return 1;
  }

  // Recieve data
  if (body) {
    if (*req_data) len += strlen(req_str);
    str = realloc(*req_data, (len+1)*sizeof(char));
    if (!str) {
      printf("Failed to allocate memory\n");
      lr_sendf(req, WS_HTTP_500, NULL, "Internal server error");
      return 1;
    }
    strncpy(str, body, len);
    str[len] = '\0';
    *req_data = str;
    return 0;
  }
  struct lm *headersIn = lr_request_get_headers(req);
  char *contentType = lm_find(headersIn, "Content-Type");
  char *value;
  int freeValue = 1;
  if(*req_data == NULL)
  {
    lr_sendf(req, WS_HTTP_400, NULL, "400 Bad Request");
    return 1;
  }
  if( ( contentType == NULL ) || ( strcmp(contentType, "application/xml") == 0 ) )
  {
    value = xmlParseState(*req_data);
  }
  else if( strcmp( contentType, "application/json" ) == 0 )
  {
    value = jsonParseState(*req_data);
    freeValue = 0;
  }
  else
  {
    free(*req_data);
    lr_sendf(req, WS_HTTP_415, NULL, "415 Unsupported Media Type");
    return 0;
  }
  free(*req_data);
  if (!value) {
    lr_sendf(req, WS_HTTP_400, NULL, "400 Bad Request");
    return 1;
  }

  // Call callback
  char *buffer = malloc((MHD_MAX_BUFFER_SIZE+1) * sizeof(char));
  int buf_len = service->putFunction(service,
      buffer, MHD_MAX_BUFFER_SIZE,
      value);

  if(freeValue) free(value);

  // Send response
  if (buf_len == 0) {
    lr_sendf(req, WS_HTTP_500, NULL, "500 Internal Server Error");
    free(buffer);
    return 1;
  } else {
    //    // Send value change event
    //    const char *IP = lr_request_get_ip(req);
    //    buffer[buf_len] = '\0';
    //    send_event_of_value_change(service, buffer, IP);

    // Reply to request
    char *ret;
    if( strcmp( contentType, "application/xml" ) == 0 )
    {
      ret = xmlGetState(buffer);
    }
    else
    {
      ret = jsonGetState(buffer);
    }

    lr_sendf(req, WS_HTTP_200, NULL, ret);
    free(ret);
  }
  free(buffer);
  return 0;
}


