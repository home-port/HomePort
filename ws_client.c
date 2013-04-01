// ws_client.c

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

#include "ws_client.h"
#include "ws_instance.h"
#include "webserver.h"
#include "ws_callbacks.h"
#include "ws_http.h"
#include "http-parser/http_parser.h"

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

/// The maximum data size we can recieve or send
#define MAXDATASIZE 1024

/// The amount of time a client may be inactive
#define TIMEOUT  15

// Methods for http_parser settings
static int parser_url_cb(http_parser *parser, const char *buf, size_t len);
static int parser_headers_complete_cb(http_parser *parser);
static int parser_body_cb(http_parser *parser, const char *buf, size_t len);

/// Global settings for http_parser
static http_parser_settings parser_settings = 
{
   .on_url = parser_url_cb,
   .on_headers_complete = parser_headers_complete_cb,
   .on_body = parser_body_cb
};

/// All data to represent a client
struct ws_client {
   char ip[INET6_ADDRSTRLEN];            ///< IP address of client.
   struct ev_loop *loop;                 ///< The event loop.
   struct ev_timer timeout_watcher;      ///< LibEV watcher for timeout.
   struct ev_io recv_watcher;            ///< Watcher for recieving data.
   struct ev_io send_watcher;            ///< Watcher for sending data.
   char send_msg[MAXDATASIZE];           ///< Data to send.
   struct ws_request *request;
   struct ws_instance *instance;         ///< Webserver instance.
   struct ws_callbacks *callbacks;
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

static int parser_headers_complete_cb(http_parser *parser)
{
   struct ws_request *request = parser->data;
   struct ws_client *client = ws_request_get_client(request);
   struct ws_response *response;

   ws_request_set_method(request);

   response = client->callbacks->header_cb(request);
   if(response == NULL) {
      return 0;
   } else {
      ws_client_send(client, "%s", ws_response_str(response));
      ws_response_destroy(response);
      return 1;
   }
}

static int parser_url_cb(http_parser *parser, const char *buf, size_t len)
{
   struct ws_request *request = parser->data;

   ws_request_cat_url(request, buf, len);

   return 0;
}

static int parser_body_cb(http_parser *parser, const char *buf, size_t len)
{
   struct ws_request *request = parser->data;
   struct ws_client *client = ws_request_get_client(request);
   struct ws_response *response;

   if (ws_request_cat_body(request, buf, len) != 0) {
      return 1;
   }
   
   if(http_body_is_final(parser))
   {
      response = client->callbacks->body_cb(request);

      if(response == NULL)
      {
         return 1;
      }

      ws_client_send(client, "%s", ws_response_str(response));
      ws_response_destroy(response);
   }

   return 0;
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

/// Client IO callback for the LibEV io watcher.
/**
 *  This function recieves data from the clients, and is used as a
 *  callback for a LibEV io watcher.
 *
 *  \param loop    The event loop, that the client is running on.
 *  \param watcher The IO watcher that recieved data.
 *  \param revents Not used.
 */
static void client_recv_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
   size_t recieved, parsed;
   char buffer[MAXDATASIZE];
   struct ws_client *client = watcher->data;

   printf("recieving data from %s\n", client->ip);

   // Receive some data
   if ((recieved = recv(watcher->fd, buffer, MAXDATASIZE-1, 0)) < 0) {
      if (recieved == -1 && errno == EWOULDBLOCK) {
         fprintf(stderr, "libev callbacked called without data to recieve (client: %s)", client->ip);
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

   if (client->request == NULL) {
      client->request = ws_request_create(client);
   }

   // Parse recieved data
   parsed = ws_request_parse(client->request, &parser_settings, buffer, recieved);
   if (parsed != recieved) {
      perror("parse");
      ws_client_kill(client);
      return;
   }

   // Reset timeout
   client->timeout_watcher.repeat = TIMEOUT;
   ev_timer_again(loop, &client->timeout_watcher);
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

   // Create client and parser
   client = malloc(sizeof(struct ws_client));
   if (client == NULL) {
      fprintf(stderr, "ERROR: Cannot accept client (malloc return NULL)\n");
      return;
   }
   client->instance = watcher->data;
   client->callbacks = ws_instance_get_callbacks(client->instance); 
   strcpy(client->ip, ip_string);
   client->loop = loop;
   client->timeout_watcher.data = client;
   client->recv_watcher.data = client;
   client->send_watcher.data = client;
   client->request = NULL;

   // Set up list
   client->prev = NULL;
   client->next = ws_instance_get_first_client(client->instance);
   if (client->next != NULL)
      client->next->prev = client;
   ws_instance_set_first_client(client->instance, client);

   // Start timeout and io watcher
   ev_io_init(&client->recv_watcher, client_recv_cb, in_fd, EV_READ);
   ev_io_init(&client->send_watcher, client_send_cb, in_fd, EV_WRITE);
   ev_io_start(loop, &client->recv_watcher);
   ev_init(&client->timeout_watcher, client_timeout_cb);
   client->timeout_watcher.repeat = TIMEOUT;
   ev_timer_again(loop, &client->timeout_watcher);
}

void ws_client_send(struct ws_client *client, char *fmt, ...) {
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

/// Kill and clean up after a client
/**
 *  This function stops the LibEV watchers, closes the socket, and frees
 *  the data structures used be a client.
 *
 *  \param client The client to kill.
 */
void ws_client_kill(struct ws_client *client) {
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
   if (client->next != NULL)
      client->next->prev = client->prev;
   if (client->prev != NULL) {
      client->prev->next = client->next;
   } else {
      ws_instance_set_first_client(client->instance, client->next);
   }

   // Cleanup
   free(client);
}

/// Kill all clients in a runnning webserver instance
/**
 *  Designed to be used within ws_stop, but can also be used for other
 *  purposes, where the desidered effect is to remove all clients from
 *  the event loop and close their sockets.
 *
 *  \param instance The webserver instance.
 */
void ws_client_killall(struct ws_instance *instance) {
   struct ws_client *next;
   struct ws_client *client = ws_instance_get_first_client(instance);

   while (client != NULL) {
      next = client->next;
      ws_client_kill(client);
      client = next;
   }
}


