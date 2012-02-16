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
 * @file   hpd_web_server_core.h
 * @brief  Methods for managing the Web Server(s)
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef HPD_WEB_SERVER_CORE
#define HPD_WEB_SERVER_CORE

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
#include "url_trie.h"

typedef struct HPD_web_server_struct HPD_web_server_struct;

struct HPD_web_server_struct
{
   ServiceElement *service_head;
   struct MHD_Daemon *daemon;
   int is_secure;
   int is_configuring;
   pthread_mutex_t configure_mutex;
   UrlTrieElement *url_head;
};

#if HPD_HTTP
int start_unsecure_web_server( HPD_web_server_struct *unsecure_web_server );
#endif

#if HPD_HTTPS
int start_secure_web_server( HPD_web_server_struct *secure_web_server );
#endif

int stop_web_server( HPD_web_server_struct *web_server );

int register_service_in_web_server( Service *service_to_register, 
                                    HPD_web_server_struct *web_server );

int unregister_service_in_web_server( Service *service_to_unregister, 
                                      HPD_web_server_struct *web_server );

Device* get_device_from_web_server( char *device_type, char *device_ID, 
                                    HPD_web_server_struct *web_server);

Service* get_service_from_web_server(char *device_type, char *device_ID, 
                                     char *service_type, char *service_ID,
				                         HPD_web_server_struct *web_server );
				
int is_service_registered_in_web_server( Service *service, 
                                         HPD_web_server_struct *web_server );

int free_web_server_services( HPD_web_server_struct *web_server );

#endif
