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


#include <assert.h>

#include "web_server_api.h"
#include "hpd_error.h"


static ServiceElement *service_head;/**< List containing all the services handled by the server */
static struct MHD_Daemon *d;/**< MDH daemon for the MHD web server listening for incoming connections */


int done_flag = 0;
int answer_continue = 0;



/**
 * Add an XML response to the queue of the server
 *
 * @param connection The client connection which will receive the response
 *
 * @return MHD return value, MHD_NO if the response failed to be created, 
 *		   return code of MHD_queue_response otherwise
 */
static int
send_xml (struct MHD_Connection *connection, const char *xmlbuff)
{

    int ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer (strlen( (char*)xmlbuff),(char *)xmlbuff, MHD_RESPMEM_PERSISTENT);

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
 * Add an error response to the queue of the server
 *
 * @param connection The client connection which will receive the response
 * 
 * @param http_error_code The http error code to send
 *
 * @return MHD return value, MHD_NO if the response failed to be created, 
 *		   return code of MHD_queue_response otherwise
 */
static int send_error(struct MHD_Connection *connection, int http_error_code)
{
    int ret;
    struct MHD_Response *response;

    switch( http_error_code )
    {
	case MHD_HTTP_NOT_FOUND :
		response = MHD_create_response_from_data(strlen("Not Found"), (void *) "Not Found", MHD_NO, MHD_NO);
		break;

	case MHD_HTTP_BAD_REQUEST :
		response = MHD_create_response_from_data(strlen("Bad Request"), (void *) "Bad Request", MHD_NO, MHD_NO);
		break;
	default :
		response = MHD_create_response_from_data(strlen("Unknown error"), (void *) "Unknown error", MHD_NO, MHD_NO);
		break;
    } 

    if(!response)
        return MHD_NO;
    ret = MHD_queue_response (connection, http_error_code, response);
    MHD_destroy_response(response);

    return ret;
}

/**
 * Free the XML response sent to a client when a request is completed (MHD_RequestCompletedCallback)
 *
 * @param cls Custom value selected at callback registration time (NOT USED)
 *	
 * @param connection The client connection to which the response has been sent
 *
 * @param con_cls Pointer to the XML file to free set in answer_to_connection
 *
 * @param toe Reason for request termination see MHD_OPTION_NOTIFY_COMPLETED (NOT USED)
 *
 * @return 0 if an XML file has been freed, -1 otherwise
 */
int request_completed(void *cls, struct MHD_Connection *connection, 
                      void **con_cls, enum MHD_RequestTerminationCode toe)
{
    if(*con_cls)
    {
        free(*con_cls);
        return 0;
    }

    return -1;

}

/**
 * Callback function used to answer clients's connection (MHD_AccessHandlerCallback)
 *
 * @param cls Custom value selected at callback registration time, used to initialize the done flag
 *	
 * @param connection The client connection 
 *
 * @param url The url on which the client made its request
 *
 * @param method The http method with which the client made its request (Only GET and PUT supported)
 *
 * @param version The HTTP version string (i.e. HTTP/1.1)
 *
 * @param upload_data Data beeing uploaded when receiving PUT
 *
 * @param upload_data_size Size of the data beeing uploaded when receiving PUT
 *
 * @param con_cls reference to a pointer, initially set to NULL, that this callback can set to some 
 *        address and that will be preserved by MHD for future calls for this request
 *
 * @return MHD_YES to pursue the request handling, MHD_NO in case of error with 
 *         the request, return value of the send_* functions otherwise
 */
int answer_to_connection (void *cls, struct MHD_Connection *connection, 
                          const char *url, 
                          const char *method, const char *version, 
                          const char *upload_data, 
                          size_t *upload_data_size, void **con_cls)
{
    ServiceElement *_requested_service_element;
    int *done = cls;
    static int aptr;
	struct sockaddr *addr;
	addr = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
	char IP[16];
    	inet_ntop(addr->sa_family,addr->sa_data + 2, IP, 16);

    if( 0 == strcmp (method, MHD_HTTP_METHOD_GET) )
    {

        if (&aptr != *con_cls)
        {
	   /* do never respond on first call */
	   *con_cls = &aptr;
	   return MHD_YES;
        }
        *con_cls = NULL;
        char *xmlbuff;

        if(strcmp(url,"/log") == 0)
	{
		Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
		return subscribe_to_log_events(connection, con_cls, HPD_IS_UNSECURE_CONNECTION);
	}
        else if(strcmp(url,"/events") == 0)
        {
	   Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
	   int ret = set_up_server_sent_event_connection(connection, HPD_IS_UNSECURE_CONNECTION);
	   if(ret == 404) return send_error (connection, MHD_HTTP_NOT_FOUND);
	   else return ret;
        }
        else if(strcmp(url,"/devices") == 0)
        {
		Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
		
		xmlbuff = get_xml_device_list();

		return send_xml (connection, xmlbuff);
	   }
        else if( ( _requested_service_element = matching_service (service_head, url) ) !=NULL )
        {
	   const char *_get_arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "x");
	   if(_get_arg != NULL)
	   {
	       if(strcmp(_get_arg, "1") == 0)
	       {
		 Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, "x=1");
		  pthread_mutex_lock(_requested_service_element->mutex);
		  xmlbuff = extract_service_xml(_requested_service_element->service);

		  *con_cls = xmlbuff;

		  pthread_mutex_unlock(_requested_service_element->mutex);
		  return send_xml (connection, xmlbuff);
	       }
	   }

	   /*Request for the Subscription to the events from the service*/
	    _get_arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "p");
	   if(_get_arg != NULL)
	   {
	       if(strcmp(_get_arg, "1") == 0)
	       {
                Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, "p=1");
		  return subscribe_to_service(connection, _requested_service_element, con_cls, HPD_IS_UNSECURE_CONNECTION);
	       }
	   }

	  Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
	   pthread_mutex_lock(_requested_service_element->mutex);

		int ret;
		ret = _requested_service_element->service->get_function(_requested_service_element->service, 
										_requested_service_element->service->get_function_buffer,
										MHD_MAX_BUFFER_SIZE);
		if(ret != 0)
			_requested_service_element->service->get_function_buffer[ret] = '\0';
		else
			return send_error(connection, MHD_HTTP_NOT_FOUND);

	   xmlbuff = get_xml_value (_requested_service_element->service->get_function_buffer);
	   *con_cls = xmlbuff;
	   pthread_mutex_unlock(_requested_service_element->mutex);
	   return send_xml(connection, xmlbuff);
        }
        else
        {
	   return send_error(connection, MHD_HTTP_NOT_FOUND);
        }
    }

    else if( 0 == strcmp(method, MHD_HTTP_METHOD_PUT) )
    {  
        if( ( *con_cls ) == NULL )
        {
			if(*upload_data_size == 0)
			{
				return MHD_YES; /* not ready yet */
			}	    
			/* Add a space for a '/0' in order to clear the end of the XML */
			char *put_data_temp = (char*)malloc((*upload_data_size)*sizeof(char)+1);
			memcpy(put_data_temp, upload_data, *upload_data_size);
			put_data_temp[*upload_data_size]='\0';
			*con_cls = put_data_temp;
			*upload_data_size = 0;
			return MHD_YES;
		}
		else
		{
			if( ( _requested_service_element = matching_service (service_head, url) ) !=NULL )
			{
				Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
				pthread_mutex_lock(_requested_service_element->mutex);
				if( _requested_service_element->service->put_function != NULL && *con_cls != NULL)
				{
					char* _value = get_value_from_xml_value (*con_cls);
					if(_value == NULL)
					{
						pthread_mutex_unlock(_requested_service_element->mutex);
						return send_error (connection, MHD_HTTP_BAD_REQUEST);
					}

					free(*con_cls);

					int ret;
					ret = _requested_service_element->service->put_function(_requested_service_element->service, 
					                                                        _requested_service_element->service->get_function_buffer,
					                                                        MHD_MAX_BUFFER_SIZE,
					                                                        _value);
					free(_value);
					
					if(ret != 0)
						_requested_service_element->service->get_function_buffer[ret] = '\0';
					else
						return send_error(connection, MHD_HTTP_NOT_FOUND);

					char* xmlbuff = get_xml_value (_requested_service_element->service->get_function_buffer);
					*con_cls = xmlbuff;
					pthread_mutex_unlock(_requested_service_element->mutex);
					send_event_of_value_change (_requested_service_element->service, _requested_service_element->service->get_function_buffer);
					return send_xml (connection, xmlbuff);
				}
				else
				{
					pthread_mutex_unlock(_requested_service_element->mutex);
					return send_error(connection, MHD_HTTP_BAD_REQUEST);
				}
			}
	   else
	       return send_error (connection, MHD_HTTP_NOT_FOUND);
        }
    }
    return MHD_NO;
}

