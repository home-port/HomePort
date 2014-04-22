// webserver.c

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

#include "webserver.h"
#include "linked_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ev.h>
#include <fcntl.h>
#include <ctype.h>

/// Instance of a webserver
struct ws {
   struct ws_settings settings;    ///< Settings
   char port_str[6];               ///< Port number - as a string
   struct ev_loop *loop;           ///< Event loop
   struct ll *conns;               ///< Linked List of connections
   int sockfd;                     ///< Socket file descriptor
   struct ev_io watcher;           ///< New connection watcher
};

/// All data to represent a connection
struct ws_conn {
   struct ws *instance;             ///< Webserver instance
   char ip[INET6_ADDRSTRLEN];       ///< IP address of client
   struct ev_timer timeout_watcher; ///< Timeout watcher
   int timeout;                     ///< Restart timeout watcher ?
   struct ev_io recv_watcher;       ///< Recieve watcher
   struct ev_io send_watcher;       ///< Send watcher
   char *send_msg;                  ///< Data to send
   size_t send_len;                 ///< Length of data to send
   int send_close;                  ///< Close socket after send ?
   void *ctx;                       ///< Connection context
};

void ws_print(struct ws *ws)
{
   struct ll_iter *iter;
   size_t conns;

   printf("----- Webserver -----\n");
   if (!ws) {
      printf("   (null)\n");
   } else {
      printf("   Port: %i\n", ws->settings.port);
      printf("   Connection timeout: %i\n", ws->settings.timeout);
      printf("   Chunk size: %lu\n", ws->settings.maxdatasize);
      printf("   Callbacks set:");
      if (ws->settings.on_connect) printf(" connect");
      if (ws->settings.on_receive) printf(" receive");
      if (ws->settings.on_disconnect) printf(" disconnect");
      printf("\n");
      ll_count(iter, ws->conns, conns);
      printf("   Connections: %lu\n", conns);
      printf("   Socket descriptor: %i\n", ws->sockfd);
   }
}

void ws_conn_print(struct ws_conn *conn)
{
   size_t i;

   printf("----- Connection -----\n");
   if (!conn) {
      printf("   (null)\n");
   } else {
      printf("   Client: %s\n", conn->ip);
      printf("   Timeout: ");
      if (conn->timeout) printf("Active\n");
      else printf("Deactive\n");
      printf("   Data waiting (%lu chars): '", conn->send_len);
      for (i = 0; i < conn->send_len; i++) {
         if (conn->send_msg[i] == '\\') printf("\\\\");
         else if (isprint(conn->send_msg[i])) printf("%c", conn->send_msg[i]);
         else printf("\\%i", conn->send_msg[i]);
      }
      printf("'\n");
      printf("   After send: ");
      if (conn->send_close) printf("Close connection\n");
      else printf("Keep open\n");
   }
}

/// Get the socket file descriptor for a port number.
/**
 *  This will also bind and start listening to the socket. Supports both
 *  ipv4 and ipv6.
 *
 *  \param port The port number to bind to and listen on.
 *
 *  \return The socket file descriptor, that should be used later for
 *  closing again.
 */
static int bind_listen(char *port) 
{
   int status, sockfd;
   struct addrinfo hints;
   struct addrinfo *servinfo, *p;

   // Clear struct and set requirements for socket
   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;     // IPv4/IPv6
   hints.ai_socktype = SOCK_STREAM; // TCP Stream
   hints.ai_flags = AI_PASSIVE;     // Wildcard address

   // Get address infos we later use to open socket with
   if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      freeaddrinfo(servinfo);
      return -1;
   }

   // Loop through results and bind to first
   for (p = servinfo; p != NULL; p=p->ai_next) {

      // Create socket
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
         perror("socket");
         continue;
      }

      // Change to non-blocking sockets
      fcntl(sockfd, F_SETFL, O_NONBLOCK);

      // TODO Reuse addr is on for testing purposes
#ifdef DEBUG
      int yes = 1;
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
         close(sockfd);
         perror("setsockopt");
         continue;
      }
