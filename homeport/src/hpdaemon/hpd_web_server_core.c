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
 * @file   hpd_web_server_core.c
 * @brief  Methods for managing the Web Server(s)
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */


#include <assert.h>

#include "hpd_web_server_core.h"
#include "hpd_error.h"
#include "hpd_device_configuration.h"

char *key_pem = NULL;
char *cert_pem = NULL;
char *root_pem = NULL;

/** Flag used for processing HTTP request */
const int continue_flag = 1;

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

	buffer = malloc (sizeof(char)*(size+1));
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
 *	   return code of MHD_queue_response otherwise
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
 *	   return code of MHD_queue_response otherwise
 */
static int 
send_error( struct MHD_Connection *connection, int http_error_code )
{
	int ret;
	struct MHD_Response *response;

	switch( http_error_code )
	{

		case MHD_HTTP_OK :
			response = MHD_create_response_from_data( strlen("OK"), (void *) "OK", MHD_NO, MHD_NO );
			break;

		case MHD_HTTP_NOT_FOUND :
			response = MHD_create_response_from_data(strlen("Not Found"), (void *) "Not Found", MHD_NO, MHD_NO);
			break;

		case MHD_HTTP_BAD_REQUEST :
			response = MHD_create_response_from_data(strlen("Bad Request"), (void *) "Bad Request", MHD_NO, MHD_NO);
			break;

		case MHD_HTTP_UNAUTHORIZED :
			response = MHD_create_response_from_data(strlen("Unauthorized"), (void *) "Unauthorized", MHD_NO, MHD_NO);
			break;

		case MHD_HTTP_SERVICE_UNAVAILABLE :
			response = MHD_create_response_from_data( strlen("Service Unavailable"), (void *) "Service Unavailable", MHD_NO, MHD_NO );
			break;
			
		case MHD_HTTP_INTERNAL_SERVER_ERROR :
			response = MHD_create_response_from_data( strlen("Internal Server Error"), (void *) "Internal Server Error", MHD_NO, MHD_NO );
			break;
			
		case MHD_HTTP_NOT_IMPLEMENTED :
			response = MHD_create_response_from_data( strlen("Not Implemented"), (void *) "Not Implemented", MHD_NO, MHD_NO );
			break;

		default :
			response = MHD_create_response_from_data(strlen("Unknown Error"), (void *) "Unknown Error", MHD_NO, MHD_NO);
			break;
	} 

	if(!response)
		return MHD_NO;
	ret = MHD_queue_response (connection, http_error_code, response);
	MHD_destroy_response(response);

	return ret;
}


#if HPD_HTTPS
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
#endif

/**
 * Process the GET requests
 *	
 * @param connection The client connection 
 *
 * @param url The url on which the client made its request
 *
 * @param arg The argument that the client gave along with the URL (can be NULL)
 *
 * @param con_cls reference to a pointer, initially set to NULL, that this callback can set to some 
 *        address and that will be preserved by MHD for future calls for this request
 *
 * @param web_server The web server struct that store information about security and service list
 *
 * @return MHD_YES to pursue the request handling, MHD_NO in case of error with 
 *         the request, return value of the send_* functions otherwise
 */
