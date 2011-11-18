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
 * @file hpd_web_server.h
 * @brief  Methods for managing the Web Server
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef WEB_SERVER_API_H
#define WEB_SERVER_API_H

#include "hpd_services.h"
#include "hpd_xml.h"
#include "utlist.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if HPD_HTTP
	#include "hpd_http_web_server.h"
#endif

#if HPD_HTTPS
	#include "hpd_https_web_server.h"
#endif


int start_server(char* hostname, char *domain_name);
int stop_server();
int register_service_in_server( Service *service_to_register );
int unregister_service_in_server( Service *service_to_unregister );
int register_device_services( Device *device_to_register );
int unregister_device_services( Device *device_to_unregister );

int is_service_registered( Service *service );

Service* get_service_from_server( char *device_type, char *device_ID, char *service_type, char *service_ID );
Device* get_device_from_server( char *device_type, char *device_ID);

#endif
