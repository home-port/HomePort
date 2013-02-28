// client.c

#include "client.h"
#include "http-parser/http_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

// TODO What data size shall we use ?
#define MAXDATASIZE 1024

// the time between a connection has been accepted till the client sends something
#define TIMEOUT  15

http_parser_settings settings;

struct watcher_data {
   http_parser parser;
   char ip[INET6_ADDRSTRLEN];
   ev_timer *timeout;
};

struct timeout_data {
   ev_io *watcher;
};

// Get sockaddr, IPv4 or IPv6
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

// Stop a watcher, close its file descripter, and free its data and it
static void kill_watcher(struct ev_loop *loop, struct ev_io *watcher)
{
   ev_io_stop(loop, watcher);
   if (close(watcher->fd) != 0) {
      perror("close");
   }
   free(watcher->data);
   free(watcher);
}

static void kill_timeout(struct ev_loop *loop, struct ev_timer *timeout)
{
   ev_timer_stop(loop, timeout);
   free(timeout->data);
   free(timeout);
}

// Read header
static void hd_reader(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
   int bytes;
   char buffer[MAXDATASIZE];
   struct watcher_data *data = watcher->data;

   kill_timeout(loop, data->timeout);

   // Receive some data
   if ((bytes = recv(watcher->fd, buffer, MAXDATASIZE-1, 0)) < 0) {
      perror("recv");
      // TODO Handle errors better - look up error# etc.
      kill_watcher(loop, watcher);
   } else if (bytes == 0) {
      fprintf(stderr, "connection closed by %s\n", data->ip);
      kill_watcher(loop, watcher);
   }
   buffer[bytes] = '\0';

   // Print message
   printf("%s\n", buffer);

   // Send hello back
   if (send(watcher->fd, "\n\nHello, world!", 15, 0) == -1)
      perror("send");
   kill_watcher(loop, watcher);
}

// Timeout handler
static void timeout_cb(struct ev_loop *loop, struct ev_timer *timer, int revents)
{
   ev_io *watcher = ((struct timeout_data *) timer->data)->watcher;

   printf("timeout: %s\n",((struct watcher_data *) watcher->data)->ip);

   kill_watcher(loop, watcher);
   kill_timeout(loop, timer);
}

void ws_cli_init(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
   struct ev_timer *timeout;
   char s[INET6_ADDRSTRLEN];
   int in_fd;
   socklen_t in_size;
   struct sockaddr_storage in_addr;
   struct ev_io *io_hd_watcher; 
   
   // Accept connection
   in_size = sizeof in_addr;
   if ((in_fd = accept(watcher->fd, (struct sockaddr *)&in_addr, &in_size)) < 0) {
      perror("accept");
      return;
   }

   // Print a nice message
   inet_ntop(in_addr.ss_family, get_in_addr((struct sockaddr *)&in_addr), s, sizeof s);
   printf("got connection from %s\n", s);

   // Create timeout timer
   struct timeout_data *tData = malloc(sizeof(struct timeout_data));
   timeout = malloc(sizeof(struct ev_timer));
   timeout->data = tData;

   // Create new watcher with custom data for this connection
   struct watcher_data *wData = malloc(sizeof(struct watcher_data));
   http_parser_init(&(wData->parser), HTTP_REQUEST);
   strcpy(wData->ip, s);

   io_hd_watcher = malloc(sizeof(struct ev_io));
   io_hd_watcher->data = wData;

   // Set up the timeout and watcher references
   tData->watcher = io_hd_watcher;
   wData->timeout = timeout;

   // Start timeout and watcher
   ev_io_init(io_hd_watcher, hd_reader, in_fd, EV_READ);
   ev_timer_init(timeout, timeout_cb, TIMEOUT, 0.);

   ev_io_start(loop, io_hd_watcher);
   ev_timer_start(loop, timeout);
}