static int
http_get_function( 	struct MHD_Connection *connection, 
			char *url, char *arg,
			void **con_cls,
			HPD_web_server_struct *web_server )
{
	int ret;
	char *xmlbuff;
	Service *requested_service = NULL;

	if( strcmp(url,"/log") == 0 )
	{
		return subscribe_to_log_events(connection, con_cls, web_server->is_secure);
	}
	else if(strcmp(url,"/events") == 0)
	{
		ret = set_up_server_sent_event_connection (connection, web_server->is_secure);
		if(ret == MHD_HTTP_NOT_FOUND)
			return send_error (connection, MHD_HTTP_NOT_FOUND);
		else 
			return ret;
	}
	else if(strcmp(url,"/devices") == 0)
	{
		xmlbuff = get_xml_device_list();
	
		return send_xml (connection, xmlbuff);
	}
	else if( ( requested_service = matching_service (web_server->service_head, url) ) != NULL )
	{
		if( arg )
		{
			if(strcmp(arg, "x=1") == 0)
			{
				pthread_mutex_lock(requested_service->mutex);	
				xmlbuff = extract_service_xml(requested_service);
	
				pthread_mutex_unlock(requested_service->mutex);
				return send_xml (connection, xmlbuff);
			}
	
		
			/*Request for the Subscription to the events from the service*/
			if(strcmp(arg, "p=1") == 0)
			{
				return subscribe_to_service(connection, requested_service, 
							    con_cls, web_server->is_secure);
			}
		}

		pthread_mutex_lock(requested_service->mutex);
		ret = requested_service->get_function(	requested_service, 
              						requested_service->get_function_buffer,
               						MHD_MAX_BUFFER_SIZE);
		if( ret != 0 )
			requested_service->get_function_buffer[ret] = '\0';
		else
		{
			ret = send_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR);
			pthread_mutex_unlock( requested_service->mutex );
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

/**
 * Process the PUT requests
 *	
 * @param connection The client connection 
 *
 * @param url The url on which the client made its request
 *
 * @param IP The IP address of the client (used for send event)
 *
 * @param con_cls reference to a pointer, initially set to NULL, that this callback can set to some 
 *        address and that will be preserved by MHD for future calls for this request
 *
 * @param web_server The web server struct that store information about security and service list
 *
 * @return MHD_YES to pursue the request handling, MHD_NO in case of error with 
 *         the request, return value of the send_* functions otherwise
 */
static int
http_put_function(	struct MHD_Connection *connection, 
			char *url, char *IP,
			void **con_cls,
			HPD_web_server_struct *web_server )
{
	int ret;
	char *xmlbuff;
	Service *requested_service = NULL;

	if(strcmp(url, "/configure") == 0)
	{
		pthread_mutex_lock( &web_server->configure_mutex );
		web_server->is_configuring = HPD_YES;
		pthread_mutex_unlock( &web_server->configure_mutex );
		ret = manage_configuration_xml( *con_cls , web_server->service_head );
		if(ret < 0)
		{
			ret = send_error (connection, MHD_HTTP_NOT_FOUND);
			pthread_mutex_lock( &web_server->configure_mutex );
			web_server->is_configuring = HPD_NO;
			pthread_mutex_unlock( &web_server->configure_mutex );
			return ret;
		}
		pthread_mutex_lock( &web_server->configure_mutex );
		web_server->is_configuring = HPD_NO;
		pthread_mutex_unlock( &web_server->configure_mutex );
		xmlbuff = get_xml_device_list();
		ret = send_xml (connection, xmlbuff);
		return ret;
	}
	else if( ( requested_service = matching_service ( web_server->service_head, url ) ) !=NULL )
	{
		pthread_mutex_lock(requested_service->mutex);
		if( requested_service->put_function != NULL && *con_cls != NULL )
		{
			char* value = get_value_from_xml_value ( (char*)*con_cls );
			free(*con_cls);
			if(value == NULL)
			{
				pthread_mutex_unlock( requested_service->mutex );
				return send_error ( connection, MHD_HTTP_BAD_REQUEST );
			}
	
			ret = requested_service->put_function(	requested_service, 
								requested_service->get_function_buffer,
								MHD_MAX_BUFFER_SIZE,
								value );
			free(value);
	
			if( ret != 0 )
				requested_service->get_function_buffer[ret] = '\0';
			else
			{
				ret = send_error(connection, MHD_HTTP_INTERNAL_SERVER_ERROR);
				pthread_mutex_unlock( requested_service->mutex );
				return ret;
			}
	
			xmlbuff = get_xml_value ( requested_service->get_function_buffer );
			pthread_mutex_unlock( requested_service->mutex );
			send_event_of_value_change (	requested_service, 
							requested_service->get_function_buffer, 
							IP );
			ret = send_xml (connection, xmlbuff);
			return ret;
		}
		else
		{
			pthread_mutex_unlock(requested_service->mutex);
			ret = send_error(connection, MHD_HTTP_BAD_REQUEST);
			return ret;
		}
	}
	else
		return send_error (connection, MHD_HTTP_NOT_FOUND);
	
}

/**
 * Callback function used to answer clients's connection (MHD_AccessHandlerCallback)
 *
 * @param cls Custom value selected at callback registration time, used to store
 *	      the web server structure
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
static int 
answer_to_connection (	void *cls, 
			struct MHD_Connection *connection, 
			const char *url, 
			const char *method, 
			const char *version, 
			const char *upload_data, 
			size_t *upload_data_size, 
			void **con_cls)
{
	/* Reject non implemented requests */
	if( 0 != strcmp ( method, MHD_HTTP_METHOD_GET ) 
	 && 0 != strcmp ( method, MHD_HTTP_METHOD_PUT ) )
	{
		return send_error( connection, MHD_HTTP_NOT_IMPLEMENTED );
	}

	HPD_web_server_struct *web_server = (HPD_web_server_struct*) cls;

	pthread_mutex_lock(&web_server->configure_mutex);

	if( web_server->is_configuring == HPD_YES )
	{
		pthread_mutex_unlock( &web_server->configure_mutex );
		return send_error ( connection, MHD_HTTP_SERVICE_UNAVAILABLE );
	}

	pthread_mutex_unlock(&web_server->configure_mutex);

	/* Security check + first call on the callback so return with MHD_YES if OK */
	if( *con_cls == NULL )
	{
#if HPD_HTTPS
		if( web_server->is_secure == HPD_IS_SECURE_CONNECTION )
		{
			gnutls_session_t tls_session = MHD_get_connection_info (connection, 
			                                       MHD_CONNECTION_INFO_GNUTLS_SESSION)->tls_session;
			                                       
			if( !tls_session )
			{
				return MHD_NO;
			}
			
			else if( verify_certificate ( tls_session ) != 0 )
				return send_error ( connection, MHD_HTTP_UNAUTHORIZED );
			else
			{
				*con_cls = &continue_flag;
				return MHD_YES;
			}
		}
		else
		{
			*con_cls = &continue_flag;
			return MHD_YES;
		}
#else
		*con_cls = &continue_flag;
		return MHD_YES;
#endif
	}
	
	struct sockaddr *addr;
	char IP[16];
	addr = MHD_get_connection_info (connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
	inet_ntop(addr->sa_family,addr->sa_data + 2, IP, 16);

	if( 0 == strcmp (method, MHD_HTTP_METHOD_GET) )
	{
		char *arg = NULL;
		
		arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "x");
		
		if( arg )
			arg = "x=1";	
		else
		{
			arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "p");
			
			if( arg )
				arg = "p=1";
		}
		
		Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, arg);
		
		return http_get_function( connection, url, arg, con_cls, web_server );
		
	}

	else if( 0 == strcmp(method, MHD_HTTP_METHOD_PUT) )
	{  
		char *put_data_temp = NULL;
		/* Second call to the callback, so now data should be available */
		if( *con_cls == &continue_flag )
		{
			/* If no data are uploaded at this point, */
			/* consider it as empty request */
			if(*upload_data_size == 0)
			{
				return send_error( connection, MHD_HTTP_BAD_REQUEST);
			}	    
			
			/* Add a space for a '/0' in order to clear the end of the XML */
			put_data_temp = (char*)malloc((*upload_data_size)*sizeof(char)+1);
			
			memcpy(put_data_temp, upload_data, *upload_data_size);
			put_data_temp[*upload_data_size] = '\0';
			*con_cls = put_data_temp;
			*upload_data_size = 0;
			return MHD_YES;
		}
		/* New data has arrived, chunked request, fill the buffer */
		else if( *upload_data_size != 0 )
		{
			put_data_temp = *con_cls;
			int old_size = strlen(put_data_temp);
			put_data_temp = realloc( put_data_temp, old_size + *upload_data_size + 1);
			memcpy( put_data_temp + old_size, upload_data, *upload_data_size );
			put_data_temp[old_size+*upload_data_size] = '\0';
			*con_cls = put_data_temp;
			*upload_data_size = 0;
			return MHD_YES;
		}
		/* All the data have been uploaded, process the request */
		else
		{
			Log (HPD_LOG_ONLY_REQUESTS, NULL, IP, method, url, NULL);
			return http_put_function( connection, url, IP, con_cls, web_server );
		}
	}
	else
	{
		return send_error( connection, MHD_HTTP_NOT_IMPLEMENTED );	
	}
	return MHD_NO;
}

