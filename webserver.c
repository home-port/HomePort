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

#include "ws_instance.h"
#include "webserver.h"
#include "ws_client.h"
#include "ws_callbacks.h"

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
/**
 *  This stuct represents a instance of the webserver. The struct
 *  constains both settings for the webserver, which may be changed by
 *  caller, and some internal data values, that are used to run the
 *  webserver.
 *
 *  Making any changes to this struct after a call to ws_start() will
 *  have either no effect or have undefined side-effects (most like
 *  negative).
 *
 *  Use ws_init() to initialise this struct, ws_start() to
 *  start a webserver, and ws_stop() to stop and clean up this instance.
 */
struct ws_instance {
   // User settings
   struct ev_loop *loop;        ///< LibEV loop to start webserver on.
   char *port;                  ///< Port number to start webserver on.
   enum ws_log_level log_level; ///< The log level to use.
   struct ws_callbacks callbacks;
   int (*log_cb)(
         struct ws_instance *instance,
         enum ws_log_level log_level,
         const char *fmt, ...); ///< Callback for logging.
   // Internal data
   int sockfd;                  ///< Socket file descriptor.
   struct ev_io watcher;        ///< LibEV IO Watcher for accepting connects.
   void *clients;               ///< Pointer to first client in list
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
      exit(1);
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

static int default_log_cb(
      struct ws_instance *instance,
      enum ws_log_level log_level,
      const char *fmt, ...)
{
   int status;
   va_list arg;

   if (instance->log_level < log_level) return 0;

   va_start(arg, fmt);
   if (log_level <= WS_LOG_WARN)
      status = vfprintf(stderr, fmt, arg);
   else
      status = vfprintf(stdout, fmt, arg);
   va_end(arg);

   return status;
}

/// Create an instance of a webserver
/**
 * Before starting the webserver, an instance of the ws_instance struct
 * is needed This struct is created using this function. You should only
 * use this function to create the ws_instance struct with.
*/
struct ws_instance *ws_instance_create(
      char *port,
      request_cb header_callback,
      request_cb body_callback,
      struct ev_loop *loop)
{
   struct ws_instance *instance = malloc(sizeof (struct ws_instance));
   if(instance == NULL)
   {
      fprintf(stderr, "ERROR: Cannot allocate memory for a new instance struct\n");
      return NULL;
   }

   instance->port = port;

   instance->callbacks.header_cb = header_callback;
   instance->callbacks.body_cb = body_callback;

   instance->loop = loop;

   // TODO: These should be from a variable parameter list
   instance->log_level = WS_LOG_INFO;
   instance->log_cb = default_log_cb;

   instance->clients = NULL;

   return instance;
}

/// Free an instance of a webserver
/**
 * When the web server should no longer be used, this function
 * should be called to free the allocated memory.
*/
void ws_instance_free(struct ws_instance *instance)
{
   free(instance);
}

/// Start the webserver on a given port.
/**
 *  The libev-based webserver is added to an event loop by a call to
 *  this function. It is the caller's resposibility to start the
 *  event loop, either before or after a call to this.
 *
 *  To stop the webserver again, one may call ws_stop(). See ws_init()
 *  for sample code.
 *
 *  \param instance The webserver instance to start. Initialised with
 *  ws_init();
 */
void ws_start(struct ws_instance *instance)
{
   // Check port
   if (instance->port == NULL) {
      instance->log_cb(instance, WS_LOG_ERROR,
               "No port number given, starting server on 'http'");
      instance->port = "http";
   }

   // Check loop
   if (instance->loop == NULL) {
      instance->log_cb(instance, WS_LOG_ERROR,
            "No event loop given, starting server on 'EV_DEFAULT' \
            loop. Loop will not be started.");
      instance->loop = EV_DEFAULT;
   }

   // Start server
   instance->log_cb(instance, WS_LOG_INFO,
         "Starting server on port '%s'\n", instance->port);
   instance->sockfd = bind_listen(instance->port);
   instance->watcher.data = instance;
   ev_io_init(&instance->watcher, ws_client_accept, instance->sockfd, EV_READ);
   ev_io_start(instance->loop, &instance->watcher);
}

/// Stop an already running webserver.
/**
 *  The webserver, startet with ws_start(), may be stopped by calling
 *  this function. It will take the webserver off the event loop and
 *  clean up after it.
 *
 *  \param instance The webserver instance to stop.
 */
void ws_stop(struct ws_instance *instance)
{
   // Stop accept watcher
   ev_io_stop(instance->loop, &instance->watcher);

   // Kill all clients
   ws_client_killall(instance);

   // Close socket
   if (close(instance->sockfd) != 0) {
      perror("close");
   }
}

struct ws_callbacks *ws_instance_get_callbacks(
      struct ws_instance *instance)
{
   return &instance->callbacks;
}

struct ws_client *ws_instance_get_first_client(
      struct ws_instance *instance)
{
   return instance->clients;
}

void ws_instance_set_first_client(
      struct ws_instance *instance,
      struct ws_client *client)
{
   instance->clients = client;
}
