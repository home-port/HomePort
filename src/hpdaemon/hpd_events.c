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

#include "hpd_events.h"
#include "hpd_error.h"

/**
 * Creates an empty Event structure
 *
 * @return An empty Event structure
 */
Event *
create_empty_event()
{
	Event *newEvent = (Event*)malloc(sizeof(Event));
	if( !newEvent )
		return NULL;
	newEvent->id = NULL;
	newEvent->data = NULL;
	newEvent->prev = NULL;
	newEvent->next = NULL;

	return newEvent;
}

/**
 * Creates an Event structure
 *
 * @param id The id of the event (The URL of the service or "Log")
 *
 * @param data The data of the event
 *
 * @return The corresponding Event structure
 */
Event *
create_event( char *id, char *data )
{
	Event *newEvent;

	if( !id || !data )
		return NULL;

	newEvent = ( Event* ) malloc ( sizeof( Event ) );
	if( !newEvent )
		return NULL;
	newEvent->id = ( char* )malloc( ( strlen( id ) + 1 ) * sizeof( char ) );
	newEvent->id = strcpy( newEvent->id, id );

	newEvent->data = ( char* ) malloc( ( strlen ( data ) + 1 ) * sizeof( char ) );
	newEvent->data = strcpy( newEvent->data, data );
	newEvent->prev = NULL;
	newEvent->next = NULL;

	return newEvent;
}

/**
 * Creates an copy of an Event structure
 *
 * @param event The event that we are willing to copy
 *
 * @return The copy of the Event structure
 */
Event *
copy_event( Event *event )
{
	Event *newEvent;

	if( !event )
		return NULL;

	newEvent = (Event*)malloc(sizeof(Event));

	if( event->id )
	{
		newEvent->id = (char*)malloc((strlen(event->id)+1)*sizeof(char));
		strcpy( newEvent->id, event->id );
	}

	if( event->data )
	{
		newEvent->data = (char*)malloc((strlen(event->data)+1)*sizeof(char));
		strcpy( newEvent->data, event->data );
	}

	newEvent->prev = NULL;
	newEvent->next = NULL;

	return newEvent;
}

/**
 * Destroys an Event structure
 *
 * @param event_to_destroy The Event that we are willing to destroy
 *
 * @return HPD_E_EVENT_IS_NULL if the Event is NULL, HPD_E_SUCCESS if successful
 */
int 
destroy_event( Event *event_to_destroy )
{
	if( !event_to_destroy )
		return HPD_E_EVENT_IS_NULL;

	if( event_to_destroy->id )
		free(event_to_destroy->id);

	if( event_to_destroy->data )
		free(event_to_destroy->data);

	free( event_to_destroy );

	return HPD_E_SUCCESS;
}

/**
 * Creates an EventQueue structure
 *
 * @return An EventQueue structure
 */
EventQueue *
create_event_queue()
{
	EventQueue *new_event_queue = (EventQueue*)malloc(sizeof(EventQueue));

	new_event_queue->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));

	pthread_mutex_init( new_event_queue->mutex, NULL );

	new_event_queue->wait_for_event = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));

	pthread_cond_init( new_event_queue->wait_for_event, NULL );

	new_event_queue->session_id = generate_session_id();
	new_event_queue->connection_id = NULL;
	new_event_queue->event_queue_head = NULL;
	new_event_queue->service_id_head = NULL;
	new_event_queue->send_log_events = HPD_NO;
	new_event_queue->is_waiting = HPD_NO;
	new_event_queue->exit = HPD_NO;
	new_event_queue->prev = NULL;
	new_event_queue->next = NULL;

	return new_event_queue;
}

/**
 * Destroys an EventQueue structure
 *
 * @param event_queue_to_destroy The EventQueue that we are willing to destroy
 *
 * @return HPD_E_EVENT_QUEUE_IS_NULL if the EventQueue is NULL, HPD_E_SUCCESS if successful
 */