#endif

      // Bind to socket
      if (bind(sockfd, p->ai_addr, p->ai_addrlen) != 0) {
         close(sockfd);
         perror("bind");
         continue;
      }

      break;
   }

   // Check if we binded to anything
   if (p == NULL) {
      close(sockfd);
      fprintf(stderr, "failed to bind\n");
      freeaddrinfo(servinfo);
      return -1;
   }

   // Clean up
   freeaddrinfo(servinfo);

   // Listen on socket
   if (listen(sockfd, SOMAXCONN) < 0) {
      close(sockfd);
      perror("listen");
      return -1;
   }

   return sockfd;
}

/// Get the in_addr from a sockaddr (IPv4 or IPv6)
/**
 *  Get the in_addr for either IPv4 or IPv6. The type depends on the
 *  protocal, which is why this returns a void pointer. It works nicely
 *  to pass the result of this function as the second argument to
 *  inet_ntop().
 *
 *  \param sa The sockaddr
 *
 *  \return An in_addr or in6_addr depending on the protocol.
 */
static void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET) {
      // IPv4
      return &(((struct sockaddr_in*)sa)->sin_addr);
   } else {
      // IPv6
      return &(((struct sockaddr_in6*)sa)->sin6_addr);
   }
}

/// Recieve callback for io-watcher
/**
  * Recieves up to maxdatasize (from struct ws_settings) of data from a
  * connection and calls on_recieve with it. Also resets the timeout for
  * the connection, if one.
  *
  * \param  loop     The event loop
  * \param  watcher  The io watcher causing the call
  * \param  revents  Not used
  */
static void conn_recv_cb(struct ev_loop *loop, struct ev_io *watcher,
                         int revents)
{
   ssize_t recieved;
   struct ws_conn *conn = watcher->data;
   struct ws_settings *settings = &conn->instance->settings;
   size_t maxdatasize = settings->maxdatasize;
   char buffer[maxdatasize];

   printf("recieving data from %s\n", conn->ip);
   if ((recieved = recv(watcher->fd, buffer, maxdatasize-1, 0)) < 0) {
      if (recieved == -1 && errno == EWOULDBLOCK) {
         fprintf(stderr, "libev callbacked called without data to " \
                         "recieve (conn: %s)", conn->ip);
         return;
      }
      perror("recv");
      ws_conn_kill(conn);
      return;
   } else if (recieved == 0) {
      printf("ws: Connection closed by %s\n", conn->ip);
      ws_conn_kill(conn);
      return;
   }

   if (settings->on_receive) {
      if (settings->on_receive(conn->instance, conn, 
                               settings->ws_ctx, &conn->ctx,
                               buffer, recieved)) {
         ws_conn_kill(conn);
         return;
      }
   }

   // Reset timeout
   if (conn->timeout) {
      conn->timeout_watcher.repeat = conn->instance->settings.timeout;
      ev_timer_again(loop, &conn->timeout_watcher);
   }
}

/// Send callback for io-watcher
/**
 * Sends message stored in send_msg on the connection. If not all the
 * data could be sent at once, the remainer is store in send_msg again
 * and the watcher is not stopped. If a connection is flaggted with
 * close, the connection is closed when all the data has been sent.
  *
  * \param  loop     The event loop
  * \param  watcher  The io watcher causing the call
  * \param  revents  Not used
 */
static void conn_send_cb(struct ev_loop *loop, struct ev_io *watcher,
      int revents)
{
   struct ws_conn *conn = watcher->data;
   size_t sent;

   sent = send(watcher->fd, conn->send_msg, conn->send_len, 0);
   if (sent == -1) {
      perror("send");
   } else if (sent == conn->send_len) {
      free(conn->send_msg);
      conn->send_msg = NULL;
      conn->send_len = 0;
   } else {
      conn->send_len -= sent;
      char *s = malloc(conn->send_len*sizeof(char));
      if (!s) {
         fprintf(stderr, "Cannot allocate enough memory\n");
         free(conn->send_msg);
         conn->send_msg = NULL;
         conn->send_len = 0;
      } else {
         strcpy(s, &conn->send_msg[sent]);
         free(conn->send_msg);
         conn->send_msg = s;
         return;
      }
   }

   ev_io_stop(conn->instance->loop, &conn->send_watcher);
   if (conn->send_close) ws_conn_kill(conn);
}

