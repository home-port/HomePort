// webserver.c

#include "webserver.h"
#include "client.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ev.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

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
      exit(1);
   }

   // Loop through results and bind to first
   for (p = servinfo; p != NULL; p=p->ai_next) {

      // Create socket
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
         perror("socket");
         continue;
      }

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
      fprintf(stderr, "failed to bind\n");
      exit(1);
   }

   // Clean up
   freeaddrinfo(servinfo);

   // Listen on socket
   if (listen(sockfd, SOMAXCONN) < 0) {
      perror("listen");
      exit(1);
   }

   return sockfd;
}

void ws_init(struct ws_instance *instance, struct ev_loop *loop) {
   // Set default settings
   instance->port = "http";
   instance->loop = loop;
}

void ws_start(struct ws_instance *instance)
{
   // Check port
   if (instance->port == NULL) {
      fprintf(stderr, "Warning: No port number given, starting server \
            on 'http'");
      instance->port = "http";
   }

   // Check loop
   if (instance->loop == NULL) {
      fprintf(stderr, "Warning: No event loop given, starting server \
            on 'EV_DEFAULT' loop. Loop will not be started.");
      instance->loop = EV_DEFAULT;
   }

   // Start server
   printf("Starting server on port '%s'\n", instance->port);
   instance->sockfd = bind_listen(instance->port);
   ev_io_init(&instance->watcher, ws_cli_init, instance->sockfd, EV_READ);
   ev_io_start(instance->loop, &instance->watcher);
}

void ws_stop(struct ws_instance *instance)
{
   // Stop accept watcher
   ev_io_stop(instance->loop, &instance->watcher);

   // Close socket
   if (close(instance->sockfd) != 0) {
      perror("close");
   }
}

