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

#include 	"server_sent_events.h"
#include	"hpd_error.h"

EventSubscriber *subscribers_head;/**< The head of the list of the clients that have subscribed to the server-sent events notifications */

/**
 * Creates the structure EventSubscriber
 *
 * @return returns the EventSubscriber or NULL if failed
 */
EventSubscriber *create_event_subscriber()
{
    EventSubscriber *_event_subscriber = (EventSubscriber*)malloc(sizeof(EventSubscriber));

    _event_subscriber-> event_received = HPD_NO_EVENT_RECEIVED;
    _event_subscriber-> send_log_events = 0;
    _event_subscriber-> first_log_event = HPD_YES;
    _event_subscriber-> log_event_message = NULL;
    _event_subscriber-> updated_value = NULL;
    _event_subscriber-> notified_service = NULL;
    _event_subscriber-> subscribed_service_head = NULL;
    _event_subscriber-> next = NULL;
    _event_subscriber-> session_id = generate_session_id();
    _event_subscriber-> mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(_event_subscriber-> mutex, NULL);
    _event_subscriber-> condition_var = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(_event_subscriber-> condition_var, NULL);
    _event_subscriber-> finished_sent = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(_event_subscriber-> finished_sent, NULL);
    
    return _event_subscriber;
}
/**
 * Destroys the structure EventSubscriber
 *
 * @param _event_subscriber The EventSubscriber to destroy
 *
 * @return returns HPD_E_SUCCESS if successful, HPD_E_EVENT_SUBSCRIBER_DOESNT_EXIST if the EventSubscriber does not exist
 */
int destroy_event_subscriber(EventSubscriber *_event_subscriber)
{
    if(_event_subscriber)
    {
	if(_event_subscriber->condition_var) 
	{	
		free(_event_subscriber->condition_var);
	}

	if(_event_subscriber->finished_sent) 
	{	
		free(_event_subscriber->finished_sent);
	}

        ServiceElement *iterator;
        LL_FOREACH(_event_subscriber->subscribed_service_head,iterator)
        {
	   LL_DELETE(_event_subscriber->subscribed_service_head,iterator);
	   destroy_subscribed_service_element (iterator);
        }
        if(_event_subscriber->session_id) free(_event_subscriber->session_id);
	if(_event_subscriber->log_event_message) free(_event_subscriber->log_event_message);
	if(_event_subscriber->updated_value) free(_event_subscriber->updated_value);

	if(_event_subscriber->mutex) 
	{
		pthread_mutex_destroy(_event_subscriber->mutex);
		free(_event_subscriber->mutex);
	}
        free(_event_subscriber);
        return HPD_E_SUCCESS;
    }
    return HPD_E_EVENT_SUBSCRIBER_DOESNT_EXIST;
}

/**
 * Returns the EventSubscriber corresponding to the client that uses the connection, if it doesn't exist, it will create it
 *
 * @param connection The connection we are checking
 *
 * @return returns the corresponding EventSubscriber
 */
EventSubscriber *get_session(struct MHD_Connection *connection)
{

    EventSubscriber *iterator;
    EventSubscriber *ret = NULL;

    const char *cookie;
    cookie = MHD_lookup_connection_value (connection, MHD_COOKIE_KIND, COOKIE_NAME);

    if(cookie!=NULL)
    {
        LL_FOREACH(subscribers_head, iterator)
        {
	   if(0 == strcmp(cookie, iterator->session_id))
	      {
		 ret = iterator;
	      }
        }

        if(NULL != ret)
        {
	   return ret;
        }
    }

    return NULL;    
}
/**
 * Adds a Cookie Header corresponding to the session to the response
 *
 * @param _event_subscriber The EventSubscriber that possess the session ID
 *
 * @param response The response to add the header
 *
 * @param is_secure_connection Variable that states if the connection is secured or not ( HPD_IS_SECURE_CONNECTION or HPD_IS_SECURE_CONNECTION ) 
 *
 * @return void
 */
