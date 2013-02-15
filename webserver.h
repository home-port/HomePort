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

/// Start the webserver on a given port.
/**
 *  The libev-based webserver is started by a call to this function. The
 *  webser will run in the same process/thread as the caller, thus this
 *  function will not return before the event loop is finished -- which
 *  would likely be never, unless it is told to stop.
 *
 *  To stop the webserver again, one may call ws_stop().
 *
 *  \param port Port number to start webserver on.
 */
void ws_start(char *port);

/// Stop an already running webserver.
/**
 *  The webserver, startet with ws_start(), may be stopped by calling
 *  this function, which will break the event-loop and force ws_start to
 *  clean-up and return.
 */
void ws_stop();

#endif

/** } */
