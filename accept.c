// accept.c

#include "accept.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

struct data {
   int sockfd;
};

static int get_socket(char *port)
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
   if ((status = getaddrinfo(NULL, "http", &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
   }

   // Loop through results and bind to first
   // TODO Should we open a socket for each, or just the first ?
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

struct ev_io *ws_acc_init(struct ev_loop *loop, char *port, void
      (*accept_f)(struct ev_loop *, struct ev_io *, int))
{
   // Get data
   int sockfd = get_socket(port);
   
   // Prepare watcher
   struct ev_io *watcher = malloc(sizeof(struct ev_io));
   struct data *data = malloc(sizeof(struct data));
   data->sockfd = sockfd;
   watcher->data = data;

   // Init and start it
   ev_io_init(watcher, accept_f, sockfd, EV_READ);
   ev_io_start(loop, watcher);

   return watcher;
}

void ws_acc_deinit(struct ev_io *watcher)
{
   struct data *data = watcher->data;
   int sockfd = data->sockfd;

   // Close socket
   if (close(sockfd) != 0) {
      perror("close");
   }

   // Clean up
   free(watcher->data);
   free(watcher);
}

