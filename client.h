// client.h

/** \defgroup webserver_client Client
 *  \ingroup webserver
 *
 *  \{
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <ev.h>

/// Initialise and accept client
/**
 *  This function is designed to be used as a callback function within
 *  LibEV. It will accept the conncetion as described inside the file
 *  descripter within the watcher. It will also add timeout and io
 *  watchers to the loop, which will handle the further communication
 *  with the client.
 *
 *  \param loop The running event loop.
 *  \param watcher The watcher that was tiggered on the connection.
 *  \param revents Not used.
 */
void ws_client_accept(struct ev_loop *loop, struct ev_io *watcher, int revents);

#endif

/** } */
