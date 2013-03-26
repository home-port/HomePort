// webserver.h

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

/** \defgroup webserver Webserver
 *  The webserver is a single threaded event based webserver, built on
 *  top of the event loop implemented in LibEV.
 *  
 *  The overall webserver consists of two sub-modules:
 *
 *  \dot
 *  graph example {
 *     node [shape=record, fontname=Helvetica, fontsize=10];
 *     webserver [ label="Webserver" URL="\ref webserver.h"];
 *     client [ label="Client" URL="\ref client.h"];
 *     webserver -- accept;
 *     webserver -- client;
 *  }
 *  \enddot
 *
 *  The \ref webserver.h "Webserver submodule" is the interface for
 *  anyone that needs to development using this webserver
 *  implementation.  Most users only needs to know the implementation
 *  within this submodule. The webser submodule also handles the
 *  acceptance of new clients.
 *
 *  The \ref client.h "Client submodule" handles the communication with
 *  a single client connected to the webserver. It takes over as the
 *  accepter of any connection made to the webserver submodule. 
 *
 *  \{
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ev.h>
#include "msg.h"

enum ws_log_level {
   WS_LOG_FATAL,
   WS_LOG_ERROR,
   WS_LOG_WARN,
   WS_LOG_INFO,
   WS_LOG_DEBUG
};

/// Instance of a webserver
/**
 *  This stuct represents a instance of the webserver. The struct
 *  constains both settings for the webserver, which may be changed by
 *  caller, and some internal data values, that are used to run the
 *  webserver.
 *
 *  Making any changes to this struct after a call to ws_start() will
 *  have either no effect or have undefined side-effects (most like
 *  negative).
 *
 *  Use ws_init() to initialise this struct, ws_start() to
 *  start a webserver, and ws_stop() to stop and clean up this instance.
 */
struct ws_instance {
   // User settings
   struct ev_loop *loop;        ///< LibEV loop to start webserver on.
   char *port;                  ///< Port number to start webserver on.
   struct ws_msg* (*header_callback)(const char*,const char*);
   struct ws_msg* (*body_callback)(const char*);
   enum ws_log_level log_level; ///< The log level to use.
   int (*log_cb)(
         struct ws_instance *instance,
         enum ws_log_level log_level,
         const char *fmt, ...); ///< Callback for logging.
   // Internal data
   int sockfd;                  ///< Socket file descriptor.
   struct ev_io watcher;        ///< LibEV IO Watcher for accepting connects.
   void *clients;               ///< Pointer to first client in list
};

/// Create an instance of a webserver
/**
 * Before starting the webserver, an instance of the ws_instance struct is needed
 * This struct is created using this function. You should only use this function to
 * create the ws_instance struct with.
*/
struct ws_instance *ws_create_instance(char *port, struct ws_msg* (*header_callback)(const char*, const char*), struct ws_msg* (*body_callback)(const char*), struct ev_loop *loop);

 /// Free an instance of a webserver
/**
 * When the web server should no longer be used, this function
 * should be called to free the allocated memory.
*/
void ws_free_instance(struct ws_instance *instance);

/// Initialise the webserver instance struct.
/**
 *  Before starting the webserver, an instance of the struct ws_instance
 *  is needed. This struct should be initialised with this function. It
 *  will set default settings for the webserver, which can be changed
 *  later by changing the values in the struct ws_instance.
 *
 *  To start the webserver instance call ws_start(). DO NOT change
 *  settings after a call to ws_start()!
 *
 *  Sample code to start the webserver:
 *  \code
 *  struct ws_instance ws_http;
 *  struct ev_loop *loop = EV_DEFAULT;
 *  
 *  // Init webserver and start it
 *  ws_init(&ws_http, loop);
 *  ws_http.port = "http";
 *  ws_start(&ws_http);
 *  
 *  // Start the loop
 *  ev_run(loop, 0);
 *  
 *  // Clean up the webserver
 *  ws_stop(&ws_http);
 *  \endcode
 *
 *  \param instance Webserver instance to initialise.
 *  \param loop     Event loop to use, when starting the server.
 */
void ws_init(struct ws_instance *instance, struct ev_loop *loop);

/// Start the webserver on a given port.
/**
 *  The libev-based webserver is added to an event loop by a call to
 *  this function. It is the caller's resposibility to start the
 *  event loop, either before or after a call to this.
 *
 *  To stop the webserver again, one may call ws_stop(). See ws_init()
 *  for sample code.
 *
 *  \param instance The webserver instance to start. Initialised with
 *  ws_init();
 */
void ws_start(struct ws_instance *instance);

/// Stop an already running webserver.
/**
 *  The webserver, startet with ws_start(), may be stopped by calling
 *  this function. It will take the webserver off the event loop and
 *  clean up after it.
 *
 *  \param instance The webserver instance to stop.
 */
void ws_stop(struct ws_instance *instance);

#endif

/** } */
