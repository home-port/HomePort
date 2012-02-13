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
 * @file hpd_server_sent_events.h
 * @brief  Methods for managing the Server Sent Events part of the Web Server
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */


#ifndef SERVER_SENT_EVENTS_H
#define SERVER_SENT_EVENTS_H

#include "hpd_web_server_interface.h"
#include "hpd_events.h"

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

int initiate_global_event_queue();

Event *create_a_copy_of_event(Event *event);

int destroy_event(Event *event);

int link_queue_to_connection(	EventQueue *event_queue, 
				char *connection_id);

void print_event_queue(EventQueue *event_queue);

int add_event_to_queue(Event *event, EventQueue *queue);

int remove_event_from_queue(Event *event, EventQueue *queue);

EventQueue *get_queue(struct MHD_Connection *connection);

static void
add_session_cookie( EventQueue *event_queue, 
                   struct MHD_Response *response, 
                   int is_secure_connection);

int send_cookied_xml (	struct MHD_Connection *connection, 
			const char *xmlbuff, 
			EventQueue *event_queue, 
			int is_secure_connection);

char *generate_session_id();

char *generate_connection_id();

void *consume_global_queue();

int send_event_of_value_change (Service *service, char *updated_value, char *IP);

EventQueue *get_event_queue_with_connection_id(char *connection_id);

static ssize_t server_sent_events_callback (	void *cls, 
						uint64_t pos, 
						char *buf, 
						size_t max);

static void server_sent_events_free_callback (void *cls);

int subscribe_to_service(struct MHD_Connection *connection, 
			Service *requested_service, 
			void** con_cls, 
			int is_secure_connection);

int set_up_server_sent_event_connection(struct MHD_Connection *connection, 
					int is_secure_connection);

int free_server_sent_events_ressources();

int subscribe_to_log_events(	struct MHD_Connection *connection, 
				void** con_cls, 
				int is_secure_connection);

int send_log_event(char *log_message);

int notify_service_availability(Service* service_to_notify, int availability);

int service_timed_out(Service* service);



#endif