/**
 * Add a service the server's service list
 *
 * @param _service The service to add
 *
 * @return A HPD error code
 */
int register_service_in_unsecure_server(Service *_service)
{
    int rc;

    assert(d);

    ServiceElement *_service_element_to_add = create_service_element (_service);

    LL_APPEND(service_head, _service_element_to_add);
    if(service_head == NULL)
    {
        printf("add_service_to_list failed\n");
        return HPD_E_LIST_ERROR;
    }

    return HPD_E_SUCCESS;
}

/**
 * Remove a service the server's service list
 *
 * @param _service The service to remove
 *
 * @return A HPD error code
 */
int unregister_service_in_unsecure_server( Service *_service )
{
    int rc;
    ServiceElement *_tmp, *_iterator;

    assert(d);

    LL_FOREACH_SAFE(service_head, _iterator, _tmp)
    {
        if( strcmp( _iterator->service->value_url, _service->value_url ) == 0 )
        {
	   LL_DELETE(service_head, _iterator);
	   break;
        }
    }

    return HPD_E_SUCCESS;
}

/**
 * Start the MHD web server and the AVAHI server
 *
 * @param _hostname Hostname for the local address of the server
 *
 * @param _domain_name Domain name for the local address of the server (if NULL = .local)
 *
 * @return A HPD error code
 */
