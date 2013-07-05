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
 * @file hpd_configure.c
 * @brief  Methods for managing the configuration of a HomePort Daemon
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include "hpd_configure.h"
#include "hpd_error.h"

config_t *cfg = NULL;

/**
 * Allocate ressources to store the hpd_daemon information
 *
 * @return returns A HPD error code
 */
int 
HPD_init_daemon()
{
	hpd_daemon = (HPD_Daemon*)malloc(sizeof(HPD_Daemon));
	if( hpd_daemon == NULL )
		return HPD_E_MALLOC_ERROR;

#if !AVAHI_CLIENT
	hpd_daemon->hostname = NULL;
#endif

#if HPD_HTTP
	hpd_daemon->http_port = 0;
#endif

#if HPD_HTTPS
	hpd_daemon->https_port = 0;
	hpd_daemon->server_cert_path = NULL;
	hpd_daemon->server_key_path = NULL;
	hpd_daemon->root_ca_path = NULL;
#endif
	return HPD_E_SUCCESS;

}


/**
 * Init the HPD configuration from a file
 *
 * @param cfg_file_path The absolute path of the configuration file
 *
 * @return returns A HPD error code
 */
int 
HPD_config_file_init( const char *cfg_file_path )
{
	int max_log_size, log_level;

	if( cfg_file_path == NULL )
		return HPD_E_NULL_POINTER;

	if( hpd_daemon == NULL )
	{
		return HPD_E_CFG_NOT_INIT;
	}
	else
	{
		cfg = (config_t*)malloc(sizeof(config_t));
		if( cfg == NULL )
		{
			free( hpd_daemon );
			return HPD_E_MALLOC_ERROR;
		}
	}


	config_init(cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(cfg, cfg_file_path))
	{
		fprintf(stderr, "Error loading config file%s:%d - %s\n", config_error_file(cfg),
		        config_error_line(cfg), config_error_text(cfg));
		config_destroy(cfg);
		free( hpd_daemon );
		free(cfg);
		return HPD_E_CFG_FILE_ERROR;
	}
#if HPD_HTTPS

	/* Get the path to the certificates and key. */
	if(!config_lookup_string(cfg, "hpdaemon.root_ca_file_path", &hpd_daemon->root_ca_path))
	{
		printf("Path to root CA not found in hpd.cfg\n");
		config_destroy(cfg);
		free(hpd_daemon);
		free(cfg);
		return HPD_E_CFG_FILE_ERROR;
	}

	if(!config_lookup_string(cfg, "hpdaemon.server_key_file_path", &hpd_daemon->server_key_path))
	{
		printf("Path to server key not found in hpd.cfg\n");
		config_destroy(cfg);
		free(hpd_daemon);
		free(cfg);
		return HPD_E_CFG_FILE_ERROR;
	}

	if(!config_lookup_string(cfg, "hpdaemon.server_certificate_file_path", &hpd_daemon->server_cert_path))
	{
		printf("Path to server key not found in hpd.cfg\n");
		config_destroy(cfg);
		free(hpd_daemon);
		free(cfg);
		return HPD_E_CFG_FILE_ERROR;
	}

	if(!config_lookup_int(cfg, "hpdaemon.https_port", &hpd_daemon->https_port))
	{
		printf("SSL port number not found in hpd.cfg\n");
		config_destroy(cfg);
		free(hpd_daemon);
		free(cfg);
		return HPD_E_CFG_FILE_ERROR;
	}	

#endif

#if HPD_HTTP
	if(!config_lookup_int(cfg, "hpdaemon.http_port", &hpd_daemon->http_port))
	{
		printf("HTTP port number not found in hpd.cfg\n");
		config_destroy(cfg);
		free(hpd_daemon);
		free(cfg);
		return HPD_E_CFG_FILE_ERROR;
	}
#endif

	if(!config_lookup_int(cfg, "hpdlog.max_log_size", &max_log_size))
	{
		printf("Max Log size not found in hpd.cfg\n");
		config_destroy(cfg);
		free(hpd_daemon);
		free(cfg);
		return HPD_E_CFG_FILE_ERROR;
	}

	if(!config_lookup_int(cfg, "hpdlog.log_level", &log_level))
	{
		printf("Log level not found in hpd.cfg\n");
		config_destroy(cfg);
		free(hpd_daemon);
		free(cfg);
		return HPD_E_CFG_FILE_ERROR;
	}

	init_hpd_log( max_log_size, log_level);

	return HPD_E_SUCCESS;

}