int 
destroy_event_queue( EventQueue *event_queue_to_destroy )
{
	if ( !event_queue_to_destroy )
		return HPD_E_EVENT_QUEUE_IS_NULL;

	if ( event_queue_to_destroy->mutex )
	{
		pthread_mutex_destroy( event_queue_to_destroy->mutex );
		free( event_queue_to_destroy->mutex );
	}

	if ( event_queue_to_destroy->wait_for_event )
	{
		pthread_cond_destroy( event_queue_to_destroy->wait_for_event );
		free( event_queue_to_destroy->wait_for_event );
	}

	if ( event_queue_to_destroy->session_id )
		free( event_queue_to_destroy->session_id );

	if ( event_queue_to_destroy->event_queue_head )
	{
		Event *iterator, *tmp;
		DL_FOREACH_SAFE( event_queue_to_destroy->event_queue_head, iterator, tmp )
		{
			DL_DELETE( event_queue_to_destroy->event_queue_head, iterator );
			free( iterator );
		}
	}

	if( event_queue_to_destroy->service_id_head )
	{
		ServiceId *iterator, *tmp;
		DL_FOREACH_SAFE( event_queue_to_destroy->service_id_head, iterator, tmp )
		{
			DL_DELETE( event_queue_to_destroy->service_id_head, iterator );
			free( iterator );
		}
	}

	free( event_queue_to_destroy );

	return HPD_E_SUCCESS;
}

/**
 * States that an EventQueue enters a state of Exit
 *
 * @param event_queue_to_exit The EventQueue that we are willing to change the state of
 *
 * @return HPD_E_EVENT_QUEUE_IS_NULL if the EventQueue is NULL, HPD_E_SUCCESS if successful
 */
int 
exit_event_queue( EventQueue *event_queue_to_exit )
{
	if( !event_queue_to_exit )
		return HPD_E_EVENT_QUEUE_IS_NULL;

	pthread_mutex_lock( event_queue_to_exit->mutex );
	event_queue_to_exit->exit = HPD_YES;

	if( event_queue_to_exit -> is_waiting == HPD_YES )
		pthread_cond_broadcast( event_queue_to_exit->wait_for_event );

	pthread_mutex_unlock( event_queue_to_exit->mutex );

	return HPD_E_SUCCESS;
}

/**
 * Creates a ServiceId structure
 *
 * @param id The ID (value URL) of the Service
	 *
 * @return NULL if the ID is NULL, a ServiceId structure
 */
ServiceId *
create_service_id( char *id )
{
	ServiceId *new_service_id;

	if( !id )
		return NULL;

	new_service_id = (ServiceId*)malloc(sizeof(ServiceId));

	new_service_id->id = (char*)malloc((strlen(id)+1)*sizeof(char));
	strcpy( new_service_id->id, id );

	new_service_id-> available = HPD_YES;
	new_service_id->prev = NULL;
	new_service_id->next = NULL;

	return new_service_id;
}

/**
 * Destroys a ServiceId structure
 *
 * @param service_id_to_destroy The ServiceId that we are willing to destroy
 *
 * @return HPD_E_SERVICE_ID_IS_NULL if the ServiceId is NULL, HPD_E_SUCCESS if successful
 */
int 
destroy_service_id( ServiceId *service_id_to_destroy )
{
	if( !service_id_to_destroy )
		return HPD_E_SERVICE_ID_IS_NULL;

	if( service_id_to_destroy->id )
		free( service_id_to_destroy->id );

	free( service_id_to_destroy );

	return HPD_E_SUCCESS;
}

/**
 * Queues the Event to an EventQueue
 *
 * @param event_queue The EventQueue in which the Event needs to be queued
 *
 * @param event The Event that needs to be queued
 *
 * @param is_global_queue An integer stating if the EventQueue is the global queue of the system (HPD_YES or HPD_NO)
 *
 * @return HPD_E_EVENT_QUEUE_OR_EVENT_IS_NULL if the EventQueue or the Event is NULL, HPD_E_SUCCESS if successful
 */
