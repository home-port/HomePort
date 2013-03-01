// webserver.h

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
   char *port;           ///< Port number to start webserver on.
   struct ev_loop *loop; ///< LibEV loop to start webserver on.
   // Internal data
   int sockfd;           ///< Socket file descriptor.
   struct ev_io watcher; ///< LibEV IO Watcher for accepting connects.
};

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
