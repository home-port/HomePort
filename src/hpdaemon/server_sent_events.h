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


#ifndef SERVER_SENT_EVENTS_H
#define SERVER_SENT_EVENTS_H

#include "web_server.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <uuid/uuid.h>

#define SUBSCRIBE		 	"yes"
#define UNSUBSCRIBE		 	"no"
#define COOKIE_NAME		 	"session"
#define HPD_DONT_SEND_LOG_EVENTS	0
#define HPD_SEND_LOG_EVENTS 		1
#define HPD_IS_SECURE_CONNECTION	2
#define HPD_IS_UNSECURE_CONNECTION	3

enum HPD_EventReceived
{
    HPD_NO_EVENT_RECEIVED = 0,
    HPD_EVENT_RECEIVED_WITH_SERVICE = 1,
    HPD_EVENT_RECEIVED_WITH_VALUE = 2,
    HPD_EVENT_RECEIVED_WITH_LOG = 3,
    HPD_NO_MORE_EVENTS = 4,
    HPD_EVENT_RECEIVED_WITH_UNSUBSCRIPTION = 5
};


typedef struct EventSubscriber EventSubscriber;
struct EventSubscriber
{
    char *session_id;/**<The ID of the session*/
    enum HPD_EventReceived event_received;/**<Integer used when an event is received*/
    int send_log_events;/**<Integer that is used to know if we need to send Log event notifications*/
    int first_log_event;
    char *log_event_message;/**<The Log message to be sent*/
    char *updated_value;/**<The new updated value to be sent*/
    Service *notified_service;/**<The service concerned by the event*/
    ServiceElement *subscribed_service_head;/**<The head of the service list that contains the services that the client subscribed to their events*/
    pthread_mutex_t *mutex; /**<A mutex used to access an Event Subscriber in the list*/
    pthread_cond_t  *condition_var; /**<A condition variable that is used to send events*/
    pthread_cond_t  *finished_sent; 
    EventSubscriber *next;/**<The next EventSubscriber in the list*/
};


EventSubscriber *create_event_subscriber();

int destroy_event_subscriber(EventSubscriber *_event_subscriber);

static ssize_t server_sent_events_callback (void *cls, uint64_t pos, char *buf, size_t max);

static void server_sent_events_free_callback (void *cls);

int send_event (Service *service);

int send_event_of_value_change (Service *service, char *_updated_value);

static void add_session_cookie( EventSubscriber *_event_subscriber, struct MHD_Response *response, int is_secure_connection);

int set_up_server_sent_event_connection(struct MHD_Connection *connection, int is_secure_connection);

int subscribe_to_service(struct MHD_Connection *connection, ServiceElement *_requested_service_element, void** con_cls, int is_secure_connection);

int subscribe_to_log_events(struct MHD_Connection *connection, void** con_cls, int is_secure_connection);

int send_cookied_xml (struct MHD_Connection *connection, const char *xmlbuff, EventSubscriber *client, int is_secure_connection);

char *generate_session_id();

int send_log_event(char *log_message);

int free_condition_variable();

int free_server_sent_events_ressources();

static int send_end_of_session_xml (struct MHD_Connection *connection, const char *xmlbuff);

int unsubscribe_and_notify_service_leaving(Service* _service_to_unsubscribe);



#endif