int start_unsecure_server()
{  

    d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG, hpd_daemon->http_port, NULL, NULL, 
                          &answer_to_connection, &done_flag, MHD_OPTION_NOTIFY_COMPLETED, 
                          &request_completed, NULL, MHD_OPTION_END);
    if (NULL == d) 
    {
	return HPD_E_MHD_ERROR;
    }

    return HPD_E_SUCCESS;
}

/**
 * Stop the MHD web server and the AVAHI server and free the associated services
 *
 * @return A HPD error code
 */
int stop_unsecure_server()
{
    assert(d);
    free_unsecure_server_services();

    MHD_stop_daemon (d);

    return HPD_E_SUCCESS;
}

/**
 * Free all the services holded by the server
 *
 * @return A HPD error code
 */
int free_unsecure_server_services()
{
	ServiceElement *_iterator, *_tmp;

	if(service_head == NULL)
		return HPD_E_NULL_POINTER;
	
	LL_FOREACH_SAFE(service_head, _iterator, _tmp)
	{
		LL_DELETE(service_head, _iterator);
		destroy_service_struct(_iterator->service);
		destroy_service_element(_iterator);
		_iterator = NULL;
	}

	return HPD_E_SUCCESS;

}

/**
 * Determines if a given service is registered in the server
 *
 * @param _service The service to check
 *
 * @return 0 If the given service is not registered
 *		   1 If the given service is registered
 */

int is_unsecure_service_registered( Service *_service )
{
    ServiceElement *_iterator = NULL;

    assert(d);

    LL_FOREACH( service_head, _iterator )
    {
        if(   ( strcmp( _iterator->service->device->type, _service->device->type ) == 0 ) 
           && ( strcmp( _iterator->service->device->ID, _service->device->ID ) == 0 )
           && ( strcmp( _iterator->service->type, _service->type ) == 0 )
           && ( strcmp( _iterator->service->ID, _service->ID ) == 0 )            )
        {
	   return 1;
        }
    }

    return 0;
}

/**
 * Looks in the server's service list for a matching service, and returns it.
 *
 * @param _device_type The type of device that owns the service
 *
 * @param _device_ID   The device's ID that own the service_head
 *
 * @param _service_type The type of the service to look for
 *
 * @param _service_ID The service's ID to look for
 *
 * @return Service* if a corresponding service was found
 *		   NULL    otherwise
 */

Service* get_service_from_unsecure_server( char *_device_type, char *_device_ID, char *_service_type, char *_service_ID )
{
    ServiceElement *_iterator = NULL;

    assert(d);

    LL_FOREACH(service_head, _iterator)
    {
        if(   ( strcmp( _iterator->service->device->type, _device_type ) == 0 ) 
           && ( strcmp( _iterator->service->device->ID, _device_ID ) == 0 )
           && ( strcmp( _iterator->service->type, _service_type ) == 0 )
           && ( strcmp( _iterator->service->ID, _service_ID ) == 0 )            )
        {
	   return _iterator->service;
        }
    }

    printf("get_service_from_server : No matching service found\n");

    return NULL;
}

/**
 * Looks in the server's service list for a matching device, and returns it.
 *
 * @param _device_type The type of device to look for
 *
 * @param _device_ID   The device's ID to look for
 *
 * @return Device* if a corresponding device was found
 *		   NULL    otherwise
 */

Device* get_device_from_unsecure_server( char *_device_type, char *_device_ID)
{
    ServiceElement *_iterator = NULL;

    assert(d);

    LL_FOREACH(service_head, _iterator)
    {
        if(   ( strcmp( _iterator->service->device->type, _device_type ) == 0 ) 
           && ( strcmp( _iterator->service->device->ID, _device_ID ) == 0 ))
        {
	   return _iterator->service->device;
        }
    }

    printf("get_device_from_server : No matching service found\n");

    return NULL;
}
