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

#include "homeport.h"
#include "json.h"
#include "xml.h"
#include "libREST.h"
#include <stdlib.h>
#include <string.h>

void
homePortSendState(Service *service, void *req_in, const char *val, size_t len)
{
   char *buffer, *state;
   struct lr_request *req = req_in;
   struct lm *headersIn = lr_request_get_headers(req);
   struct lm *headers;

   // Call callback and send response
   buffer = malloc((len+1) * sizeof(char));
   strncpy(buffer, val, len);
   if (len) {
     buffer[len] = '\0';
     /*TODO Check header for XML or jSON*/
     char *accept = lm_find( headersIn, "Accept" );
     if( strcmp(accept, "application/json") == 0 )
     {
       state = jsonGetState(buffer);
       headers =  lm_create();
       lm_insert(headers, "Content-Type", "application/json");
       lr_sendf(req, WS_HTTP_200, headers, state);
     }
     else
     { 
       state = xmlGetState(buffer);
       headers =  lm_create();
       lm_insert(headers, "Content-Type", "application/xml");
       lr_sendf(req, WS_HTTP_200, headers, state);
     }
     lm_destroy(headers);
     free(state);
     free(buffer);
   } else {
     lr_sendf(req, WS_HTTP_500, NULL, "Internal Server Error");
   }
}