int 
queue_event( EventQueue *event_queue, Event *event, int is_global_queue )
{
	ServiceId *iterator = NULL;
	int found = HPD_NO;

	if( !event_queue || !event )
		return HPD_E_EVENT_QUEUE_OR_EVENT_IS_NULL;

	int is_log = strcmp(event->id, "Log");
	pthread_mutex_lock( event_queue->mutex );

	if( (  is_log == 0 ) && ( event_queue->send_log_events == HPD_NO ) )
	{
		pthread_mutex_unlock( event_queue->mutex );
		return HPD_E_SUCCESS;
	}
	else if( HPD_NO == is_global_queue && ( is_log != 0 ) )
	{
		DL_FOREACH( event_queue->service_id_head, iterator )
		{
			if( strcmp( iterator->id, event->id ) == 0 )
			{
				found = HPD_YES;
				break;
			}
		}

		if( found == HPD_NO )
		{
			pthread_mutex_unlock( event_queue->mutex );
			return HPD_E_SUCCESS;
		}
	}

	if( HPD_NO == is_global_queue )
		event = copy_event( event );
	DL_APPEND( event_queue->event_queue_head, event );
	if( event_queue->is_waiting == HPD_YES &&  is_global_queue == HPD_YES )
		pthread_cond_broadcast( event_queue->wait_for_event );
	else if( event_queue->is_waiting == HPD_YES &&  is_global_queue == HPD_NO && event_queue-> connection_id )
	{
		pthread_cond_broadcast( event_queue->wait_for_event );
	}

	pthread_mutex_unlock( event_queue->mutex );

	return HPD_E_SUCCESS;
}

/**
 * Dequeues the Event from an EventQueue
 *
 * @param event_queue The EventQueue from which the Event needs to be dequeued
 *
 * @param connection_id The connection ID on which we perform the dequeuing
 *
 * @param is_global_queue An integer stating if the EventQueue is the global queue of the system (HPD_YES or HPD_NO)
 *
 * @return NULL if the EventQueue is NULL or if the connection id is different from the connection id of the EventQueue or if the EventQueue is in an exit state, the Event to dequeue if successful
 */
Event *
dequeue_event( EventQueue *event_queue , char* connection_id, int is_global_queue )
{
	Event *event_to_dequeue;

	if( !event_queue )
		return NULL;


	pthread_mutex_lock( event_queue->mutex );

	/*If the queue is empty but still has a connection*/
	if( event_queue->event_queue_head == NULL )
	{
		event_queue->is_waiting = HPD_YES;
		pthread_cond_wait( event_queue->wait_for_event, event_queue->mutex );
		event_queue->is_waiting = HPD_NO;
	}

	/*If another connection has taken the queue*/
	if( connection_id && is_global_queue == HPD_NO )
	{
		if( 0 != strcmp(event_queue-> connection_id, connection_id) )
		{
			pthread_mutex_unlock( event_queue-> mutex );
			return NULL;
		}
	}

	/*If the program is exiting*/
	if( event_queue->exit == HPD_YES )
	{
		pthread_mutex_unlock( event_queue-> mutex );
		return NULL;
	}

	event_to_dequeue = event_queue->event_queue_head;

	DL_DELETE( event_queue->event_queue_head, event_queue->event_queue_head );

	pthread_mutex_unlock( event_queue->mutex );

	return event_to_dequeue;
}

/**
 * Method used to wait for the timeout of the EventQueue whenever the client disconnected and didn't come back.
 *
 * @param event_queue The EventQueue that will wait for timeout
 *
 * @return HPD_YES if the timeout has been reached, HPD_NO if a new client connected for that EventQueue
 */
int 
wait_for_timeout( EventQueue *event_queue )
{
	int               rc;
	struct timespec   ts;
	struct timeval    tp;

	rc =  gettimeofday( &tp, NULL );
	ts.tv_sec  = tp.tv_sec;
	ts.tv_nsec = tp.tv_usec * 1000;
	ts.tv_sec += 5;

	pthread_mutex_lock( event_queue->mutex );
	event_queue-> is_waiting = HPD_WAIT_FOR_TIMEOUT;
	rc = pthread_cond_timedwait( event_queue->wait_for_event, event_queue->mutex, &ts );
	event_queue-> is_waiting = HPD_NO;

	if(rc == ETIMEDOUT)
	{
		pthread_mutex_unlock( event_queue-> mutex );
		return HPD_YES;
	}

	pthread_mutex_unlock( event_queue->mutex );
	return HPD_NO;
}

/**
 * Method used to know if an EventQueue is subscribed to a given Service
 *
 * @param event_queue The EventQueue that we want to check
 *
 * @param URL The URL of the Service that we want to check if the EventQueue is subscribed to
 *
 * @return HPD_E_EVENT_QUEUE_OR_URL_IS_NULL if the EventQueue or the URL is NULL, HPD_YES if the EventQueue is subscribed to the Service, HPD_NO if not
 */
