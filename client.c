// client.c

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

#include "client.h"
#include "webserver.h"
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
   struct ws_instance *instance;         ///< Webserver instance.
   struct ws_client *prev;               ///< Previous client list.
   struct ws_client *next;               ///< Next client in list.
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

   // Remove from list
   if (client->next != NULL)
      client->next->prev = client->prev;
   if (client->prev != NULL) {
      client->prev->next = client->next;
   } else {
      client->instance->clients = client->next;
   }

   // Cleanup
   free(client);
}

void ws_client_killall(struct ws_instance *instance) {
   struct ws_client *next;
   struct ws_client *client = instance->clients;

   while (client != NULL) {
      next = client->next;
      kill_client(client);
      client = next;
   }
}

int message_begin_cb(http_parser *parser)
{
   //struct ws_client *client = parser->data;
  
   return 0;
}

int url_cb(http_parser *parser, const char *buf, size_t len)
{
   return 0;
}

int message_complete_cb(http_parser *parser)
{
   //printf("major nr: '%d' \n", parser->http_major);
   //printf("state: '%c' \n", parser->state);
   //printf("header state: '%c' \n", parser->header_state);
   //printf("type: '%c' \n", parser->type);
   return 0;
}


static http_parser_settings parser_settings = 
{
   .on_message_begin = message_begin_cb,
   .on_url = url_cb,
   .on_message_complete = message_complete_cb    
};

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
   size_t length, nparser;
   char buffer[MAXDATASIZE];
   struct ws_client *client = watcher->data;

   // TODO Kill timeout is a bad idea -- can we reset instead ?
   //kill_timeout(loop, data->timeout);

   // Receive some data
   if ((length = recv(watcher->fd, buffer, MAXDATASIZE-1, 0)) < 0) {
      perror("recv");
      // TODO Handle errors better - look up error# etc.
      kill_client(client);
   } else if (length == 0) {
      fprintf(stderr, "connection closed by %s\n", client->ip);
      kill_client(client);
   }
   //
   nparser = http_parser_execute(&client->parser, &parser_settings, buffer, length);
   if (nparser == length) {
      perror("parse");
      //handle error 
   }
   buffer[length] = '\0';

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
 *  callback for a LibEV timeout watcher
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
   if (client == NULL) {
      fprintf(stderr, "ERROR: Cannot accept client (malloc return NULL)\n");
      return;
   }
   strcpy(client->ip, ip_string);
   client->loop = loop;
   client->timeout_watcher.data = client;
   client->io_watcher.data = client;
   http_parser_init(&(client->parser), HTTP_REQUEST);

   // Set up list
   client->instance = watcher->data;
   client->prev = NULL;
   client->next = client->instance->clients;
   client->instance->clients = client;

   // Start timeout and io watcher
   ev_io_init(&client->io_watcher, client_io_cb, in_fd, EV_READ);
   ev_io_start(loop, &client->io_watcher);
   ev_timer_init(&client->timeout_watcher, client_timeout_cb, TIMEOUT, 0.);
   ev_timer_start(loop, &client->timeout_watcher);
}

