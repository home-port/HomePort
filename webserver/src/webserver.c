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
#include <stdarg.h>
#include <ev.h>
#include <fcntl.h>

/// The maximum data size we can recieve or send
#define MAXDATASIZE 1024

/// The amount of time a client may be inactive, in seconds
#define TIMEOUT 15

/// Instance of a webserver
struct ws {
   struct ws_settings settings;    ///< Settings
   char port_str[6];               ///< Port number - as a string
   struct ev_loop *loop;           ///< Event loop
   struct ll *clients;             ///< Linked List of connected clients
   int sockfd;                     ///< Socket file descriptor
   struct ev_io watcher;           ///< New connection watcher
};

/// All data to represent a client
struct ws_client {
   struct ws *instance;             ///< Webserver instance
   struct ws_settings *settings;    ///< Webserver settings
   char ip[INET6_ADDRSTRLEN];       ///< IP address of the client
   struct ev_loop *loop;            ///< The event loop
   struct ev_timer timeout_watcher; ///< Timeout watcher
   struct ev_io recv_watcher;       ///< Recieve watcher
   struct ev_io send_watcher;       ///< Send watcher
   char send_msg[MAXDATASIZE];      ///< Data to send
   void *ctx;
};

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

static void client_recv_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
   ssize_t recieved;
   size_t parsed;
   char buffer[MAXDATASIZE];
   struct ws_client *client = watcher->data;

   printf("recieving data from %s\n", client->ip);

   // Receive some data
   if ((recieved = recv(watcher->fd, buffer, MAXDATASIZE-1, 0)) < 0) {
      if (recieved == -1 && errno == EWOULDBLOCK) {
         fprintf(stderr, "libev callbacked called without data to " \
                         "recieve (client: %s)", client->ip);
         return;
      }
      perror("recv");
      // TODO Handle errors better - look up error# etc.
      ws_client_kill(client);
      return;
   } else if (recieved == 0) {
      printf("connection closed by %s\n", client->ip);
      ws_client_kill(client);
      return;
   }

   if (client->settings->on_receive(client->instance, client,
                                    buffer, recieved)) {
      ws_client_kill(client);
      return;
   }

   // Reset timeout
   client->timeout_watcher.repeat = TIMEOUT;
   ev_timer_again(loop, &client->timeout_watcher);
}

static void client_send_cb(struct ev_loop *loop, struct ev_io *watcher,
      int revents)
{
   struct ws_client *client = watcher->data;

   printf("sending response to %s\n", client->ip);
   if (send(watcher->fd, client->send_msg, strlen(client->send_msg), 0) == -1)
      perror("send");
   ev_io_stop(client->loop, &client->send_watcher);

   ws_client_kill(client);
}

static void client_timeout_cb(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
   struct ws_client *client = watcher->data;
   printf("timeout on %s\n", client->ip);
   ws_client_kill(client);
}

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
static void ws_client_accept(
      struct ev_loop *loop,
      struct ev_io *watcher,
      int revents)
{
   char ip_string[INET6_ADDRSTRLEN];
   int in_fd;
   socklen_t in_size;
   struct sockaddr_storage in_addr;
   struct ws_client *client;
   
   // Accept connection
   in_size = sizeof in_addr;
   if ((in_fd = accept(watcher->fd, (struct sockaddr *)&in_addr, &in_size)) < 0) {
      perror("accept");
      return;
   }

   // Print a nice message
   inet_ntop(in_addr.ss_family,
         get_in_addr((struct sockaddr *)&in_addr),
         ip_string,
         sizeof ip_string);
   printf("got connection from %s\n", ip_string);

   // Create client and parser
   client = malloc(sizeof(struct ws_client));
   if (client == NULL) {
      fprintf(stderr, "ERROR: Cannot accept client (malloc return NULL)\n");
      return;
   }
   client->instance = watcher->data;
   client->settings = &client->instance->settings; 
   strcpy(client->ip, ip_string);
   client->loop = loop;
   client->timeout_watcher.data = client;
   client->recv_watcher.data = client;
   client->send_watcher.data = client;
   client->ctx = NULL;

   // Set up list
   ws_instance_add_client(client->instance, client);

   // Call back
   if (client->settings->on_connect) {
      if (client->settings->on_connect(client->instance, client)) {
         ws_client_kill(client);
         return;
      }
   }

   // Start timeout and io watcher
   ev_io_init(&client->recv_watcher, client_recv_cb, in_fd, EV_READ);
   ev_io_init(&client->send_watcher, client_send_cb, in_fd, EV_WRITE);
   ev_io_start(loop, &client->recv_watcher);
   ev_init(&client->timeout_watcher, client_timeout_cb);
   client->timeout_watcher.repeat = TIMEOUT;
   ev_timer_again(loop, &client->timeout_watcher);
}