/**
 * Init the HPD configuration to default values
 *
 * @return returns A HPD error code
 */
int 
HPD_config_default_init()
{
	if(hpd_daemon == NULL)
	{
		return HPD_E_CFG_NOT_INIT;
	}
#if HPD_HTTPS
	hpd_daemon->root_ca_path = "./CA_root_cert.pem";
	hpd_daemon->server_key_path = "./server.key";
	hpd_daemon->server_cert_path = "./server.pem";
	hpd_daemon->https_port = 443;
#endif

#if HPD_HTTP
	hpd_daemon->http_port = 80;
#endif
	init_hpd_log( 2048, 1);

	return HPD_E_SUCCESS;
}

/**
 * Deinit and frees the HPD configuration
 *
 * @return returns A HPD error code
 */
int 
HPD_config_deinit()
{

	destroy_hpd_log();

	if( hpd_daemon == NULL )
		return HPD_E_CFG_NOT_INIT;

	else
	{
		free( hpd_daemon );
		hpd_daemon = NULL;
		if( cfg )
		{
			config_destroy(cfg);
			free( cfg );
		}
		return HPD_E_SUCCESS;
	}
}

/**
 * Set the absolute path of the root CA
 * The configuration needs to be already initialized
 *
 * @param root_ca_path The absolute path of the root CA
 *
 * @return returns A HPD error code
 */
int HPD_config_set_root_ca_path( char *root_ca_path )
{
	if( root_ca_path == NULL )
		return HPD_E_NULL_POINTER;

	if( hpd_daemon == NULL )
		return HPD_E_CFG_NOT_INIT;
#if HPD_HTTPS
	hpd_daemon->root_ca_path = root_ca_path;
#else
	return HPD_E_NO_HTTPS;
#endif
	return HPD_E_SUCCESS;
}

/**
 * Set the absolute path of the server key
 * The configuration needs to be already initialized
 *
 * @param server_key_path The absolute path of the server key file
 *
 * @return returns A HPD error code
 */
int 
HPD_config_set_server_key_path( char *server_key_path )
{
	if( server_key_path == NULL )
		return HPD_E_NULL_POINTER;

	if( hpd_daemon == NULL )
		return HPD_E_CFG_NOT_INIT;	
#if HPD_HTTPS
	hpd_daemon->server_key_path = server_key_path;
#else
	return HPD_E_NO_HTTPS;
#endif
	return HPD_E_SUCCESS;	
}

/**
 * Set the absolute path of the server certificate
 * The configuration needs to be already initialized
 *
 * @param server_cert_path The absolute path of the server certificate file
 *
 * @return returns A HPD error code
 */
int 
HPD_config_set_server_cert_path( char *server_cert_path )
{
	if( server_cert_path == NULL )
		return HPD_E_NULL_POINTER;

	if( hpd_daemon == NULL )
		return HPD_E_CFG_NOT_INIT;
#if HPD_HTTPS
	hpd_daemon->server_cert_path = server_cert_path;
#else
	return HPD_E_NO_HTTPS;
#endif
	return HPD_E_SUCCESS;
}

/**
 * Set the SSL port to use for HTTPS communications
 * The configuration needs to be already initialized
 *
 * @param ssl_port The SSL port to use
 *
 * @return returns A HPD error code
 */
