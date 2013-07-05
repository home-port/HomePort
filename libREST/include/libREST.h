// libREST.h

/*  Copyright 2013 Aalborg University. All rights reserved.
 *   
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  
 *  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *  
 *  The views and conclusions contained in the software and
 *  documentation are those of the authors and should not be interpreted
 *  as representing official policies, either expressed.
 */

#ifndef LIBREST_H
#define LIBREST_H

#include "http_types.h"
#include <stddef.h>

// Structs
struct lr;
struct lr_request;
struct ev_loop;

struct lr_settings {
	int port;
	int timeout;
};
#define LR_SETTINGS_DEFAULT { \
	.port = WS_PORT_HTTP, \
	.timeout = 15 }

// Callbacks
typedef int (*lr_cb)(void *req, const char* body, size_t len);

// libREST instance functions
struct lr *lr_create(struct lr_settings *settings, struct ev_loop *loop);
void lr_destroy(struct lr *ins);

int lr_start(struct lr *ins);
void lr_stop(struct lr *ins);

// Registers a new service. Returns 1 if the url is already registered or not enough memory
int lr_register_service(struct lr *ins,
                         char *url,
                         lr_cb on_get,
                         lr_cb on_post,
                         lr_cb on_put,
                         lr_cb on_delete);

void lr_unregister_service(struct lr *ins, char *url);

// Send response functions
void lr_sendf(void *req, enum httpws_http_status_code status,
              char *fmt, ...);

// send(req,
// start_send(req, 
// send_chunk(req, 		#error if called without start_send
// stop_send(req, 

#endif