/**
 * Add a service to the web server's service list
 *
 * @param service_to_register The service to add
 *
 * @param web_server The web server struct that store information about security 
 *                   and service list
 *
 * @return A HPD error code
 */
int 
register_service_in_web_server( Service *service_to_register, 
				HPD_web_server_struct *web_server )
{
	int rc;
	ServiceElement *new_se = NULL;

	if( !service_to_register )
		return HPD_E_NULL_POINTER;

	new_se = create_service_element_struct( service_to_register );
	if( !new_se )
		return HPD_E_NULL_POINTER;

	DL_APPEND(web_server->service_head, new_se);
	if(web_server->service_head == NULL)
	{
		printf("register_service_in_web_server\n");
		return HPD_E_LIST_ERROR;
	}

	return HPD_E_SUCCESS;
}

/**
 * Remove a service from the server's service list
 *
 * @param service_to_unregister The service to remove
 *
 * @param web_server The web server struct that store information about security 
 *                   and service list
 *
 * @return A HPD error code
 */
int 
unregister_service_in_web_server( Service *service_to_unregister, 
				  HPD_web_server_struct *web_server )
{
	int rc;
	ServiceElement *tmp, *iterator;
	assert(web_server->daemon);
	
	DL_FOREACH_SAFE(web_server->service_head, iterator, tmp)
	{
		if( strcmp( iterator->service->value_url, service_to_unregister->value_url ) == 0 )
		{
			DL_DELETE(web_server->service_head, iterator);
			destroy_service_element_struct( iterator );
			break;
		}
	}

	return HPD_E_SUCCESS;
}