/// Timeout callback for timeout watcher
/**
 * Kills the connection on timeout
  *
  * \param  loop     The event loop
  * \param  watcher  The io watcher causing the call
  * \param  revents  Not used
 */
static void conn_timeout_cb(struct ev_loop *loop,
                            struct ev_timer *watcher,
                            int revents)
{
   struct ws_conn *conn = watcher->data;
   printf("timeout on %s [%ld]\n", conn->ip, (long)conn);
   ws_conn_kill(conn);
}

/// Add a connection to an instance
/**
 *  Adds an already etablished connection to a webserver instance.
 *
 *  \param  instance  The webser instance
 *  \param  conn      The connection to add
 *
 *  \return 0 on success, 1 on error
 */
static int ws_instance_add_conn(struct ws *instance,
                                struct ws_conn *conn)
{
   struct ll_iter *it = ll_tail(instance->conns);
   ll_insert(instance->conns, it, conn);
   if (it) it = ll_next(it);
   else it = ll_tail(instance->conns);
   if (it == NULL) {
      fprintf(stderr, "Not enough memory to add connection\n");
      return 1;
   }
   return 0;
}

/// Initialise and accept connection
/**
 *  This function is designed to be used as a callback function within
 *  LibEV. It will accept the conncetion as described inside the file
 *  descripter within the watcher. It will also add timeout and io
 *  watchers to the loop, which will handle the further communication
 *  with the connection.
 *
 *  \param loop The running event loop.
 *  \param watcher The watcher that was tiggered on the connection.
 *  \param revents Not used.
 */
static void ws_conn_accept(
      struct ev_loop *loop,
      struct ev_io *watcher,
      int revents)
{
   char ip_string[INET6_ADDRSTRLEN];
   int in_fd;
   socklen_t in_size;
   struct sockaddr_storage in_addr_storage;
   struct sockaddr *in_addr = (struct sockaddr *)&in_addr_storage;
   struct ws_conn *conn;
   struct ws_settings *settings = &((struct ws *)watcher->data)->settings;
   
   // Accept connection
   in_size = sizeof in_addr;
   if ((in_fd = accept(watcher->fd, in_addr, &in_size)) < 0) {
      perror("accept");
      return;
   }

   // Print a nice message
   inet_ntop(in_addr_storage.ss_family,
         get_in_addr(in_addr),
         ip_string,
         sizeof ip_string);
   printf("ws: Got connection from %s\n", ip_string);

   // Create conn and parser
   conn = malloc(sizeof(struct ws_conn));
   if (conn == NULL) {
      fprintf(stderr, "Cannot allocation memory for connection\n");
      close(in_fd);
      return;
   }
   conn->instance = watcher->data;
   strcpy(conn->ip, ip_string);
   conn->timeout_watcher.data = conn;
   conn->recv_watcher.data = conn;
   conn->send_watcher.data = conn;
   conn->ctx = NULL;
   conn->send_msg = NULL;
   conn->send_len = 0;
   conn->send_close = 0;
   conn->timeout = 1;

   // Set up list
   ws_instance_add_conn(conn->instance, conn);

   // Call back
   if (settings->on_connect) {
      if (settings->on_connect(conn->instance, conn,
                               settings->ws_ctx, &conn->ctx)) {
         ws_conn_kill(conn);
         return;
      }
   }

   // Start timeout and io watcher
   ev_io_init(&conn->recv_watcher, conn_recv_cb, in_fd, EV_READ);
   ev_io_init(&conn->send_watcher, conn_send_cb, in_fd, EV_WRITE);
   ev_io_start(loop, &conn->recv_watcher);
   ev_init(&conn->timeout_watcher, conn_timeout_cb);
   conn->timeout_watcher.repeat = settings->timeout;
   if (conn->timeout)
      ev_timer_again(loop, &conn->timeout_watcher);
}

/// Send message on connection
/**
 * This function is used similary to the standard printf function, with
 * a format string and variable arguments. It calls ws_conn_vsendf() to
 * handle the actually sending, see this for more information.
 *
 * Connection is kept open for further communication, use ws_conn_close
 * to close it.
 *
 * \param  conn  Connection to send on
 * \param  fmt   Format string
 *
 * \return The same as ws_conn_vsendf() 
 */