static void add_session_cookie( EventSubscriber *_event_subscriber, struct MHD_Response *response, int is_secure_connection)
{
    char cstr[256];
    pthread_mutex_lock(_event_subscriber->mutex);
    switch(is_secure_connection)
    {
	case HPD_IS_SECURE_CONNECTION :

		snprintf(cstr, sizeof(cstr), "%s=%s; path=/; secure", COOKIE_NAME, _event_subscriber->session_id);
		break;

	case HPD_IS_UNSECURE_CONNECTION :

		snprintf(cstr, sizeof(cstr), "%s=%s; path=/", COOKIE_NAME, _event_subscriber->session_id);
		break;
    }
    pthread_mutex_unlock(_event_subscriber->mutex);
    if(MHD_NO == MHD_add_response_header (response, MHD_HTTP_HEADER_SET_COOKIE, cstr))
	printf("Failed to add cookie header\n");
}

/**
 * Callback of the server-sent Events
 *
 * @param cls Generic pointer that is used to pass an EventSubscriber
 *
 * @param pos The position in the datastream to access
 *
 * @param buf The response buffer
 *
 * @param max The maximum size of the buffer
 *
 * @return returns 0 if no events have been received, the size of the buffer if else
 */
static ssize_t server_sent_events_callback (void *cls, uint64_t pos, char *buf, size_t max)
{
	

	EventSubscriber *client = cls;
	if(client == NULL) return MHD_NO;
	ServiceElement *_notified_service_element = NULL;

	/*Lock the client Mutex*/
	pthread_mutex_lock(client->mutex);

	/*If we are not in a state of end of program, we wait for the condition
	  variable that states if an event has been received*/
	if(client->event_received != HPD_NO_MORE_EVENTS)
	{
		pthread_cond_signal(client->finished_sent);
		pthread_cond_wait(client->condition_var, client->mutex);
		int ret;

		switch(client->event_received)
		{
			case HPD_NO_MORE_EVENTS :
				pthread_mutex_unlock(client->mutex);
				return 0;	
				break;

			case HPD_EVENT_RECEIVED_WITH_SERVICE :
				//NOT USED FOR NOW 
				/*
				_notified_service_element = matching_service (client->subscribed_service_head, client->notified_service->value_url);
				pthread_mutex_lock(_notified_service_element->mutex);
				ret = _notified_service_element->service->get_function(_notified_service_element->service, 
											_notified_service_element->service->get_function_buffer,
											MHD_MAX_BUFFER_SIZE);
				if(ret != 0)
					memset(_notified_service_element->service->get_function_buffer, '\0', ret);
				else
					return 0;

				sprintf(buf,"data: %s<br>id: %s<br><br>",
								_notified_service_element->service->get_function_buffer,
								_notified_service_element->service->value_url);
				pthread_mutex_unlock(_notified_service_element->mutex);*/

				break;

			case HPD_EVENT_RECEIVED_WITH_VALUE :

				sprintf(buf,"data: %s<br>id: %s<br><br>", client->updated_value, client->notified_service->value_url);
				break;

			case HPD_EVENT_RECEIVED_WITH_LOG :
			
				sprintf(buf,"data: %s<br>id: log<br><br>",client->log_event_message);
				break;

			case HPD_EVENT_RECEIVED_WITH_UNSUBSCRIPTION :
				sprintf(buf,"data: unavailable<br>id: %s<br><br>",client->updated_value);
				break;
		}
		client->event_received = HPD_NO_EVENT_RECEIVED;

		/*Unlock the client Mutex*/
		pthread_mutex_unlock(client->mutex);
		return strlen(buf);
	}
	else
	{
		pthread_mutex_unlock(client->mutex);
		return 0;
	}
}

/**
 * Callback to free resources linked to server-sent Events 
 *
 * @param cls Generic pointer that is used to pass an EventSubscriber
 *
 * @return void
 */
