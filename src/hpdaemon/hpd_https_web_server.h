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
 * @file hpd_https_web_server.h
 * @brief  Methods for managing a secure Web Server
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef WEB_SERVER_SECURE_API_H
#define WEB_SERVER_SECURE_API_H

#define	XML_FILE_NAME "services.xml"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include "hpd_services.h"
#include "utlist.h"
#include "hpd_configure.h"
#include "hpd_server_sent_events.h"
#include "hpd_xml.h"

#define NOT_FOUND "<html><head><title>Not Found</title></head></html>"
#define BAD_REQUEST "<html><head><title>Bad Request</title></head></html>"
#define UNAUTHORIZED "<html><head><title>Unauthorized</title></head></html>"
#define UNKNOWN_ERROR "<html><head><title>Unknown Error</title></head></html>"

int start_secure_server();
int stop_secure_server();
int register_service_in_secure_server( Service *service_to_register );
int unregister_service_in_secure_server( Service *service_to_unregister );

int is_secure_service_registered( Service *service );
int free_secure_server_services();

Service* get_service_from_secure_server( 	char *device_type, 
											char *device_ID, 
											char *service_type, 
											char *service_ID );

Device* get_device_from_secure_server( char *device_type, char *device_ID );


#endif /* WEB_SERVER_SECURE_API_H */