int 
is_queue_subscribed_to_service( EventQueue *event_queue, char *URL )
{
	ServiceId *iterator = NULL;

	if( !event_queue || !URL)
		return HPD_E_EVENT_QUEUE_OR_URL_IS_NULL;

	pthread_mutex_lock( event_queue-> mutex );

	DL_FOREACH( event_queue-> service_id_head, iterator )
	{
		if( 0 == strcmp( URL, iterator-> id ) )
		{
			pthread_mutex_unlock( event_queue-> mutex );
			return HPD_YES;
		}
	}
	pthread_mutex_unlock( event_queue-> mutex );
	return HPD_NO;
}

/**
 * Adds a ServiceId to an EventQueue
 *
 * @param event_queue The EventQueue 
 *
 * @param service_id The ServiceId
 *
 * @return HPD_E_EVENT_QUEUE_OR_SERVICE_ID_IS_NULL if the EventQueue or the ServiceId is NULL, HPD_E_SUCCESS if successful
 */
int 
add_service_id_to_queue( ServiceId* service_id, EventQueue *event_queue )
{
	if( !event_queue || !service_id)
		return HPD_E_EVENT_QUEUE_OR_SERVICE_ID_IS_NULL;

	pthread_mutex_lock( event_queue-> mutex );
	DL_APPEND( event_queue-> service_id_head, service_id );
	pthread_mutex_unlock( event_queue-> mutex );
	return HPD_E_SUCCESS;
}

/**
 * Removes a ServiceId from an EventQueue
 *
 * @param URL The URL of the ServiceId that we want to remove
 *
 * @param event_queue The EventQueue from which we want to remove the ServiceId
 *
 * @return HPD_E_EVENT_QUEUE_OR_URL_IS_NULL if the EventQueue or the URL is NULL, HPD_E_SERVICE_ID_NOT_IN_EVENT_QUEUE if the ServiceId is not in the EventQueue HPD_E_SUCCESS if successful
 */
int 
remove_service_id_from_queue( char *URL, EventQueue *event_queue )
{
	if( !event_queue || !URL)
		return HPD_E_EVENT_QUEUE_OR_URL_IS_NULL;

	ServiceId *iterator, *temp;

	pthread_mutex_lock( event_queue-> mutex );
	DL_FOREACH_SAFE( event_queue-> service_id_head, iterator, temp )
	{
		if( 0 == strcmp( iterator->id, URL ) )
		{
			DL_DELETE( event_queue-> service_id_head, iterator );
			destroy_service_id( iterator );
			pthread_mutex_unlock( event_queue-> mutex );
			return HPD_E_SUCCESS;
		}
	}
	pthread_mutex_unlock( event_queue-> mutex );
	return HPD_E_SERVICE_ID_NOT_IN_EVENT_QUEUE;
}

/**
 * Sets the availability of the Service
 *
 * @param URL The URL of the Service
 *
 * @param event_queue The EventQueue from which we want to set the availability of one of its Services
 *
 * @param availability An integer used to check the availability of the Service
 *
 * @return HPD_E_EVENT_QUEUE_OR_URL_IS_NULL if the EventQueue or the URL is NULL, HPD_E_NOT_HPD_BOOLEAN if the availability is not HPD_YES or HPD_NO,  HPD_YES if successful HPD_NO if not
 */
int 
set_availability_of_service( char *URL, EventQueue *event_queue, int availability )
{
	ServiceId *iterator;

	if( !event_queue || !URL )
	{
		printf("event_queue or URL NULL\n");
		return HPD_E_EVENT_QUEUE_OR_URL_IS_NULL;
	}

	if( availability != HPD_YES && availability != HPD_NO )
	{
		printf("Needs to be HPD_YES/NO\n");
		return HPD_E_NOT_HPD_BOOLEAN;
	}

	pthread_mutex_lock( event_queue-> mutex );

	DL_FOREACH( event_queue-> service_id_head, iterator )
	{
		if( 0 == strcmp( URL, iterator-> id ) )
		{
			iterator-> available = availability;
			pthread_mutex_unlock( event_queue-> mutex );
			return HPD_YES;
		}
	}
	pthread_mutex_unlock( event_queue-> mutex );
	return HPD_NO;
}