int ws_conn_sendf(struct ws_conn *conn, const char *fmt, ...) {
   int stat;
   va_list arg;

   va_start(arg, fmt);
   stat = ws_conn_vsendf(conn, fmt, arg);
   va_end(arg);

   return stat;
}

/// Send message on connection
/**
 * This function is simiar to the standard vprintf function, with a
 * format string and a list of variable arguments.
 *
 * Note that this function only schedules the message to be send. A send
 * watcher on the event loop will trigger the actual sending, when the
 * connection is ready for it.
 *
 * Connection is kept open for further communication, use ws_conn_close
 * to close it.
 *
 * \param  conn  Connection to send on
 * \param  fmt   Format string
 * \param  arg   List of arguments
 *
 * \return  zero on success, -1 or the return value of vsprintf on
 *          failure
 */
int ws_conn_vsendf(struct ws_conn *conn, const char *fmt, va_list arg)
{
   int stat;
   char *new_msg;
   size_t new_len;
   va_list arg2;

   // Copy arg to avoid errors on 64bit
   va_copy(arg2, arg);

   // Get the length to expand with
   new_len = vsnprintf("", 0, fmt, arg);

   // Expand message to send
   new_msg = realloc(conn->send_msg,
         (conn->send_len + new_len + 1)*sizeof(char));
   if (new_msg == NULL) {
      fprintf(stderr, "Cannot allocate enough memory\n");
      return -1;
   }
   conn->send_msg = new_msg;

   // Concatenate strings
   stat = vsprintf(&(conn->send_msg[conn->send_len]), fmt, arg2);

   // Start send watcher
   if (conn->send_len == 0 && conn->instance != NULL)
      ev_io_start(conn->instance->loop, &conn->send_watcher);

   // Update length
   conn->send_len += new_len;

   if (stat < 0) return stat;
   else return 0;
}

/// Remove connection from instance
/**
 * This will remove a connection from a webserver instance. Will NOT
 * free or close the connection.
 *
 * \param  instance  The webserver instance
 * \param  conn      The connection to remove
 */
static void ws_instance_rm_conn(struct ws *instance, struct ws_conn
      *conn)
{
   struct ll_iter *iter;

   ll_find(iter, instance->conns, conn);
   ll_remove(iter);
}

/// Close a connection, after the remaining data has been sent
/**
 * This sets the close flag on a connection. The connection will be
 * closed after the remaining messages has been sent. If there is no
 * waiting messages the connection will be closed instantly.
 *
 * \param  conn  The connection to close
 */
void ws_conn_close(struct ws_conn *conn) {
   conn->send_close = 1;
      
   if (conn->send_msg == NULL) {
      ev_io_stop(conn->instance->loop, &conn->send_watcher);
      if (conn->send_close) ws_conn_kill(conn);
   }
}

/// Disable timeout on connection
/**
 *  Every connection have per default a timeout value, which is set in
 *  the struct ws_settings. If there is no activity on the connection
 *  before the timeout run out the connection is killed. This function
 *  disables the timeout, so connections will stay open. A connection
 *  will still be killed when the client closes the connection, or
 *  kill/close is called.
 *
 *  \param  conn  The connection to keep open
 */
void ws_conn_keep_open(struct ws_conn *conn)
{
   conn->timeout = 0;
   ev_timer_stop(conn->instance->loop, &conn->timeout_watcher);
}

/// Kill and clean up after a connection
/**
 *  This function stops the LibEV watchers, closes the socket, and frees
 *  the data structures used by a connection.
 *
 *  Note that you should use ws_conn_close for a graceful closure of the
 *  connection, where the remaining data is sent.
 *
 *  \param conn The connection to kill.
 */