#if HPD_HTTP
/**
 * Start the MHD web server and the AVAHI server
 *
 * @param unsecure_web_server The web server struct that store information about 
 *                            security and service list
 *
 * @return A HPD error code
 */
int 
start_unsecure_web_server( HPD_web_server_struct *unsecure_web_server )
{  

	unsecure_web_server->daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION, 
							hpd_daemon->http_port, 
							NULL, NULL, 
	                      				&answer_to_connection, 
	                      				unsecure_web_server,
	                      				MHD_OPTION_END);
	if (NULL == unsecure_web_server->daemon) 
	{
		return HPD_E_MHD_ERROR;
	}

	return HPD_E_SUCCESS;
}
#endif


#if HPD_HTTPS
/**
 * Start the MHD secure web server
 *
 * @param secure_web_server The web server struct that store information about 
 *                          security and service list
 *
 * @return A HPD error code
 */
int 
start_secure_web_server( HPD_web_server_struct *secure_web_server )
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

	secure_web_server->daemon = MHD_start_daemon (
					MHD_USE_THREAD_PER_CONNECTION 
					| MHD_USE_SSL, 
					hpd_daemon->https_port, 
					NULL, 
					NULL, 
	                             	&answer_to_connection,
					secure_web_server, 
	                             	MHD_OPTION_HTTPS_MEM_KEY, key_pem,
	                             	MHD_OPTION_HTTPS_MEM_CERT, cert_pem,
	                             	MHD_OPTION_HTTPS_MEM_TRUST, root_pem,
	                            	MHD_OPTION_END);

	if ( NULL == secure_web_server->daemon )
	{
		free(key_pem);
		free(cert_pem);
		free(root_pem);
		return HPD_E_MHD_ERROR;
	}

	return HPD_E_SUCCESS;
}

#endif

/**
 * Stop the MHD web server and the AVAHI server and delete the XML file
 *
 * @param web_server The web server struct that store information about security and service list
 *
 * @return a HPD error code
 */
int 
stop_web_server( HPD_web_server_struct *web_server )
{
	assert(web_server->daemon);

	free_web_server_services( web_server );

	MHD_stop_daemon ( web_server->daemon );
	
	if( web_server->is_secure == HPD_IS_SECURE_CONNECTION )
	{
		if( key_pem )
			free(key_pem);
		if( cert_pem )
			free(cert_pem);
		if( root_pem )
			free(root_pem);
	}

	return HPD_E_SUCCESS;
}

/**
 * Free all the services holded by the server
 *
 * @param web_server The web server struct that store information about security and service list
 *
 * @return A HPD error code
 */
int 
free_web_server_services( HPD_web_server_struct *web_server )
{
	ServiceElement *iterator, *tmp;

	if( web_server->service_head == NULL)
		return HPD_E_NULL_POINTER;

	DL_FOREACH_SAFE(web_server->service_head, iterator, tmp)
	{
		DL_DELETE(web_server->service_head, iterator);
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
 * @param web_server The web server struct that store information about security and service list
 *
 * @return 0 If the given service is not registered
 *	   1 If the given service is already registered
 */

int 
is_service_registered_in_web_server( Service *service, HPD_web_server_struct *web_server )
{
	ServiceElement *iterator = NULL;

	assert(web_server->daemon);

	DL_FOREACH( web_server->service_head, iterator )
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
 * @param web_server The web server struct that store information about security and service list
 *
 * @return Service* if a corresponding service was found
 *	   NULL    otherwise
 */

Service* 
get_service_from_web_server(char *device_type, 
			char *device_ID, 
			char *service_type, 
			char *service_ID,
			HPD_web_server_struct *web_server )
{
	ServiceElement *iterator = NULL;

	assert(web_server->daemon);

	DL_FOREACH(web_server->service_head, iterator)
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
 * @param web_server The web server struct that store information about security and service list
 *
 * @return Device* if a corresponding device was found
 *		   NULL    otherwise
 */

Device* 
get_device_from_web_server( char *device_type, char *device_ID, HPD_web_server_struct *web_server)
{
	ServiceElement *iterator = NULL;

	assert(web_server->daemon);

	DL_FOREACH(web_server->service_head, iterator)
	{
		if(   ( strcmp( iterator->service->device->type, device_type ) == 0 ) 
		   && ( strcmp( iterator->service->device->ID, device_ID ) == 0 ))
		{
			return iterator->service->device;
		}
	}

	return NULL;
}
