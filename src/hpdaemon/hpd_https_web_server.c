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
 * @file hpd_https_web_server.c
 * @brief  Methods for managing a secure Web Server
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */


#include <assert.h>

#include "hpd_https_web_server.h"
#include "hpd_error.h"

char *key_pem;
char *cert_pem;
char *root_pem;

static ServiceElement *secure_service_head;/**< List containing all the services handled by the server */
static struct MHD_Daemon *secure_d;/**< MDH daemon for the MHD web server listening for incoming connections */


int secure_done_flag = 0;
int secure_continue = 1;

/**
 * Get the size of a file 
 *
 * @param filename the name of the file 
 *
 * @return the size of the file, or zero if the file could not be open
 *
*/
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

/**
 * load a file
 *
 * @param filename the name of the file 
 *
 * @return a malloc'd buffer containing the content of the file
 *
*/
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
 * @param xmlbuff The XML to send
 *
 * @return MHD return value, MHD_NO if the response failed to be created, 
 *		   return code of MHD_queue_response otherwise
 */
static int
send_xml ( struct MHD_Connection *connection, const char *xmlbuff )
{

	int ret;
	struct MHD_Response *response;

	response = MHD_create_response_from_buffer (strlen(xmlbuff),xmlbuff, MHD_RESPMEM_MUST_FREE);

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
static int 
send_error( struct MHD_Connection *connection, int http_error_code )
{
	int ret;
	struct MHD_Response *response;

	switch( http_error_code )
	{
		case MHD_HTTP_NOT_FOUND :
			response = MHD_create_response_from_data(strlen(NOT_FOUND), (void *) NOT_FOUND, MHD_NO, MHD_NO);
			break;

		case MHD_HTTP_BAD_REQUEST :
			response = MHD_create_response_from_data(strlen(BAD_REQUEST), (void *) BAD_REQUEST, MHD_NO, MHD_NO);
			break;

		case MHD_HTTP_UNAUTHORIZED :
			response = MHD_create_response_from_data(strlen(UNAUTHORIZED), (void *) UNAUTHORIZED, MHD_NO, MHD_NO);
			break;
			
		case MHD_HTTP_INTERNAL_SERVER_ERROR :
			response = MHD_create_response_from_data(strlen("Internal Server Error"), (void *) "Internal Server Error", MHD_NO, MHD_NO);
			break;
		default :
			response = MHD_create_response_from_data(strlen(UNKNOWN_ERROR), (void *) UNKNOWN_ERROR, MHD_NO, MHD_NO);
			break;
	} 

	if(!response)
		return MHD_NO;
	ret = MHD_queue_response (connection, http_error_code, response);
	MHD_destroy_response(response);

	return ret;
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
int 
secure_answer_to_connection (void *cls, struct MHD_Connection *connection, 
                             					const char *url, 
                             					const char *method, 
								const char *version, 
                             					const char *upload_data, 
                             					size_t *upload_data_size, 
								void **con_cls)
{
	Service *requested_service;
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
			return send_error (connection, MHD_HTTP_UNAUTHORIZED);
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
			if(ret == MHD_HTTP_NOT_FOUND)
				return send_error (connection, MHD_HTTP_NOT_FOUND);
			else 
				return ret;
		}

		else if(strcmp(url,"/devices") == 0)
		{
			Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);

			xmlbuff = get_xml_device_list();

			return send_xml (connection, xmlbuff);
		}
		else if( ( requested_service = matching_service (secure_service_head, url) ) != NULL )
		{
			const char *get_arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "x");
			if(get_arg != NULL)
			{
				if(strcmp(get_arg, "1") == 0)
				{
					Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, "x=1");
					pthread_mutex_lock(requested_service->mutex);	
					xmlbuff = extract_service_xml(requested_service);

					pthread_mutex_unlock(requested_service->mutex);
					return send_xml (connection, xmlbuff);
				}
			}

			/*Request for the Subscription to the events from the service*/
			get_arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "p");
			if(get_arg != NULL)
			{
				if(strcmp(get_arg, "1") == 0)
				{
					Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, "p=1");
					return subscribe_to_service(connection, requested_service, con_cls, HPD_IS_SECURE_CONNECTION);
				}
			}

			Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
			pthread_mutex_lock(requested_service->mutex);

			int ret;
			ret = requested_service->get_function(	requested_service, 
			                                       						requested_service->get_function_buffer,
			                                       						MHD_MAX_BUFFER_SIZE);
			if( ret != 0 )
				requested_service->get_function_buffer[ret] = '\0';
			else
			{
				ret = send_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR);
				pthread_mutex_unlock( requested_service->mutex );
				Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
				return ret;
			}

			xmlbuff = get_xml_value (requested_service->get_function_buffer);
			pthread_mutex_unlock(requested_service->mutex);
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
			if( ( requested_service = matching_service (secure_service_head, url) ) !=NULL )
			{
				Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
				pthread_mutex_lock(requested_service->mutex);
				if( requested_service->put_function != NULL && *con_cls != NULL)
				{
					char* value = get_value_from_xml_value ((char*)*con_cls);
					free(*con_cls);
					if(value == NULL)
					{
						pthread_mutex_unlock(requested_service->mutex);
						return send_error (connection, MHD_HTTP_BAD_REQUEST);
					}

					int ret;
					ret = requested_service->put_function(	requested_service, 
					                                      						requested_service->get_function_buffer,
					                                     						MHD_MAX_BUFFER_SIZE,
					                                       						value);
					free(value);

					if( ret != 0 )
						requested_service->get_function_buffer[ret] = '\0';
					else
					{
						ret = send_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR);
						pthread_mutex_unlock( requested_service->mutex );
						Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
						return ret;
					}

					char* xmlbuff = get_xml_value (requested_service->get_function_buffer);
					pthread_mutex_unlock(requested_service->mutex);
					send_event_of_value_change (requested_service, requested_service->get_function_buffer);
					return send_xml (connection, xmlbuff);
				}
				else
				{
					pthread_mutex_unlock(requested_service->mutex);
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
 * @param service_to_register The service to add
 *
 * @return A HPD error code
 */
int 
register_service_in_secure_server( Service *service_to_register )
{
	int rc;
	ServiceElement *new_se = NULL;

	if( !service_to_register )
		return HPD_E_NULL_POINTER;

	new_se = create_service_element_struct( service_to_register );
	if( !new_se )
		return HPD_E_NULL_POINTER;

	DL_APPEND(secure_service_head, new_se);
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
 * @param service_to_unregister The service to remove
 *
 * @return A HPD error code
 */
int 
unregister_service_in_secure_server( Service *service_to_unregister )
{
	int rc;
	ServiceElement *tmp, *iterator;
	assert(secure_d);
	DL_FOREACH_SAFE(secure_service_head, iterator, tmp)
	{
		if( strcmp( iterator->service->value_url, service_to_unregister->value_url ) == 0 )
		{
			DL_DELETE(secure_service_head, iterator);
			destroy_service_element_struct( iterator );
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
int 
start_secure_server()
{  

	root_pem = load_file(hpd_daemon->root_ca_path);
	key_pem = load_file(hpd_daemon->server_key_path);
	cert_pem = load_file(hpd_daemon->server_cert_path);

	if( root_pem == NULL )
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

	secure_d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL | MHD_USE_DEBUG, 
									hpd_daemon->https_port, 
									NULL, 
									NULL, 
	                             					&secure_answer_to_connection, &secure_done_flag, 
	                             					MHD_OPTION_HTTPS_MEM_KEY, key_pem,
	                             					MHD_OPTION_HTTPS_MEM_CERT, cert_pem,
	                             					MHD_OPTION_HTTPS_MEM_TRUST, root_pem,
	                            					 MHD_OPTION_END);

	if ( NULL == secure_d )
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
int 
stop_secure_server()
{
	assert(secure_d);

	free_secure_server_services();

	MHD_stop_daemon (secure_d);
	if( key_pem )
		free(key_pem);
	if( cert_pem )
		free(cert_pem);
	if( root_pem )
		free(root_pem);

	return HPD_E_SUCCESS;
}

/**
 * Free all the services holded by the server
 *
 * @return A HPD error code
 */
int 
free_secure_server_services()
{
	ServiceElement *iterator, *tmp;

	if(secure_service_head == NULL)
		return HPD_E_NULL_POINTER;

	DL_FOREACH_SAFE(secure_service_head, iterator, tmp)
	{
		DL_DELETE(secure_service_head, iterator);
		destroy_service_struct(iterator->service);
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
 *	   1 If the given service is already registered
 */

int 
is_secure_service_registered( Service *service )
{
	ServiceElement *iterator = NULL;

	assert(secure_d);

	DL_FOREACH( secure_service_head, iterator )
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
 *	   NULL    otherwise
 */

Service* 
get_service_from_secure_server( 	char *device_type, 
									char *device_ID, 
									char *service_type, 
									char *service_ID )
{
	ServiceElement *iterator = NULL;

	assert(secure_d);

	DL_FOREACH(secure_service_head, iterator)
	{
		if(   ( strcmp( iterator->service->device->type, device_type ) == 0 ) 
		   && ( strcmp( iterator->service->device->ID, device_ID ) == 0 )
		   && ( strcmp( iterator->service->type, service_type ) == 0 )
		   && ( strcmp( iterator->service->ID, service_ID ) == 0 )            )
		{
			return iterator->service;
		}
	}

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
get_device_from_secure_server( char *device_type, char *device_ID)
{
	ServiceElement *iterator = NULL;

	assert(secure_d);

	DL_FOREACH(secure_service_head, iterator)
	{
		if(   ( strcmp( iterator->service->device->type, device_type ) == 0 ) 
		   && ( strcmp( iterator->service->device->ID, device_ID ) == 0 ))
		{
			return iterator->service->device;
		}
	}

	return NULL;
}
