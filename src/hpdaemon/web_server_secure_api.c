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

#include "web_server_secure_api.h"
#include "hpd_error.h"

char *key_pem;
char *cert_pem;
char *root_pem;

static ServiceElement *secure_service_head;/**< List containing all the services handled by the server */
static struct MHD_Daemon *secure_d;/**< MDH daemon for the MHD web server listening for incoming connections */


int secure_done_flag = 0;
int secure_continue = 1;


static long
get_file_size (const char *filename)
{
	FILE *fp;

	fp = fopen (filename, "rb");
	if (fp)
	{
		long size;

		if ((0 != fseek (fp, 0, SEEK_END)) || (-1 == (size = ftell (fp))))
			size = 0;

		fclose (fp);

		return size;
	}
	else
		return 0;
}

static char *
load_file (const char *filename)
{
	FILE *fp;
	char *buffer;
	long size;

	size = get_file_size (filename);
	if (size == 0)
		return NULL;

	fp = fopen (filename, "rb");
	if (!fp)
		return NULL;

	buffer = malloc (size);
	if (!buffer)
	{
		fclose (fp);
		return NULL;
	}

	if (size != fread (buffer, 1, size, fp))
	{
		free (buffer);
		buffer = NULL;
	}

	fclose (fp);
	return buffer;
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
send_xml (struct MHD_Connection *connection, const char *xmlbuff)
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

		case MHD_HTTP_UNAUTHORIZED :
			response = MHD_create_response_from_data(strlen("Unauthorized"), (void *) "Unauthorized", MHD_NO, MHD_NO);
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
int secure_request_completed(void *cls, struct MHD_Connection *connection, 
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
 * Verify the client certificate against the root CA.
 *
 * @param session The TLS session to be verified
 *
 * @return A HPD or GNUTLS error code
 */
static int
verify_certificate(gnutls_session_t session)
{
	unsigned int status;
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size;
	int ret;
	gnutls_x509_crt_t cert;
	//const char *hostname;

	if(session == NULL)
	{
		printf("Session NULL\n");
		return HPD_E_NULL_POINTER;
	}

	ret = gnutls_certificate_verify_peers2 (session, &status);
	if (ret < 0)
	{
		printf ("Error\n");
		return GNUTLS_E_CERTIFICATE_ERROR;
	}

	if (status & GNUTLS_CERT_INVALID)
	{
		printf ("The certificate is not trusted.\n");
		return HPD_E_CLIENT_CERT_ERROR;
	}

	if (status & GNUTLS_CERT_SIGNER_NOT_FOUND)
	{
		printf ("The certificate hasn't got a known issuer.\n");
		return HPD_E_CLIENT_CERT_ERROR;
	}

	if (status & GNUTLS_CERT_REVOKED)
	{
		printf ("The certificate has been revoked.\n");
		return HPD_E_CLIENT_CERT_ERROR;
	}

	if (status & GNUTLS_CERT_EXPIRED)
	{
		printf ("The certificate has expired\n");
		return HPD_E_CLIENT_CERT_ERROR;
	}

	if (status & GNUTLS_CERT_NOT_ACTIVATED)
	{
		printf ("The certificate is not yet activated\n");
		return HPD_E_CLIENT_CERT_ERROR;
	}

	if (gnutls_certificate_type_get (session) != GNUTLS_CRT_X509)
		return GNUTLS_E_CERTIFICATE_ERROR;

	if (gnutls_x509_crt_init (&cert) < 0)
	{
		printf ("error in initialization\n");
		return GNUTLS_E_CERTIFICATE_ERROR;
	}

	cert_list = gnutls_certificate_get_peers (session, &cert_list_size);
	if (cert_list == NULL)
	{
		printf ("No certificate was found!\n");
		return GNUTLS_E_CERTIFICATE_ERROR;
	}

	/* FIX : Chack for all certificate, not only first */
	if (gnutls_x509_crt_import (cert, &cert_list[0], GNUTLS_X509_FMT_DER) < 0)
	{
		printf ("error parsing certificate\n");
		return GNUTLS_E_CERTIFICATE_ERROR;
	}


	/*if (!gnutls_x509_crt_check_hostname (cert, hostname))
	{
		printf ("The certificate's owner does not match hostname '%s'\n",
		        hostname);
				return GNUTLS_E_CERTIFICATE_ERROR;
	}*/

	gnutls_x509_crt_deinit (cert);

	return HPD_E_SUCCESS;
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
int secure_answer_to_connection (void *cls, struct MHD_Connection *connection, 
                                 const char *url, 
                                 const char *method, const char *version, 
                                 const char *upload_data, 
                                 size_t *upload_data_size, void **con_cls)
{
	ServiceElement *_requested_service_element;
	int *done = cls;
	struct sockaddr *addr;
	addr = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
	char IP[16];
    	inet_ntop(addr->sa_family,addr->sa_data + 2, IP, 16);

	if( *con_cls == NULL)
	{
		gnutls_session_t tls_session;

		tls_session = MHD_get_connection_info (connection, 
		                                       MHD_CONNECTION_INFO_GNUTLS_SESSION)->tls_session;
		if( verify_certificate (tls_session) != 0 )
		{
			return send_error (connection, MHD_HTTP_UNAUTHORIZED);
		}
		else
		{
			*con_cls = &secure_continue;
			return MHD_YES;
		}
	}

	if( 0 == strcmp (method, MHD_HTTP_METHOD_GET) )
	{

		char *xmlbuff;
		*con_cls = NULL;

	if(strcmp(url,"/log") == 0)
	{
	    Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
	    return subscribe_to_log_events(connection, con_cls, HPD_IS_SECURE_CONNECTION);
	}
        else if(strcmp(url,"/events") == 0)
        {
	   Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
	   int ret = set_up_server_sent_event_connection (connection, HPD_IS_SECURE_CONNECTION);
	   if(ret == MHD_HTTP_NOT_FOUND) return send_error (connection, MHD_HTTP_NOT_FOUND);
	   else return ret;
        }

	else if(strcmp(url,"/devices") == 0)
	{
		Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
		
		xmlbuff = get_xml_device_list();

		return send_xml (connection, xmlbuff);
	}
	else if( ( _requested_service_element = matching_service (secure_service_head, url) ) != NULL )
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
		  return subscribe_to_service(connection, _requested_service_element, con_cls, HPD_IS_SECURE_CONNECTION);
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
		if( *con_cls == &secure_continue )
		{
			if(*upload_data_size == 0)
			{
				return MHD_YES; /* not ready yet */
			}	    
			*done = 1;
			/* Add a space for a '/0' in order to clear the end of the XML */
			char *secure_put_data_temp = (char*)malloc((*upload_data_size)*sizeof(char)+1);
			memcpy(secure_put_data_temp, upload_data, *upload_data_size);
			secure_put_data_temp[*upload_data_size]='\0';
			*con_cls = secure_put_data_temp;
			*upload_data_size = 0;
			return MHD_YES;
		}
		else
		{
			if( ( _requested_service_element = matching_service (secure_service_head, url) ) !=NULL )
			{
				Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
				pthread_mutex_lock(_requested_service_element->mutex);
				if( _requested_service_element->service->put_function != NULL && *con_cls != NULL)
				{
					char* _value = get_value_from_xml_value ((char*)*con_cls);
					free(*con_cls);
		  			if(_value == NULL)
					  {
					      pthread_mutex_unlock(_requested_service_element->mutex);
					      return send_error (connection, MHD_HTTP_BAD_REQUEST);
					  }
					  
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
					return send_error (connection, MHD_HTTP_BAD_REQUEST);
				}
			}
			else
				return send_error (connection, MHD_HTTP_NOT_FOUND);
		}
	}
	return MHD_NO;
}

/**
 * Add a service to the server's service list
 *
 * @param _service The service to add
 *
 * @return A HPD error code
 */
int register_service_in_secure_server(Service *_service)
{
	int rc;

	ServiceElement *_service_element_to_add = create_service_element (_service);

	LL_APPEND(secure_service_head, _service_element_to_add);
	if(secure_service_head == NULL)
	{
		printf("add_service_to_list failed\n");
		return HPD_E_LIST_ERROR;
	}

	return HPD_E_SUCCESS;
}

/**
 * Remove a service from the server's service list
 *
 * @param _service The service to remove
 *
 * @return A HPD error code
 */
int unregister_service_in_secure_server( Service *_service )
{
	int rc;
	ServiceElement *_tmp, *_iterator;
	assert(secure_d);
	LL_FOREACH_SAFE(secure_service_head, _iterator, _tmp)
	{
		if( strcmp( _iterator->service->value_url, _service->value_url ) == 0 )
		{
			LL_DELETE(secure_service_head, _iterator);
			destroy_service_element(_iterator);
			break;
		}
	}

	return HPD_E_SUCCESS;
}

/**
 * Start the MHD secure web server
 *
 * @return A HPD error code
 */
int start_secure_server()
{  

	root_pem = load_file(hpd_daemon->root_ca_path);
	key_pem = load_file(hpd_daemon->server_key_path);
	cert_pem = load_file(hpd_daemon->server_cert_path);

	if( root_pem == NULL)
	{
		printf("Error loading root CA\n");
		if( key_pem )
			free(key_pem);
		if( cert_pem )
			free( cert_pem );
		return HPD_E_SSL_CERT_ERROR;
	}

	if( key_pem == NULL )
	{
		printf("Error loading server key\n");
		if( root_pem )
			free( root_pem );
		if( cert_pem )
			free( cert_pem );
		return HPD_E_SSL_CERT_ERROR;
	}	

	if( cert_pem == NULL )
	{
		printf("Error loading server certificate\n");
		if( root_pem )
			free( root_pem );
		if( key_pem )
			free( key_pem );
		return HPD_E_SSL_CERT_ERROR;
	}

	secure_d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL | MHD_USE_DEBUG, hpd_daemon->https_port, NULL, NULL, 
	                             &secure_answer_to_connection, &secure_done_flag, 
	                             MHD_OPTION_NOTIFY_COMPLETED, &secure_request_completed, NULL,
	                             MHD_OPTION_HTTPS_MEM_KEY, key_pem,
	                             MHD_OPTION_HTTPS_MEM_CERT, cert_pem,
	                             MHD_OPTION_HTTPS_MEM_TRUST, root_pem,
	                             MHD_OPTION_END);

	if (NULL == secure_d)
	{
		free(key_pem);
		free(cert_pem);
		free(root_pem);
		return HPD_E_MHD_ERROR;
	}

	return HPD_E_SUCCESS;
}

/**
 * Stop the MHD web server and the AVAHI server and delete the XML file
 *
 * @return a HPD error code
 */
int stop_secure_server()
{
	assert(secure_d);

	free_secure_server_services();
	
	MHD_stop_daemon (secure_d);
	if(key_pem)
		free(key_pem);
	if(cert_pem)
		free(cert_pem);
	if(root_pem)
		free(root_pem);

	return HPD_E_SUCCESS;
}

/**
 * Free all the services holded by the server
 *
 * @return A HPD error code
 */
int free_secure_server_services()
{
	ServiceElement *_iterator, *_tmp;

	if(secure_service_head == NULL)
		return HPD_E_NULL_POINTER;
	
	LL_FOREACH_SAFE(secure_service_head, _iterator, _tmp)
	{
		LL_DELETE(secure_service_head, _iterator);
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
 *	   1 If the given service is already registered
 */

int is_secure_service_registered( Service *_service )
{
	ServiceElement *_iterator = NULL;

	assert(secure_d);

	LL_FOREACH( secure_service_head, _iterator )
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
 *	   NULL    otherwise
 */

Service* get_service_from_secure_server( char *_device_type, char *_device_ID, char *_service_type, char *_service_ID )
{
	ServiceElement *_iterator = NULL;

	assert(secure_d);

	LL_FOREACH(secure_service_head, _iterator)
	{
		if(   ( strcmp( _iterator->service->device->type, _device_type ) == 0 ) 
		   && ( strcmp( _iterator->service->device->ID, _device_ID ) == 0 )
		   && ( strcmp( _iterator->service->type, _service_type ) == 0 )
		   && ( strcmp( _iterator->service->ID, _service_ID ) == 0 )            )
		{
			return _iterator->service;
		}
	}

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

Device* get_device_from_secure_server( char *_device_type, char *_device_ID)
{
	ServiceElement *_iterator = NULL;

	assert(secure_d);

	LL_FOREACH(secure_service_head, _iterator)
	{
		if(   ( strcmp( _iterator->service->device->type, _device_type ) == 0 ) 
		   && ( strcmp( _iterator->service->device->ID, _device_ID ) == 0 ))
		{
			return _iterator->service->device;
		}
	}

	return NULL;
}