int 
HPD_config_set_ssl_port( int ssl_port )
{
	if( ssl_port <= 0 )
		return HPD_E_BAD_PARAMETER;

	if( hpd_daemon == NULL )
		return HPD_E_CFG_NOT_INIT;

#if HPD_HTTPS
	hpd_daemon->https_port = ssl_port;
#else
	return HPD_E_NO_HTTPS;
#endif
	return HPD_E_SUCCESS;
}

/**
 * Set the port to use for HTTP communications
 * The configuration needs to be already initialized
 *
 * @param port The port to use
 *
 * @return returns A HPD error code
 */
int 
HPD_config_set_port( int port )
{
	if( port <= 0 )
		return HPD_E_BAD_PARAMETER;

	if( hpd_daemon == NULL )
		return HPD_E_CFG_NOT_INIT;
#if HPD_HTTP
	hpd_daemon->http_port = port;
#else
	return HPD_E_NO_HTTP;
#endif
	return HPD_E_SUCCESS;
}

/**
 * Get the absolute path of the root CA
 * The configuration needs to be already initialized
 *
 * @param root_ca_path Pointer to store the returned value
 *
 * @return returns A HPD error code
 */
int 
HPD_config_get_root_ca_path( char **root_ca_path )
{
	if( hpd_daemon == NULL )
		return HPD_E_CFG_NOT_INIT;

	if( root_ca_path == NULL )
		return HPD_E_NULL_POINTER;
#if HPD_HTTPS
	*root_ca_path = hpd_daemon->root_ca_path;
#else
	return HPD_E_NO_HTTPS;
#endif
	return HPD_E_SUCCESS;
}

/**
 * Get the absolute path of the server key file
 * The configuration needs to be already initialized
 *
 * @param server_key_path Pointer to store the returned value
 *
 * @return returns A HPD error code
 */
int 
HPD_config_get_server_key_path( char **server_key_path )
{
	if( hpd_daemon == NULL )
		return HPD_E_CFG_NOT_INIT;

	if( server_key_path == NULL )
		return HPD_E_NULL_POINTER;
#if HPD_HTTPS
	*server_key_path = hpd_daemon->server_key_path;
#else
	return HPD_E_NO_HTTPS;
#endif
	return HPD_E_SUCCESS;	
}

/**
 * Get the absolute path of the server certificate file
 * The configuration needs to be already initialized
 *
 * @param server_cert_path Pointer to store the returned value
 *
 * @return returns A HPD error code
 */
int 
HPD_config_get_server_cert_path( char **server_cert_path )
{
	if( hpd_daemon == NULL )
		return HPD_E_CFG_NOT_INIT;

	if( server_cert_path == NULL )
		return HPD_E_NULL_POINTER;
#if HPD_HTTPS
	*server_cert_path = hpd_daemon->server_cert_path;
#else
	return HPD_E_NO_HTTPS;
#endif
	return HPD_E_SUCCESS;
}

/**
 * Get the SSL port in use
 * The configuration needs to be already initialized
 *
 * @param ssl_port Pointer to store the returned value
 *
 * @return returns A HPD error code
 */
int 
HPD_config_get_ssl_port( int *ssl_port )
{
	if( ssl_port == NULL )
		return HPD_E_NULL_POINTER;

	if( hpd_daemon != NULL )
	{
#if HPD_HTTPS
		*ssl_port = hpd_daemon->https_port;
		return HPD_E_SUCCESS;
#else
		return HPD_E_NO_HTTPS;
#endif

	}

	return HPD_E_CFG_NOT_INIT;
}

/**
 * Get the port in use
 * The configuration needs to be already initialized
 *
 * @param port Pointer to store the returned value
 *
 * @return returns A HPD error code
 */
int 
HPD_config_get_port( int *port )
{
	if( port == NULL )
		return HPD_E_NULL_POINTER;

	if( hpd_daemon != NULL )
	{
#if HPD_HTTP
		*port = hpd_daemon->http_port;
		return HPD_E_SUCCESS;
#else
		return HPD_E_NO_HTTP;
#endif
	}

	return HPD_E_CFG_NOT_INIT;
}

