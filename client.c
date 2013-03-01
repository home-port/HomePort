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

/// The maximum data size we can recieve
#define MAXDATASIZE 1024

/// The amount of time a client may be inactive
#define TIMEOUT  15

/// All data to represent a client
struct ws_client {
   char ip[INET6_ADDRSTRLEN];            ///< IP address of client.
   struct ev_loop *loop;                 ///< The event loop.
   struct ev_timer timeout_watcher;      ///< LibEV watcher for timeout.
   struct ev_io io_watcher;              ///< LibEV watcher for data.
   http_parser parser;                   ///< The parser in use.
   http_parser_settings parser_settings; ///< Settings for the parser.
};

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

/// Kill and clean up after a client
/**
 *  This function stops the LibEV watchers, closes the socket, and frees
 *  the data structures used be a client.
 *
 *  \param client The client to kill.
 */
static void kill_client(struct ws_client *client) {
   // Stop watchers
   ev_io_stop(client->loop, &client->io_watcher);
   ev_timer_stop(client->loop, &client->timeout_watcher);

   // Close socket
   if (close(client->io_watcher.fd) != 0) {
      perror("close");
   }

   // Cleanup
   free(client);
}

/// Client IO callback for the LibEV io watcher.
/**
 *  This function recieves data from the clients, and is used as a
 *  callback for a LibEV io watcher.
 *
 *  \param loop    The event loop, that the client is running on.
 *  \param watcher The IO watcher that recieved data.
 *  \param revents Not used.
 */
static void client_io_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
   int bytes;
   char buffer[MAXDATASIZE];
   struct ws_client *client = watcher->data;

   // TODO Kill timeout is a bad idea -- can we reset instead ?
   //kill_timeout(loop, data->timeout);

   // Receive some data
   if ((bytes = recv(watcher->fd, buffer, MAXDATASIZE-1, 0)) < 0) {
      perror("recv");
      // TODO Handle errors better - look up error# etc.
      kill_client(client);
   } else if (bytes == 0) {
      fprintf(stderr, "connection closed by %s\n", client->ip);
      kill_client(client);
   }
   buffer[bytes] = '\0';

   // Print message
   printf("%s\n", buffer);

   // Send hello back
   if (send(watcher->fd, "\n\nHello, world!", 15, 0) == -1)
      perror("send");
   kill_client(client);
}

/// Client Timeout callback for the LibEV timeout watcher
/**
 *  This function handles timeouts of a client, and is used as a
 *  callback for a LibEV timeout watcher.
 *
 *  \param loop    The event loop, that the client is running on.
 *  \param watcher The timeout watcher that timed out.
 *  \param revents Not used.
 */
static void client_timeout_cb(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
   struct ws_client *client = watcher->data;
   printf("timeout: %s\n", client->ip);
   kill_client(client);
}

void ws_client_accept(struct ev_loop *loop, struct ev_io *watcher, int revents)
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

   // Create client
   client = malloc(sizeof(struct ws_client));
   strcpy(client->ip, ip_string);
   client->loop = loop;
   client->timeout_watcher.data = client;
   client->io_watcher.data = client;
   http_parser_init(&(client->parser), HTTP_REQUEST);

   // Start timeout and io watcher
   ev_io_init(&client->io_watcher, client_io_cb, in_fd, EV_READ);
   ev_io_start(loop, &client->io_watcher);
   ev_timer_init(&client->timeout_watcher, client_timeout_cb, TIMEOUT, 0.);
   ev_timer_start(loop, &client->timeout_watcher);
}