static void server_sent_events_free_callback (void *cls)
{

	EventSubscriber *client = cls;
	if(client != NULL && client->subscribed_service_head == NULL && client->send_log_events == HPD_DONT_SEND_LOG_EVENTS)
	{
			pthread_mutex_lock(client->mutex);
			client->event_received = HPD_NO_MORE_EVENTS;
			pthread_mutex_unlock(client->mutex);

			pthread_mutex_lock(client->mutex);
			pthread_cond_signal(client->condition_var);
			pthread_mutex_unlock(client->mutex);

			pthread_cond_destroy(client->finished_sent);
			pthread_cond_destroy(client->condition_var);

		LL_DELETE(subscribers_head, client);
		destroy_event_subscriber(client);
	}
}

/**
 * Sends an event of a changing of value from a Service
 *
 * @param service The service that value changed
 *
 * @return HPD_E_SUCCESS if successful
 */
int send_event (Service *service)
{
    EventSubscriber *iterator;
    ServiceElement *_notified_service_element = NULL;
    
    LL_FOREACH(subscribers_head, iterator)
    {
        if( ( _notified_service_element = matching_service (iterator->subscribed_service_head, service->value_url) ) !=NULL )
        {
	   pthread_mutex_lock(iterator->mutex);
	   iterator->event_received = HPD_EVENT_RECEIVED_WITH_SERVICE;
	   iterator->notified_service = service;
	   pthread_cond_signal(iterator->condition_var);
	   pthread_cond_wait(iterator->finished_sent, iterator->mutex);
	   pthread_mutex_unlock(iterator->mutex);
        }
    }

	return HPD_E_SUCCESS;
}

/**
 * Sends an event of a changing of value from a given Service
 *
 * @param _service_url The URL of the service
 *
 * @param _updated_value The new value.
 *
 * @return HPD_E_SUCCESS if successful
 */
int send_event_of_value_change (Service *service, char *_updated_value)
{
    EventSubscriber *iterator;
    ServiceElement *_notified_service_element = NULL;
    
    LL_FOREACH(subscribers_head, iterator)
    {
        if( ( _notified_service_element = matching_service (iterator->subscribed_service_head, service->value_url) ) !=NULL )
        {
	   pthread_mutex_lock(iterator->mutex);

	   if(iterator->updated_value == NULL) 
		iterator->updated_value = malloc(sizeof(char)*strlen(_updated_value));
	   	else 
		iterator->updated_value = realloc(iterator->updated_value, sizeof(char)*strlen(_updated_value));
           strcpy(iterator->updated_value,_updated_value);

	   iterator->notified_service = service;
	   iterator->event_received = HPD_EVENT_RECEIVED_WITH_VALUE;

	   pthread_cond_signal(iterator->condition_var);
	   pthread_cond_wait(iterator->finished_sent, iterator->mutex);

	   pthread_mutex_unlock(iterator->mutex);
        }
    }

	return HPD_E_SUCCESS;
}

/**
 * Sends the log event
 *
 * @param log_message The Log message to send
 *
 * @return HPD_E_SUCCESS if successful
 */
int send_log_event(char *log_message)
{
    EventSubscriber *iterator;
    
    LL_FOREACH(subscribers_head, iterator)
    {
        if(iterator->send_log_events == HPD_SEND_LOG_EVENTS)
        {
	   if(iterator->first_log_event == HPD_YES)
	   {
		   iterator->first_log_event = HPD_NO;
	   }
	   else
	   {
		   pthread_mutex_lock(iterator->mutex);

		   if(iterator->log_event_message == NULL) 
			iterator->log_event_message = (char*)malloc(sizeof(char)*strlen(log_message)+1);
		   	else 
				iterator->log_event_message = realloc(iterator->log_event_message, sizeof(char)*strlen(log_message)+1);
		   strcpy(iterator->log_event_message,log_message);

		   iterator->event_received = HPD_EVENT_RECEIVED_WITH_LOG;
		   pthread_cond_signal(iterator->condition_var);
		   pthread_cond_wait(iterator->finished_sent, iterator->mutex);
		   pthread_mutex_unlock(iterator->mutex);
	   }
        }
    }
    
    return HPD_E_SUCCESS;
}

