// webserver.h

/** \defgroup webserver Webserver
 *  The webserver is a single threaded event based webserver, built on
 *  top of the event loop implemented in LibEV.
 *  
 *  The overall webserver consists of three sub-modules:
 *
 *  \dot
 *  graph example {
 *     node [shape=record, fontname=Helvetica, fontsize=10];
 *     webserver [ label="Webserver" URL="\ref webserver.h"];
 *     accept [ label="Accept" URL="\ref accept.h"];
 *     client [ label="Client" URL="\ref client.h"];
 *     webserver -- accept;
 *     webserver -- client;
 *  }
 *  \enddot
 *
 *  The \ref webserver.h "Webserver submodule" is the interface for
 *  anyone that needs to development using this webserver
 *  implementation.  Most users only needs to know the implementation
 *  within this submodule.
 *
 *  The \ref accept.h "Accept submodule" handles the acceptance of new
 *  clients. The module takes a callback, which it will call on any new
 *  client connecting to the webserver. This callback will be one from
 *  the client submodule.
 *
 *  The \ref client.h "Client submodule" handles the communication with
 *  a single client connected to the webserver. It takes over as the
 *  accepter of any connection made to the accept submodule. 
 *
 *  \{
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ev.h>

// TODO How do I "hide" the internal data of this struct ?
struct ws_instance {
   // User settings
   char *port;
   struct ev_loop *loop;
   // Internal data
   int sockfd;
   struct ev_io watcher;
};

void ws_init(struct ws_instance *instance, struct ev_loop *loop);

/// Start the webserver on a given port.
/**
 *  The libev-based webserver is added to an event loop by a call to
 *  this function. It is the caller's resposibility to start the
 *  event loop, either before or after a call to this.
 *
 *  To stop the webserver again, one may call ws_stop().
 *
 *  General code to start one instance of the weserver (on port 80):
 *  \code
 *  struct ev_loop *loop = EV_DEFAULT;
 *  ws_start(80, loop);
 *  ev_run(loop, 0);
 *  \endcode
 *
 *  \param port Port number to start webserver on.
 *  \param loop The event loop to use.
 */
void ws_start(struct ws_instance *instance);

/// Stop an already running webserver.
/**
 *  The webserver, startet with ws_start(), may be stopped by calling
 *  this function. It will take the webserver off the event loop and
 *  clean up after it.
 */
void ws_stop(struct ws_instance *instance);

#endif

/** } */
