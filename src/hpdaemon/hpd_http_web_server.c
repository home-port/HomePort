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
 * @file hpd_http_web_server.c
 * @brief  Methods for managing an unsecure Web Server
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */


#include <assert.h>

#include "hpd_http_web_server.h"
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
 * @param xmlbuff The XML to send
 *
 * @return MHD return value, MHD_NO if the response failed to be created, 
 *		   return code of MHD_queue_response otherwise
 */
static int
send_xml ( struct MHD_Connection *connection, char *xmlbuff )
{

	int ret;
	struct MHD_Response *response;

	if( !xmlbuff )
	{
		printf("The XML that is attempted to send is NULL\n");
		return MHD_NO;
	}

	response = MHD_create_response_from_buffer (strlen(xmlbuff), xmlbuff, MHD_RESPMEM_MUST_FREE);

	if( !response )
	{
		if( xmlbuff )
			free( xmlbuff );

		return MHD_NO;
	}

	MHD_add_response_header ( response, "Content-Type", "text/xml" );
	ret = MHD_queue_response ( connection, MHD_HTTP_OK, response );
	MHD_destroy_response( response );

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
static int 
send_error( struct MHD_Connection *connection, int http_error_code )
{
	int ret;
	struct MHD_Response *response;

	switch( http_error_code )
	{
		case MHD_HTTP_NOT_FOUND :
			response = MHD_create_response_from_data( strlen("Not Found"), (void *) "Not Found", MHD_NO, MHD_NO );
			break;

		case MHD_HTTP_BAD_REQUEST :
			response = MHD_create_response_from_data( strlen("Bad Request"), (void *) "Bad Request", MHD_NO, MHD_NO );
			break;
		case MHD_HTTP_INTERNAL_SERVER_ERROR :
			response = MHD_create_response_from_data( strlen("Internal Server Error"), (void *) "Internal Server Error", MHD_NO, MHD_NO );
			break;
		default :
			response = MHD_create_response_from_data( strlen("Unknown error"), (void *) "Unknown error", MHD_NO, MHD_NO );
			break;
	} 

	if( !response )
		return MHD_NO;
	ret = MHD_queue_response ( connection, http_error_code, response );
	MHD_destroy_response( response );

	return ret;
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
int 
answer_to_connection ( void *cls, struct MHD_Connection *connection, 
                       const char *url, 
                       const char *method, const char *version, 
                       const char *upload_data, 
                       size_t *upload_data_size, void **con_cls		)
{
	Service *requested_service;
	char *xmlbuff = NULL;
	int ret;
	int *done = cls;
	static int aptr;
	const char *get_arg;
	struct sockaddr *addr;
	addr = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
	char IP[16];
	inet_ntop(addr->sa_family,addr->sa_data + 2, IP, 16);

	if( 0 == strcmp (method, MHD_HTTP_METHOD_GET) )
	{

		if ( &aptr != *con_cls )
		{
			/* do never respond on first call */
			*con_cls = &aptr;
			return MHD_YES;
		}
		*con_cls = NULL;


		if( strcmp(url,"/log") == 0 )
		{
			ret = subscribe_to_log_events(connection, con_cls, HPD_IS_UNSECURE_CONNECTION);
			Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
			return ret;
		}
		else if( strcmp(url,"/events") == 0 )
		{
			ret = set_up_server_sent_event_connection( connection, HPD_IS_UNSECURE_CONNECTION );
			if( ret == MHD_HTTP_NOT_FOUND ) 
			{
				Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
				return send_error (connection, MHD_HTTP_NOT_FOUND);
			}
			else 
			{
				Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
				return ret;
			}
		}
		else if( strcmp(url,"/devices") == 0 )
		{
			xmlbuff = get_xml_device_list();
			ret = send_xml (connection, xmlbuff);
			Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
			return ret;
		}
		else if( ( requested_service = matching_service (service_head, url) ) != NULL )
		{
			get_arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "x");
			if( get_arg != NULL )
			{
				if( strcmp(get_arg, "1") == 0 )
				{
					pthread_mutex_lock(requested_service->mutex);
					xmlbuff = extract_service_xml(requested_service);
					pthread_mutex_unlock(requested_service->mutex);
					ret = send_xml (connection, xmlbuff);
					Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, "x=1");
					return ret;
				}
			}

			get_arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "p");
			if( get_arg != NULL )
			{
				if( strcmp(get_arg, "1") == 0 )
				{
					ret = subscribe_to_service(connection, requested_service, con_cls, HPD_IS_UNSECURE_CONNECTION);
					Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, "p=1");
					return ret;
				}
			}

			pthread_mutex_lock( requested_service->mutex );
			ret = requested_service-> get_function(	requested_service, 
			                                        requested_service->get_function_buffer,
			                                        MHD_MAX_BUFFER_SIZE);
			if( ret != 0 )
				requested_service->get_function_buffer[ret] = '\0';
			else
			{
				pthread_mutex_unlock( requested_service->mutex );
				ret = send_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR);
				Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
				return ret;
			}

			xmlbuff = get_xml_value (requested_service->get_function_buffer);
			pthread_mutex_unlock(requested_service->mutex);
			ret = send_xml(connection, xmlbuff);
			Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
			return ret;
		}
		else
		{
			ret = send_error(connection, MHD_HTTP_NOT_FOUND);
			Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
			return ret;
		}
	}

	else if( 0 == strcmp(method, MHD_HTTP_METHOD_PUT) )
	{ 

		if( ( *con_cls ) == NULL )
		{
			if( *upload_data_size == 0 )
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
			if( ( requested_service = matching_service (service_head, url) ) !=NULL )
			{
				pthread_mutex_lock(requested_service->mutex);
				if( requested_service->put_function != NULL && *con_cls != NULL )
				{
					char* _value = get_value_from_xml_value (*con_cls);
					if(_value == NULL)
					{
						pthread_mutex_unlock(requested_service->mutex);
						ret = send_error (connection, MHD_HTTP_BAD_REQUEST);
						Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
						return ret;
					}

					free(*con_cls);
					ret = requested_service-> put_function( requested_service, 
					                                        requested_service->get_function_buffer,
					                                        MHD_MAX_BUFFER_SIZE,
					                                        _value );
					free(_value);

					if( ret != 0 )
						requested_service->get_function_buffer[ret] = '\0';
					else
					{
						pthread_mutex_unlock( requested_service->mutex );
						ret = send_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR);
						Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
						return ret;
					}

					xmlbuff = get_xml_value (requested_service->get_function_buffer);
					pthread_mutex_unlock(requested_service->mutex);
					send_event_of_value_change (requested_service, requested_service->get_function_buffer);
					ret = send_xml (connection, xmlbuff);
					Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
					return ret;
				}
				else
				{
					pthread_mutex_unlock(requested_service->mutex);
					ret = send_error(connection, MHD_HTTP_BAD_REQUEST);
					Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
					return ret;
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
 * @param service_to_register The service to add
 *
 * @return A HPD error code
 */
int 
register_service_in_unsecure_server(Service *service_to_register)
{
	int rc;
	ServiceElement *new_se = NULL;	

	assert(d);

	new_se = create_service_element_struct( service_to_register );
	if( !new_se )
		return HPD_E_NULL_POINTER;

	DL_APPEND( service_head, new_se );
	if( service_head == NULL )
	{
		printf("add_service_to_list failed\n");
		return HPD_E_LIST_ERROR;
	}

	return HPD_E_SUCCESS;
}

/**
 * Remove a service the server's service list
 *
 * @param service_to_unregister The service to remove
 *
 * @return A HPD error code
 */
int 
unregister_service_in_unsecure_server( Service *service_to_unregister )
{
	int rc;
	ServiceElement *tmp, *iterator;

	assert(d);

	DL_FOREACH_SAFE( service_head, iterator, tmp )
	{
		if( strcmp( iterator->service->value_url,service_to_unregister->value_url ) == 0 )
		{
			DL_DELETE( service_head, iterator );
			destroy_service_element_struct( iterator );
			break;
		}
	}

	return HPD_E_SUCCESS;
}

/**
 * Start the MHD web server and the AVAHI server
 *
 * @return A HPD error code
 */
int 
start_unsecure_server()
{  

	d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG, hpd_daemon->http_port, NULL, NULL, 
	                      &answer_to_connection, &done_flag, MHD_OPTION_END);
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
int 
stop_unsecure_server()
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
int 
free_unsecure_server_services()
{
	ServiceElement *iterator, *tmp;

	if( service_head == NULL )
		return HPD_E_NULL_POINTER;

	DL_FOREACH_SAFE( service_head, iterator, tmp )
	{
		DL_DELETE( service_head, iterator );
		destroy_service_struct( iterator->service );
		destroy_service_element_struct( iterator );
		iterator = NULL;
	}

	return HPD_E_SUCCESS;
}

/**
 * Determines if a given service is registered in the server
 *
 * @param service The service to check
 *
 * @return 0 If the given service is not registered
 *		   1 If the given service is registered
 */

int 
is_unsecure_service_registered( Service *service )
{
	ServiceElement *iterator = NULL;

	assert(d);

	DL_FOREACH( service_head, iterator )
	{
		if(   ( strcmp( iterator->service->device->type, service->device->type ) == 0 ) 
		   && ( strcmp( iterator->service->device->ID, service->device->ID ) == 0 )
		   && ( strcmp( iterator->service->type, service->type ) == 0 )
		   && ( strcmp( iterator->service->ID, service->ID ) == 0 )            )
		{
			return 1;
		}
	}

	return 0;
}

/**
 * Looks in the server's service list for a matching service, and returns it.
 *
 * @param device_type The type of device that owns the service
 *
 * @param device_ID   The device's ID that own the service_head
 *
 * @param service_type The type of the service to look for
 *
 * @param service_ID The service's ID to look for
 *
 * @return Service* if a corresponding service was found
 *		   NULL    otherwise
 */

Service* 
get_service_from_unsecure_server( char *device_type, char *device_ID, char *service_type, char *service_ID )
{
	ServiceElement *iterator = NULL;

	assert(d);

	DL_FOREACH(service_head, iterator)
	{
		if(   ( strcmp( iterator->service->device->type, device_type ) == 0 ) 
		   && ( strcmp( iterator->service->device->ID, device_ID ) == 0 )
		   && ( strcmp( iterator->service->type, service_type ) == 0 )
		   && ( strcmp( iterator->service->ID, service_ID ) == 0 )            )
		{
			return iterator->service;
		}
	}

	printf("get_service_from_server : No matching service found\n");

	return NULL;
}

/**
 * Looks in the server's service list for a matching device, and returns it.
 *
 * @param device_type The type of device to look for
 *
 * @param device_ID   The device's ID to look for
 *
 * @return Device* if a corresponding device was found
 *		   NULL    otherwise
 */

Device* 
get_device_from_unsecure_server( char *device_type, char *device_ID )
{
	ServiceElement *iterator = NULL;

	assert(d);

	DL_FOREACH(service_head, iterator)
	{
		if(   ( strcmp( iterator->service->device->type, device_type ) == 0 ) 
		   && ( strcmp( iterator->service->device->ID, device_ID ) == 0 ))
		{
			return iterator->service->device;
		}
	}

	printf("get_device_from_server : No matching service found\n");

	return NULL;
}