/**
 * Sets up the stream of events with the client
 *
 * @param connection The connection that is used
 *
 * @param is_secure_connection Variable that states if the connection is secured or not ( HPD_IS_SECURE_CONNECTION or HPD_IS_SECURE_CONNECTION )
 *
 * @return MHD_HTTP_NOT_FOUND if there is no corresponding session, MHD_NO if the respons failed to create
 */
int set_up_server_sent_event_connection(struct MHD_Connection *connection, int is_secure_connection)
{
    struct MHD_Response *response;
    int ret;

    EventSubscriber *client;
    client = get_session(connection);

    if(client == NULL)
    {
         return MHD_HTTP_NOT_FOUND;
    }
    else if(client->subscribed_service_head != NULL || client->send_log_events == HPD_SEND_LOG_EVENTS)
    {
	
        response = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN,
                                                      32 * 1024,
                                                      &server_sent_events_callback,
                                                      client,
                                                      &server_sent_events_free_callback);
        if(response == NULL) return MHD_NO;

        add_session_cookie(client, response, is_secure_connection);

        MHD_add_response_header (response, "Content-Type", "text/html; charset=utf-8");
        ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
        if(response != NULL)
	   MHD_destroy_response (response);

        return ret;
    }
    else
	return MHD_HTTP_NOT_FOUND;
}
/**
 * Subscribe to service events
 *
 * @param connection The connection that is used
 *
 * @param _requested_service_element The concerned ServiceElement
 *
 * @param con_cls A generic pointer that is used to store the xml in order to be able to free it
 *
 * @param is_secure_connection Variable that states if the connection is secured or not ( HPD_IS_SECURE_CONNECTION or HPD_IS_SECURE_CONNECTION )
 *
 * @return HPD_E_SUCCESS if successful, HPD_E_FAILED_XML_SUBSCRIPTION if failed
 */