void ws_client_sendf(struct ws_client *client, char *fmt, ...) {
   int status;
   va_list arg;

   va_start(arg, fmt);
   status = vsnprintf(client->send_msg, MAXDATASIZE, fmt, arg);
   va_end(arg);

   if (status >= MAXDATASIZE) {
      fprintf(stderr, "Data is too large to send!");
      ws_client_kill(client);
      return;
   }

   ev_io_start(client->loop, &client->send_watcher);
}

static void ws_instance_rm_client(struct ws *instance, struct ws_client
      *client)
{
   struct ll_iter *iter;

   ll_find(iter, instance->clients, client);
   ll_remove(iter);
}

/// Kill and clean up after a client
/**
 *  This function stops the LibEV watchers, closes the socket, and frees
 *  the data structures used be a client.
 *
 *  \param client The client to kill.
 */
void ws_client_kill(struct ws_client *client) {

   printf("killing client %s\n", client->ip);

   // Stop watchers
   int sockfd = client->recv_watcher.fd;
   ev_io_stop(client->loop, &client->recv_watcher);
   ev_io_stop(client->loop, &client->send_watcher);
   ev_timer_stop(client->loop, &client->timeout_watcher);

   // Close socket
   if (close(sockfd) != 0) {
      perror("close");
   }

   // Remove from list
   ws_instance_rm_client(client->instance, client);

   // Call back
   if (client->settings->on_disconnect)
      client->settings->on_disconnect(client->instance, client);

   // Cleanup
   free(client);
}

void ws_client_set_ctx(struct ws_client *client, void *ctx)
{
   client->ctx = ctx;
}

void *ws_client_get_ctx(struct ws_client *client)
{
   return client->ctx;
}

void ws_destroy(struct ws *instance)
{
   ll_destroy(instance->clients);
   free(instance);
}

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
   ll_create(instance->clients);
   if (instance->clients == NULL) {
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
 *  \param instance The webserver instance to start. Initialised with
 *  ws_init();
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
   printf("Starting server on port '%s'\n", instance->port_str);

   // Bind to socket
   instance->sockfd = bind_listen(instance->port_str);
   if (instance->sockfd < 0) {
      fprintf(stderr, "Could not bind to port [%s]\n",
            instance->port_str);
      return 1;
   }

   // Set listener on libev
   instance->watcher.data = instance;
   ev_io_init(&instance->watcher, ws_client_accept, instance->sockfd,
              EV_READ);
   ev_io_start(instance->loop, &instance->watcher);

   return 0;
}

/// Stop an already running webserver.
/**
 *  The webserver, startet with ws_start(), may be stopped by calling
 *  this function. It will take the webserver off the event loop and
 *  clean up after it.
 *
 *  \param instance The webserver instance to stop.
 */
void ws_stop(struct ws *instance)
{
   struct ll_iter *it;

   // Stop accept watcher
   ev_io_stop(instance->loop, &instance->watcher);

   // Kill all clients
   for (it = ll_head(instance->clients); it != NULL; it = ll_next(it)) {
      ws_client_kill(ll_data(it));
   }

   // Close socket
   if (close(instance->sockfd) != 0) {
      perror("close");
   }
}

struct ws_settings *ws_instance_get_settings(
      struct ws *instance)
{
   return &instance->settings;
}

int ws_instance_add_client(struct ws *instance, struct ws_client
      *client)
{
   struct ll_iter *it = ll_tail(instance->clients);
   ll_insert(instance->clients, it, client);
   if (it) it = ll_next(it);
   else it = ll_tail(instance->clients);
   if (it == NULL) {
      fprintf(stderr, "Not enough memory to add client\n");
      return 1;
   }
   return 0;
}

void *ws_get_ctx(struct ws *instance)
{
   return instance->settings.ws_ctx;
}


































