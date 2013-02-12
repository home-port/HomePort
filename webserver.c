// webserver.c

#include "webserver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ev.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

//#include <errno.h>
//#include <netinet/in.h>
//#include <sys/wait.h>
//#include <signal.h>

static int get_socket(char *port) {
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
         perror("socket error");
         continue;
      }

      // Bind to socket
      if (bind(sockfd, p->ai_addr, p->ai_addrlen) != 0) {
         close(sockfd);
         perror("bind error");
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
      perror("listen error");
      exit(1);
   }

   return sockfd;
}

// Get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET) {
      // IPv4
      return &(((struct sockaddr_in*)sa)->sin_addr);
   } else {
      // IPv6
      return &(((struct sockaddr_in6*)sa)->sin6_addr);
   }
}

static void callback(struct ev_loop *loop, struct ev_io *watcher, int revents) {
   int in_fd;
   socklen_t in_size;
   struct sockaddr_storage in_addr;
   char s[INET6_ADDRSTRLEN];

   // Accept connection
   in_size = sizeof in_addr;
   if ((in_fd = accept(watcher->fd, (struct sockaddr *)&in_addr, &in_size)) < 0) {
      perror("accept error");
      return;
   }

   // Print a nice message
   inet_ntop(in_addr.ss_family, get_in_addr((struct sockaddr *)&in_addr), s, sizeof s);
   printf("got connection from %s\n", s);

   if (send(in_fd, "Hello, world!", 13, 0) == -1)
      perror("send");
   close(in_fd);

   fprintf(stderr, "Someone forgot to write me :(\n");
}

void start(char *port) {
   struct ev_loop *loop = EV_DEFAULT;
   struct ev_io io_watcher;

   int sockfd = get_socket(port);

   ev_io_init(&io_watcher, callback, sockfd, EV_READ);
   ev_io_start(loop, &io_watcher);

   ev_run(loop, 0);

   close(sockfd);
}

void stop() {
}