int subscribe_to_service(struct MHD_Connection *connection, ServiceElement *_requested_service_element, void **con_cls, int is_secure_connection)
{

    	pthread_mutex_lock(_requested_service_element->mutex);
    
    	EventSubscriber *client;
    	char *xmlbuff;
    	ServiceElement *is_subscribed_element = NULL;
    
    	client = get_session(connection);

	if(client != NULL)
	{
		pthread_mutex_lock(client->mutex);
		if( ( is_subscribed_element = matching_service (client->subscribed_service_head, _requested_service_element->service->value_url) ) !=NULL )
		{
			xmlbuff = get_xml_subscription (UNSUBSCRIBE, is_subscribed_element->service->value_url);

			if(xmlbuff == NULL)
			{
				pthread_mutex_unlock(client->mutex);
				pthread_mutex_unlock(_requested_service_element->mutex);
				return MHD_NO;
			}

			LL_DELETE(client->subscribed_service_head, is_subscribed_element);
			destroy_subscribed_service_element (is_subscribed_element);
		}
		else
		{
			is_subscribed_element = create_subscribed_service_element (_requested_service_element);
			LL_APPEND(client->subscribed_service_head,is_subscribed_element);
			xmlbuff = get_xml_subscription (SUBSCRIBE, is_subscribed_element->service->value_url);

			if(xmlbuff == NULL)
			{
				pthread_mutex_unlock(client->mutex);
				pthread_mutex_unlock(_requested_service_element->mutex);
				return MHD_NO;
			}
		}
	}
	else
	{
		client = create_event_subscriber();
		LL_APPEND(subscribers_head, client);
		
		pthread_mutex_lock(client->mutex);

		is_subscribed_element = create_subscribed_service_element (_requested_service_element);
        	LL_APPEND(client->subscribed_service_head,is_subscribed_element);
        	xmlbuff = get_xml_subscription (SUBSCRIBE, is_subscribed_element->service->value_url);
		if(xmlbuff == NULL)
		{
			pthread_mutex_lock(client->mutex);
			pthread_mutex_unlock(_requested_service_element->mutex);
			return MHD_NO;
		}
	}

	pthread_mutex_unlock(_requested_service_element->mutex);

	if(client->subscribed_service_head == NULL && client->send_log_events == HPD_DONT_SEND_LOG_EVENTS)
	{
		pthread_mutex_unlock(client->mutex);
		*con_cls = xmlbuff;
		send_end_of_session_xml (connection, xmlbuff);
	}
	else
	{
		pthread_mutex_unlock(client->mutex);
		*con_cls = xmlbuff;
		send_cookied_xml (connection, xmlbuff, client, is_secure_connection);
	}

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
 * @return HPD_E_SUCCESS if successful, HPD_E_FAILED_XML_SUBSCRIPTION if failed
 */
int subscribe_to_log_events(struct MHD_Connection *connection, void **con_cls, int is_secure_connection)
{
    	EventSubscriber *client;
    	char *xmlbuff;
    	client = get_session(connection);

	if(client == NULL)
	{
		client = create_event_subscriber();
		LL_APPEND(subscribers_head, client);

		pthread_mutex_lock(client->mutex);
		
		client->send_log_events = HPD_SEND_LOG_EVENTS;
		xmlbuff = get_xml_subscription (SUBSCRIBE, "LOG");
		if(xmlbuff == NULL)
		{
			pthread_mutex_unlock(client->mutex);
			return MHD_NO;
		}
		
		
	}
	else
	{
		pthread_mutex_lock(client->mutex);
		
		    if(client->send_log_events == HPD_SEND_LOG_EVENTS)
		    {
			client->send_log_events = HPD_DONT_SEND_LOG_EVENTS;
			xmlbuff = get_xml_subscription (UNSUBSCRIBE, "LOG");
			if(xmlbuff == NULL)
			{
				pthread_mutex_unlock(client->mutex);
				return MHD_NO;
			}
		    }
		    else
		    {
			client->send_log_events = HPD_SEND_LOG_EVENTS;
			xmlbuff = get_xml_subscription (SUBSCRIBE, "LOG");
			if(xmlbuff == NULL)
			{
				pthread_mutex_unlock(client->mutex);
				return MHD_NO;
			}
		    }
	}



    *con_cls = xmlbuff;

    int i = 0;
    EventSubscriber *iterator;
    LL_FOREACH(subscribers_head, iterator)
    {
        if(iterator->send_log_events == HPD_SEND_LOG_EVENTS)
            i++;
    }

    if(i == 0)
        set_event_sending(HPD_DONT_SEND_LOG_EVENTS);
    else
        set_event_sending(HPD_SEND_LOG_EVENTS);

    

	if(client->subscribed_service_head == NULL && client->send_log_events == HPD_DONT_SEND_LOG_EVENTS)
	{
		pthread_mutex_unlock(client->mutex);
		*con_cls = xmlbuff;
		send_end_of_session_xml (connection,xmlbuff);
	}
	else
	{
		pthread_mutex_unlock(client->mutex);
		*con_cls = xmlbuff;
		send_cookied_xml (connection,xmlbuff, client, is_secure_connection);
	}

	return MHD_YES;
}

/**
 * Add an XML response to the queue of the server with cookie
 *
 * @param connection The client connection which will receive the response
 *
 * @param xmlbuff The XML response to be sent
 *
 * @param client The Session that uses the connection
 *
 * @param is_secure_connection Variable that states if the connection is secured or not ( HPD_IS_SECURE_CONNECTION or HPD_IS_SECURE_CONNECTION )
 *
 * @return MHD return value, MHD_NO if the response failed to be created, 
 *		   return code of MHD_queue_response otherwise
 */
int send_cookied_xml (struct MHD_Connection *connection, const char *xmlbuff, EventSubscriber *client, int is_secure_connection)
{

    int ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer (strlen(xmlbuff),xmlbuff, MHD_RESPMEM_PERSISTENT);

    if(!response)
    {
        if(xmlbuff)
	   free(xmlbuff);
        return MHD_NO;
    }	
    
    add_session_cookie(client, response, is_secure_connection);
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
char *generate_session_id()
{
        uuid_t *uuid = (unsigned char*)malloc (sizeof (uuid_t));
        char *uuid_str = (char*)malloc (37 * sizeof (char));
        uuid_generate_random (uuid);
        uuid_unparse(uuid, uuid_str);
        free (uuid);
        
        return uuid_str;
}

/**
 * Releases the locked mutex that are blocked from the conditional variable and frees the conditional variable
 *
 * @return HPD_E_SUCCESS if successful, HPD_E_SUBSCRIBERS_HEAD_EMPTY if the subscribers list is empty
 */
int free_condition_variable()
{
	if(subscribers_head)
	{
		EventSubscriber *iterator;
		LL_FOREACH(subscribers_head, iterator)
		{
			pthread_mutex_lock(iterator->mutex);
			iterator->event_received = HPD_NO_MORE_EVENTS;
			pthread_mutex_unlock(iterator->mutex);
		}
		LL_FOREACH(subscribers_head, iterator)
		{
			pthread_mutex_lock(iterator->mutex);
			pthread_cond_signal(iterator->condition_var);
			pthread_mutex_unlock(iterator->mutex);
		}
		LL_FOREACH(subscribers_head, iterator)
		{
			pthread_cond_destroy(iterator->finished_sent);
			pthread_cond_destroy(iterator->condition_var);
		}
		return HPD_E_SUCCESS;
	}

	return HPD_E_SUBSCRIBERS_HEAD_EMPTY;
	
}

/**
 * Frees the ressources used for the server sent events
 *
 * @return HPD_E_SUCCESS if successful, HPD_E_SUBSCRIBERS_HEAD_EMPTY if the subscribers list is empty
 */
int free_server_sent_events_ressources()
{
	if(subscribers_head)
	{
		EventSubscriber *iterator;
		LL_FOREACH(subscribers_head, iterator)
		{
			LL_DELETE(subscribers_head, iterator);
			destroy_event_subscriber(iterator);
		}
		return HPD_E_SUCCESS;
	}
	return HPD_E_SUBSCRIBERS_HEAD_EMPTY;
}

/**
 * Add an XML response to the queue of the server
 *
 * @param connection The client connection which will receive the response
 *
 * @return MHD return value, MHD_NO if the response failed to be created, 
 *		   return code of MHD_queue_response otherwise
 */
static int
send_end_of_session_xml (struct MHD_Connection *connection, const char *xmlbuff)
{

    int ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer (strlen(xmlbuff),xmlbuff, MHD_RESPMEM_PERSISTENT);

    if(!response)
    {
        if(xmlbuff)
	   free(xmlbuff);

	return MHD_NO;
    }

    MHD_add_response_header (response, "Content-Type", "text/xml");
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

/**
 * In the case of an unregistering of a Service in the server, notifies the different subscribed clients that the service is now unavailable and frees its ressources
 *
 * @param _service_to_unsubscribe The service to unsubscribe
 *
 * @return HPD_E_SUCCESS if successful
 */
int unsubscribe_and_notify_service_leaving(Service* _service_to_unsubscribe)
{    
    	EventSubscriber *iterator;
	ServiceElement *is_subscribed_element = NULL;

	LL_FOREACH(subscribers_head,iterator)
	{
		if( ( is_subscribed_element = matching_service (iterator->subscribed_service_head,  _service_to_unsubscribe->value_url) ) !=NULL )
		{
			pthread_mutex_lock(iterator->mutex);
			LL_DELETE(iterator->subscribed_service_head, is_subscribed_element);
			destroy_subscribed_service_element (is_subscribed_element);
			is_subscribed_element = NULL;
		
			if(iterator->updated_value == NULL) 
				iterator->updated_value = malloc(sizeof(char)*strlen(_service_to_unsubscribe->value_url));
	   			else 
				iterator->updated_value = realloc(iterator->updated_value, sizeof(char)*strlen(_service_to_unsubscribe->value_url));
           		strcpy(iterator->updated_value,_service_to_unsubscribe->value_url);

			iterator->event_received = HPD_EVENT_RECEIVED_WITH_UNSUBSCRIPTION;
	   		pthread_cond_signal(iterator->condition_var);
           		pthread_mutex_unlock(iterator->mutex);
		}
	}
	return HPD_E_SUCCESS;
}




