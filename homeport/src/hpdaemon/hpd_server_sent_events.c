/*Copyright 2011 Aalborg University. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed*/

/**
 * @file hpd_server_sent_events.c
 * @brief  Methods for managing the Server Sent Events part of the Web Server
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include 	"hpd_server_sent_events.h"
#include	"hpd_error.h"

// TODO FIND A BETTER LOCATION FOR THIS
#define TIMEOUT 15

static struct event_socket *sockets = NULL;

void destroy_socket(struct event_socket *socket)
{
   // TODO Is this done ?
   ev_timer_stop(socket->loop, &socket->timeout_watcher);
   close_event_socket(socket);
   free(socket->url);
   free(socket);
}

static void timeout_cb(struct ev_loop *loop, struct ev_timer *watcher, int revents)
{
   struct event_socket *socket = watcher->data;
   printf("Socket timeout - closing it\n");
   unregister_socket(socket);
   destroy_socket(socket);
}

struct event_socket *subscribe_to_events(const char *body, struct
      ev_loop *loop)
{ 
   struct event_socket *socket = malloc(sizeof(struct event_socket));
   socket->url = NULL;
   socket->req = NULL;
   socket->next = NULL;
   socket->prev = NULL;

   // Generate url
	socket->url = malloc((8+36+1) * sizeof(char));
   strcpy(socket->url, "/events/");
   uuid_t uuid;
	uuid_generate(uuid);
	uuid_unparse(uuid, &socket->url[8]);
	uuid_clear(uuid);
   
   // Add timeout on socket
   socket->loop = loop;
   ev_init(&socket->timeout_watcher, timeout_cb);
   socket->timeout_watcher.repeat = TIMEOUT;
   ev_timer_again(loop, &socket->timeout_watcher);
   socket->timeout_watcher.data = socket;

   return socket;
}

void open_event_socket(struct event_socket *socket,
                       void *req)
{
   ev_timer_stop(socket->loop, &socket->timeout_watcher);
   socket->req = req;
   if (sockets) sockets->prev = socket;
   socket->next = sockets;
   sockets = socket;
}

void close_event_socket(struct event_socket *socket)
{
   socket->req = NULL;

   if (sockets == socket) sockets = socket->next;
   if (socket->next) socket->next->prev = socket->prev;
   if (socket->prev) socket->prev->next = socket->next;

   socket->next = NULL;
   socket->prev = NULL;
}

/**
 * Notifies the subscribed client of the (un)availability of a Service
	 *
 * @param service_to_notify The service concerned
 *
 * @param availability The availability of the service (HPD_YES or HPD_NO)
 *
 * @return HPD_E_SERVICE_IS_NULL if the service is NULL, HPD_E_SUCCESS if successful
 */
int 
notify_service_availability(Service* service_to_notify, int availability)
{
   struct event_socket *s;

	if(!service_to_notify)
		return HPD_E_SERVICE_IS_NULL;

   for (s = sockets; s != NULL; s = s->next) {
      send_event(s, "event: %s\ndata: %s\ndata: %s\nid: %s\n\n",
            "service_availability",
            (availability == HPD_YES) ? "available" : "unavailable",
            "",
            service_to_notify->value_url);
   }

	return HPD_E_SUCCESS;	
}

/**
 * Sends an event of a Service value that changed
 *
 * @param service The Service of which the value changed
 *
 * @param value The updated value of the Service
 *
 * @return HPD_E_SERVICE_IS_NULL, HPD_E_EVENT_IS_NULL or HPD_E_ERROR_QUEUING_EVENT if an error occured, HPD_E_SUCCESS if successful
 */
int 
send_event_of_value_change( Service *service, const char *value )
{
   struct event_socket *s;

	if( !service )
		return HPD_E_SERVICE_IS_NULL;

	if( !service->value_url )
		return HPD_E_SERVICE_IS_NULL;

   for (s = sockets; s != NULL; s = s->next) {
      send_event(s, "event: %s\ndata: %s\nid: %s\n\n",
            "value_change",
            value,
            service->value_url);
   }

	return HPD_E_SUCCESS;
}

/**
 * Sends an event of Log
 *
 * @param log_data The Log data to send
 *
 * @return HPD_E_LOG_DATA_IS_NULL, HPD_E_EVENT_IS_NULL or HPD_E_ERROR_QUEUING_EVENT if an error occured, HPD_E_SUCCESS if successful
 */
int 
send_log_event( char *log_data)
{
   struct event_socket *s;

	if( !log_data )
		return HPD_E_LOG_DATA_IS_NULL;

   for (s = sockets; s != NULL; s = s->next) {
      send_event(s, "event: %s\ndata: %s\ndata: %s\nid: %s\n\n",
            "log",
            log_data,
            NULL,
            "/log");
   }

	return HPD_E_SUCCESS;
}


