// instance.c

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

#include "instance.h"
#include "client.h"
#include "linked_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ev.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

/// Instance of a webserver
struct ws {
   struct ws_settings settings;    ///< Settings
   char port_str[6];               ///< Port number - as a string
   struct ev_loop *loop;           ///< Event loop
   struct ll *clients;             ///< Linked List of connected clients
   int sockfd;                     ///< Socket file descriptor
   struct ev_io watcher;           ///< New connection watcher
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

void ws_instance_rm_client(struct ws *instance, struct ws_client
      *client)
{
   struct ll_iter *iter;

   ll_find(iter, instance->clients, client);
   ll_remove(iter);
}
