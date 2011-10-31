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
 * @file hpd_events.c
 * @brief  Methods for managing the Event structure
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef EVENTS_H
#define EVENTS_H

#include <pthread.h>
#include "utlist.h"

typedef struct Event Event;
typedef struct EventQueue EventQueue;
typedef struct ServiceId ServiceId;


struct Event
{
	char *id;/**<The ID of the event*/
	char *data;/**<The Data of the event*/
	Event *prev;/**<The previous event in the list*/
	Event *next;/**<The next event in the list*/
};

struct EventQueue
{
	pthread_mutex_t	*mutex;/**<The mutex used to access the EventQueue*/
	pthread_cond_t	*wait_for_event;/**<The condition variable that is used in order to sleep when no Events are received*/
	char *session_id;/**<The ID of the session received and sent using Cookies*/
	char *connection_id;/**<The ID of the connection using the EventQueue*/
	Event *event_queue_head;/**<The head of the actual queue of Events*/
	ServiceId *service_id_head;/**<The list of IDs for which the queue will send Events*/
	int send_log_events;/**<An integer stating if the program needs to send events from the Log to the connection*/
	int exit;/**<An integer used to quit the program*/
	int is_waiting;/**<An integer stating if the EventQueue is in a state of waiting*/
	EventQueue *prev;/**<The previous EventQueue in the list of EventQueues*/
	EventQueue *next;/**<The next EventQueue in the list of EventQueues*/
};

struct ServiceId
{
	char *id;/**<The ID the Service, namely its unique URL*/
	int available;/**<An integer stating if the service is available or not*/
	ServiceId *prev;/**<The previous ServiceId element in the list*/
	ServiceId *next;/**<The next ServiceId element in the list*/
};

Event *create_empty_event();
Event *create_event( char *id, char *data );
Event *copy_event( Event *event );
int destroy_event( Event *event_to_destroy );


EventQueue *create_event_queue();
int destroy_event_queue( EventQueue *event_queue_to_destroy );

int exit_event_queue( EventQueue *event_queue_to_quit );

ServiceId *create_service_id( char *id );
int destroy_service_id( ServiceId *service_id_to_destroy );

int queue_event( EventQueue *event_queue, Event *event, int is_global_queue );
Event *dequeue_event( EventQueue *event_queue , char* connection_id, int is_global_queue);

int is_queue_subscribed_to_service( EventQueue *event_queue, char *URL );
int add_service_id_to_queue( ServiceId* service_id, EventQueue *event_queue );
int remove_service_id_from_queue( char* URL, EventQueue *event_queue );
int set_availability_of_service( char *URL, EventQueue *event_queue, int availability );

int wait_for_timeout( EventQueue *_event_queue );

#endif
