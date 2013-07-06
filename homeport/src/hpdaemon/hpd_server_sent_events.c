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

EventQueue *global_event_queue = NULL; /**<The Global EventQueue*/
EventQueue *event_queue_list_head;/**<The List of EventQueues of the system*/
pthread_mutex_t queue_list_mutex = PTHREAD_MUTEX_INITIALIZER;/**<The mutex to access the List of EventQueues*/
pthread_t global_event_thread;/**<The Global Event Thread that will consume the Global EventQueue*/

void *consume_global_event_queue();

/**
 * Initiates the Global EventQueue
 *
 * @return HPD_E_EVENT_QUEUE_IS_NULL if the creation of the Global EventQueue failed, HPD_E_SUCCESS if the initiation was successful
 */
int 
initiate_global_event_queue()
{

	global_event_queue = create_event_queue();
	if( !global_event_queue )
	{
		printf("Error creating the global event queue\n");
		return HPD_E_EVENT_QUEUE_IS_NULL;
	}

	global_event_queue->send_log_events = HPD_YES;

	pthread_create( &global_event_thread, NULL, consume_global_event_queue, NULL );

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
send_event_of_value_change( Service *service, const char *value , const char *IP )
{
	Event *new_event;
	if( !service )
		return HPD_E_SERVICE_IS_NULL;

	if( !service->value_url )
		return HPD_E_SERVICE_IS_NULL;

	new_event = create_event( "value_change", service->value_url, IP, value );
	if ( !new_event )
		return HPD_E_EVENT_IS_NULL;

	if( queue_event( global_event_queue, new_event, HPD_YES ) )
	{
		printf("Error while queueing new event\n");
		return HPD_E_ERROR_QUEUING_EVENT;
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
	Event *new_event;
	if( !log_data )
		return HPD_E_LOG_DATA_IS_NULL;

	new_event = create_event( "log" ,"/log", NULL, log_data );
	if ( !new_event )
		return HPD_E_EVENT_IS_NULL;

	if( queue_event( global_event_queue, new_event, HPD_YES ) )
	{
		printf("Error while queueing new event\n");
		return HPD_E_ERROR_QUEUING_EVENT;
	}

	return HPD_E_SUCCESS;
}

/**
 * The callback function that consumes the global queue
 *
 * @return void
 */
void *
consume_global_event_queue()
{
	Event *new_event = NULL;
	EventQueue *iterator = NULL;

	while(1)
	{
		new_event = dequeue_event( global_event_queue , NULL, HPD_YES );
		if( !new_event )
		{
			printf("Error dequeing event, EXIT\n");
			destroy_event_queue( global_event_queue );
			return;
		}

		pthread_mutex_lock(&queue_list_mutex);
		DL_FOREACH( event_queue_list_head, iterator )
		{
			if( queue_event( iterator, new_event, HPD_NO ) != 0 )
			{
				printf("Error queueing event\n");
				pthread_mutex_unlock(&queue_list_mutex);
				return;
			}
		}
		pthread_mutex_unlock(&queue_list_mutex);

		destroy_event(new_event);
	}
}


/**
 * Adds a Cookie Header corresponding to the session to the response
 *
 * @param event_queue The EventQueue that possess the session ID
 *
 * @param response The response to add the header to
 *
 * @param is_secure_connection Variable that states if the connection is secured or not ( HPD_IS_SECURE_CONNECTION or HPD_IS_SECURE_CONNECTION ) 
 *
 * @return void
 */
static void
add_session_cookie( EventQueue *event_queue, 
                   struct MHD_Response *response, 
                   int is_secure_connection)
{
	char cstr[256];

	if(event_queue == NULL)
	{
		return; //TO BE CHANGED
	}

	pthread_mutex_lock(event_queue-> mutex);
	switch(is_secure_connection)
	{
		case HPD_IS_SECURE_CONNECTION :

			snprintf(cstr, 
			         sizeof(cstr), 
			         "%s=%s; path=/; secure", 
			         COOKIE_NAME, 
			         event_queue-> session_id);
			break;

		case HPD_IS_UNSECURE_CONNECTION :

			snprintf(cstr, 
			         sizeof(cstr), 
			         "%s=%s; path=/", 
			         COOKIE_NAME, 
			         event_queue-> session_id);
			break;
		default :
			break;
	}
	pthread_mutex_unlock(event_queue->mutex);
   // TODO Needs change to new webserver
	if(MHD_NO == MHD_add_response_header (response, 
	                                      MHD_HTTP_HEADER_SET_COOKIE, 
	                                      cstr))
		printf("Failed to add cookie header\n");

}

/**
 * Add an XML response to the queue of the server with cookie
 *
 * @param connection The connection
 *
 * @param xmlbuff The XML response to be sent
 *
 * @param event_queue The EventQueue linked to the connection
 *
 * @param is_secure_connection Variable that states if the connection is secured or not ( HPD_IS_SECURE_CONNECTION or HPD_IS_SECURE_CONNECTION )
 *
 * @return MHD return value, MHD_NO if the response failed to be created, 
 *		   return code of MHD_queue_response otherwise
 */
int 
send_cookied_xml (	struct MHD_Connection *connection, 
                  char *xmlbuff, 
                  EventQueue *event_queue, 
                  int is_secure_connection)
{
	int ret;
	struct MHD_Response *response;

   // TODO Needs change to new webserver
	response = MHD_create_response_from_buffer (strlen(xmlbuff),
	                                            xmlbuff, 
	                                            MHD_RESPMEM_MUST_FREE);

	if(!response)
	{
		if(xmlbuff)
			free(xmlbuff);
		return MHD_NO;
	}	

	add_session_cookie(event_queue, response, is_secure_connection);
   // TODO Needs change to new webserver
	MHD_add_response_header (response, "Content-Type", "text/xml");
	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

/**
 * Generates a Universal Unique ID for the Session ID
 *
 * @return The session ID
 */
char *
generate_session_id()
{
	uuid_t uuid;
	char *uuid_str = (char*)malloc (37 * sizeof (char));
	uuid_generate (uuid);
	uuid_unparse(uuid, uuid_str);
	uuid_clear (uuid);

	return uuid_str;
}

/**
 * Generates a Universal Unique ID for the Connection ID
 *
 * @return The session ID
 */
char *generate_connection_id()
{
	uuid_t uuid;
	char *uuid_str = (char*)malloc (37 * sizeof (char));
	uuid_generate (uuid);
	uuid_unparse(uuid, uuid_str);
	uuid_clear (uuid);

	return uuid_str;
}

/**
 * Method to get the EventQueue linked to a given connection ID
 *
 * @param connection_id The corresponding connection ID
 *
 * @return NULL if an error occured, the EventQueue if successful
 */
EventQueue *
get_event_queue_with_connection_id( char *connection_id )
{
	EventQueue *iterator;

	if( !connection_id )
		return NULL;

	pthread_mutex_lock( &queue_list_mutex );

	DL_FOREACH( event_queue_list_head, iterator )
	{
		if( strcmp( iterator->connection_id, connection_id ) == 0 )
		{
			pthread_mutex_unlock( &queue_list_mutex );
			return iterator;
		}
	}

	pthread_mutex_unlock( &queue_list_mutex );

	return NULL;
}


/**
 * Callback of the server-sent Events
 *
 * @param cls Generic pointer that is used to pass a connection ID
 *
 * @param pos The position in the datastream to access
 *
 * @param buf The response buffer
 *
 * @param max The maximum size of the buffer
 *
 * @return -1 if a problem occured, the size of the buffer if successful
 */
static ssize_t 
server_sent_events_callback (	void *cls, 
                             uint64_t pos, 
                             char *buf, 
                             size_t max)
{

	char *connection_id = cls;

	if(!connection_id)
		return -1;

	EventQueue *event_queue = get_event_queue_with_connection_id(connection_id);

	if(!event_queue)
	{
		printf("Failed to retrieve the EventQueue\n");
		return -1;
	}

	Event *new_event = dequeue_event( event_queue , connection_id , HPD_NO);

	if( !new_event )
	{
		printf("Error dequeing\n");
		if(event_queue-> exit ==1)
			destroy_event_queue( event_queue ); 
		return -1;
	}

	if( !new_event->id || !new_event->data || !new_event->event_name)
	{
		printf("Event error\n");
		return -1;
	}

	sprintf(buf,"event: %s\ndata: %s\ndata: %s\nid: %s\n\n",new_event->event_name, new_event->data, (new_event->IP != NULL) ? new_event->IP : "", new_event->id);

	destroy_event(new_event);

	return strlen(buf);
}

/**
 * Callback to free resources linked to the connection
 *
 * @param cls Generic pointer that is used to pass a connection ID
 *
 * @return void
 */
static void 
server_sent_events_free_callback (void *cls)
{
	char *connection_id = cls;
	if(connection_id)
	{
		EventQueue *event_queue = get_event_queue_with_connection_id(connection_id);
		if(event_queue)
		{	
			if(HPD_YES == wait_for_timeout(event_queue))
			{
				pthread_mutex_lock(&queue_list_mutex);
				DL_DELETE(event_queue_list_head, event_queue);
				pthread_mutex_unlock(&queue_list_mutex);

				destroy_event_queue(event_queue);
			}
		}
		free(connection_id);
	}
}

/**
 * Subscribes to service Events
 *
 * @param connection The connection that is used
 *
 * @param requested_service The concerned Service
 *
 * @param con_cls A generic pointer that is used to store the xml in order to be able to free it
 *
 * @param is_secure_connection Variable that states if the connection is secured or not ( HPD_IS_SECURE_CONNECTION or HPD_IS_SECURE_CONNECTION )
 *
 * @return MHD_YES if successful, MHD_NO if failed
 */
int 
subscribe_to_service(struct MHD_Connection *connection, Service *requested_service, void **con_cls, int is_secure_connection)
{
	char *xmlbuff;
	ServiceId *new_service_id;
	EventQueue *event_queue = get_queue(connection);



	/*Is there a queue for this cookie ?*/
	if(!event_queue)
	{
		event_queue = create_event_queue();
		pthread_mutex_lock(&queue_list_mutex);
		DL_APPEND(event_queue_list_head, event_queue);
		pthread_mutex_unlock(&queue_list_mutex);
	}

	/* Is it already subscribe to the service ?*/
	pthread_mutex_lock(requested_service-> mutex);
	if(HPD_NO == is_queue_subscribed_to_service(event_queue, requested_service-> value_url ))
	{
		/* If No, then subscribe */
		new_service_id = create_service_id( requested_service-> value_url );
		add_service_id_to_queue(new_service_id, event_queue);

		xmlbuff = get_xml_subscription (SUBSCRIBE, requested_service-> value_url);
		if(xmlbuff == NULL)
		{
			pthread_mutex_unlock(requested_service-> mutex);
			return MHD_NO;
		}
	}

	else
	{
		/* If Yes, then unsuscribe */
		xmlbuff = get_xml_subscription (UNSUBSCRIBE, requested_service-> value_url);

		if(xmlbuff == NULL)
		{
			pthread_mutex_unlock(event_queue-> mutex);
			pthread_mutex_unlock(requested_service-> mutex);
			return MHD_NO;
		}

		remove_service_id_from_queue( requested_service-> value_url, event_queue);
	}

	pthread_mutex_unlock(event_queue-> mutex);
	pthread_mutex_unlock(requested_service->mutex);
	send_cookied_xml (connection, xmlbuff, event_queue, is_secure_connection);

	return MHD_YES;
}


/**
 * Subscribe to Log events 
 *
 * @param connection The connection that is used
 *
 * @param con_cls A generic pointer that is used to store the xml in order to be able to free it
 *
 * @param is_secure_connection Variable that states if the connection is secured or not ( HPD_IS_SECURE_CONNECTION or HPD_IS_SECURE_CONNECTION )
 *
 * @return MHD_NO if failed, MHD_YES if successful
 */
int 
subscribe_to_log_events(struct MHD_Connection *connection, void **con_cls, int is_secure_connection)
{
	char *xmlbuff;
	EventQueue *event_queue = get_queue(connection);


	/*Is there a queue for this cookie ?*/
	if(!event_queue)
	{
		event_queue = create_event_queue();
		pthread_mutex_lock(&queue_list_mutex);
		DL_APPEND(event_queue_list_head, event_queue);
		pthread_mutex_unlock(&queue_list_mutex);
	}

	pthread_mutex_lock(event_queue-> mutex);

	if(event_queue-> send_log_events == HPD_YES)
	{
		event_queue-> send_log_events = HPD_NO;
		xmlbuff = get_xml_subscription (UNSUBSCRIBE, "log");
		if(xmlbuff == NULL)
		{
			pthread_mutex_unlock(event_queue->mutex);
			return MHD_NO;
		}
	}
	else
	{
		event_queue-> send_log_events = HPD_YES;
		xmlbuff = get_xml_subscription (SUBSCRIBE, "log");
		if(xmlbuff == NULL)
		{
			pthread_mutex_unlock(event_queue->mutex);
			return MHD_NO;
		}
	}

	pthread_mutex_unlock(event_queue->mutex);
	send_cookied_xml (connection, xmlbuff, event_queue, is_secure_connection);

	return MHD_YES;

}


/**
 * Sets up  the connection to send Server Sent Events to a client
 *
 * @param connection The connection that is used
 *
 * @param is_secure_connection Variable that states if the connection is secured or not ( HPD_IS_SECURE_CONNECTION or HPD_IS_SECURE_CONNECTION )
 *
 * @return MHD_HTTP_NOT_FOUND if there is no corresponding EventQueue, MHD_NO if the response failed to create, the returning of MHD_queue_response if successful
 */
int 
set_up_server_sent_event_connection(struct MHD_Connection *connection, 
                                    int is_secure_connection)
{
	struct MHD_Response *response;
	int ret;
	char *new_connection_id;
	EventQueue *event_queue = get_queue(connection);

	if(!event_queue)
		return MHD_HTTP_NOT_FOUND;
	else
	{
		new_connection_id = generate_connection_id();
		pthread_mutex_lock(event_queue-> mutex);
		if(event_queue-> is_waiting > HPD_NO && event_queue-> connection_id)
		{
			event_queue-> connection_id = new_connection_id;
			pthread_cond_broadcast(event_queue->wait_for_event);
		}
		else
			event_queue-> connection_id = new_connection_id;
		pthread_mutex_unlock(event_queue-> mutex);
	}


	if(event_queue-> service_id_head != NULL || event_queue-> send_log_events == HPD_SEND_LOG_EVENTS)
	{
   // TODO Needs change to new webserver
		response = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN,
		                                              32 * 1024,
		                                              &server_sent_events_callback,
		                                              new_connection_id,
		                                              &server_sent_events_free_callback);
		if(response == NULL) return MHD_NO;

		add_session_cookie(event_queue, response, is_secure_connection);

   // TODO Needs change to new webserver
		MHD_add_response_header (response, "Content-Type", "text/event-stream; charset=utf-8");
		ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
		if(response != NULL)
			MHD_destroy_response (response);

		return ret;
	}
	return MHD_HTTP_NOT_FOUND;
}

/**
 * Frees the ressources used for the server sent events
 *
 * @return HPD_E_SUCCESS if successful
 */
int 
free_server_sent_events_ressources()
{
	EventQueue *iterator, *tmp;

	pthread_mutex_lock( &queue_list_mutex );
	DL_FOREACH_SAFE( event_queue_list_head, iterator, tmp )
	{
		DL_DELETE( event_queue_list_head, iterator );
		exit_event_queue( iterator );
	}
	pthread_mutex_unlock( &queue_list_mutex );
	exit_event_queue( global_event_queue );
	pthread_join( global_event_thread, NULL );
	return HPD_E_SUCCESS;
}

/**
 * Method used to return an EventQueue corresponding to a connection
 *
 * @param connection The connection from which we want to retrieve the EventQueue
 *
 * @return NULL if failed, the EventQueue if successful
 */
EventQueue *
get_queue(struct MHD_Connection *connection)
{

	EventQueue *iterator;
   // TODO Needs change to new webserver
	const char *session_id = MHD_lookup_connection_value (connection,
	                                                      MHD_COOKIE_KIND,
	                                                      COOKIE_NAME);
	if(session_id != NULL)
	{
		pthread_mutex_lock(&queue_list_mutex);
		DL_FOREACH(event_queue_list_head, iterator)
		{
			pthread_mutex_lock(iterator-> mutex);
			if(0 == strcmp(session_id, iterator-> session_id))
			{
				pthread_mutex_unlock(iterator-> mutex);
				pthread_mutex_unlock(&queue_list_mutex);
				return iterator;
			}	
			pthread_mutex_unlock(iterator-> mutex);		
		}
		pthread_mutex_unlock(&queue_list_mutex);
	}
	return NULL;    

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
	EventQueue *iterator;
	Event *new_event;

	if(!service_to_notify)
		return HPD_E_SERVICE_IS_NULL;

	pthread_mutex_lock(&queue_list_mutex);
	DL_FOREACH(event_queue_list_head, iterator)
	{
		if(HPD_YES == set_availability_of_service(service_to_notify-> value_url,iterator, availability ))
		{
			new_event = create_event( "service_availability", service_to_notify-> value_url , NULL, (availability == HPD_YES) ? "available" : "unavailable" );
			queue_event( iterator, new_event, HPD_NO );
		}
	}
	pthread_mutex_unlock(&queue_list_mutex);
	return HPD_E_SUCCESS;	
}