void ws_conn_kill(struct ws_conn *conn)
{
   struct ws_settings *settings = &conn->instance->settings;

   // Print messange
   printf("ws: Killing connection %s\n", conn->ip);

   int sockfd = conn->recv_watcher.fd;

   // Stop circular calls and only kill this connection once
   if (sockfd < 0) return;

   // Stop watchers
   ev_io_stop(conn->instance->loop, &conn->recv_watcher);
   ev_io_stop(conn->instance->loop, &conn->send_watcher);
   ev_timer_stop(conn->instance->loop, &conn->timeout_watcher);

   // Close socket
   if (close(sockfd) != 0) {
      perror("close");
   }
   conn->recv_watcher.fd = -1;

   // Remove from list
   ws_instance_rm_conn(conn->instance, conn);

   // Call back
   if (settings->on_disconnect)
      settings->on_disconnect(conn->instance, conn,
                              settings->ws_ctx, &conn->ctx);

   // Cleanup
   free(conn->send_msg);
   free(conn);
}

/// Destroy webserver and free used memory
/**
 *  This function destroys and frees all connections are instances. The
 *  webserver should be stopped before destroy to properly close all
 *  connections and sockets first.
 *
 *  \param  instance  The webserver instance to destroy
 */
void ws_destroy(struct ws *instance)
{
   ll_destroy(instance->conns);
   free(instance);
}

/// Create new webserver instance
/**
 *  This creates a new webserver instances, that can be started with
 *  ws_start() and stopped with ws_stop(). The instance should be
 *  destroyed with ws_destroy when not longer needed.
 *
 *  \param  settings  The settings for the webserver.
 *  \param  loop      The event loop to run webserver on.
 *
 *  \return  The new webserver instance.
 */
struct ws *ws_create(
      struct ws_settings *settings,
      struct ev_loop *loop)
{
   struct ws *instance = malloc(sizeof(struct ws));
   if (instance == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory for a new " \
                      "webserver struct\n");
      return NULL;
   }

   memcpy(&instance->settings, settings, sizeof(struct ws_settings));
   sprintf(instance->port_str, "%i", settings->port);

   instance->loop = loop;
   ll_create(instance->conns);
   if (instance->conns == NULL) {
      fprintf(stderr, "ERROR: Cannot allocate memory for a new " \
                      "webserver struct\n");
      ws_destroy(instance);
      return NULL;
   }

   return instance;
}

/// Start the webserver
/**
 *  The libev-based webserver is added to an event loop by a call to
 *  this function. It is the caller's resposibility to start the
 *  event loop.
 *
 *  To stop the webserver again, one may call ws_stop().
 *
 *  \param instance The webserver instance to start. Created with
 *  ws_create();
 *
 *  \return  0 on success, 1 on error.
 */
int ws_start(struct ws *instance)
{
   // Check loop
   if (instance->loop == NULL) {
      fprintf(stderr, "No event loop given, starting server on " \
            "'EV_DEFAULT' loop. Loop will not be started.");
      instance->loop = EV_DEFAULT;
   }

   // Print message
   printf("ws: Starting server on port '%s'\n", instance->port_str);

   // Bind to socket
   instance->sockfd = bind_listen(instance->port_str);
   if (instance->sockfd < 0) {
      fprintf(stderr, "Could not bind to port [%s]\n",
            instance->port_str);
      return 1;
   }

   // Set listener on libev
   instance->watcher.data = instance;
   ev_io_init(&instance->watcher, ws_conn_accept, instance->sockfd,
              EV_READ);
   ev_io_start(instance->loop, &instance->watcher);

   return 0;
}

/// Stop an already running webserver.
/**
 *  The webserver, started with ws_start(), may be stopped by calling
 *  this function. It will take the webserver off the event loop and
 *  clean up after it.
 *
 *  This includes killing all connections without waiting for
 *  remaining data to be sent.
 *
 *  \param instance The webserver instance to stop.
 */
void ws_stop(struct ws *instance)
{
   struct ll_iter *it, *next;

   // Stop accept watcher
   ev_io_stop(instance->loop, &instance->watcher);

   // Kill all connections
   it = ll_head(instance->conns);
   while (it != NULL) {
      next = ll_next(it);
      ws_conn_kill(ll_data(it));
      it = next;
   }

   // Close socket
   if (close(instance->sockfd) != 0) {
      perror("close");
   }
}

/// Get the IP address of the client
/**
 *  \param  conn  The connection on which the client is connected.
 *
 *  \return  The IP address in a string.
 */
const char *ws_conn_get_ip(struct ws_conn *conn)
{
   return conn->ip;
}

