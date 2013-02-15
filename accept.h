// accept.h

/** \defgroup webserver_accept Accept
 *  \ingroup webserver
 *
 *  \{
 */

#ifndef ACCEPT_H
#define ACCEPT_H

#include <ev.h>

/// Initialise the accept watcher.
/**
 *  The accept watcher opens a socket and waits for new clients to
 *  connect to this socket. When a new client connects it calls the
 *  accept function, which is responsible for accepting the
 *  connection and handling it from there.
 *
 *  \param loop The event-loop to add this watcher to.
 *  \param port The port number to listen on.
 *  \param accept_f The accept function, that handles acceptance of the
 *  new clients. 
 *
 *  \return A pointer to the created watcher, which has already been
 *  added to the event loop. You should free this pointer with
 *  ws_acc_deinit() when the event loop breaks. 
 */
struct ev_io *ws_acc_init(struct ev_loop *loop, char *port,
      void (*accept_f)(struct ev_loop *, struct ev_io *, int));

/// Free the watcher returned by ws_acc_init().
/**
 *  The watcher returned by ws_acc_init should be freed, when the event
 *  loop breaks or the watcher is removed from the event loop.
 *
 *  \param watcher The watcher to be freed.
 */
void ws_acc_deinit(struct ev_io *watcher);

#endif

/** } */

