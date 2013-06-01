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

#include "ws_types.h"

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

// Enums
enum lr_method
{
	GET,
	POST,
	PUT,
	DELETE
};

// Callbacks
typedef int (*lr_cb)(void *data, const char* url, size_t url_len);

// libREST instance functions
struct lr *lr_create(struct lr_settings *settings, struct ev_loop *loop);
void lr_destroy(struct lr *ins);

int lr_start(struct lr *ins);
void lr_stop(struct lr *ins);

void lr_register_service(struct lr *ins,
                         char *url,
                         lr_cb on_get,
                         lr_cb on_post,
                         lr_cb on_put,
                         lr_cb on_delete);

void lr_unregister_service(struct lr *ins, char *url);

// Features libREST should have:
// Register a callback
// Start, stop (webserver. Should also have port etc.)
// Call callback (maybe with chunks). It has to connect it with a request (to know which chunks belong together). That will be the void *
// Start send chunk, send chunk, stop send chunk etc.

// call_cb(req, 
// send(req,
// start_send(req, 
// send_chunk(req, 		#error if called without start_send
// stop_send(req, 
// lr create    		#should also create a webserver etc.
// destroy(lr, 
// start(lr, 
// stop(lr,
// register(lr, url, method, 
// unregister(lr, 

// libREST will register on on_url etc, and handle the answers if the url is not registered etc. 

#endif