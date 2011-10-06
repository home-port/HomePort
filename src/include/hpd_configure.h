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

#ifndef HPD_CONFIGURE_H
#define HPD_CONFIGURE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct HPD_Daemon HPD_Daemon;
struct HPD_Daemon
{

#if !AVAHI_CLIENT
	char *hostname;
#endif

#if HPD_HTTP
	int http_port;
#endif

#if HPD_HTTPS
	int https_port;
	char *server_cert_path;
	char *server_key_path;
	char *root_ca_path;
#endif

};


HPD_Daemon *hpd_daemon;

int HPD_init_daemon();

int HPD_config_file_init( char *_cfg_file_path );
int HPD_config_default_init();
int HPD_config_set_root_ca_path( char *_root_ca_path);
int HPD_config_set_server_key_path( char *_server_key_path);
int HPD_config_set_server_cert_path( char *_server_cert_path);
int HPD_config_set_ssl_port( int _ssl_port );
int HPD_config_set_port( int _port );
int HPD_config_get_root_ca_path( char **_root_ca_path);
int HPD_config_get_server_key_path( char **_server_key_path );
int HPD_config_get_server_cert_path( char **_server_cert_path );
int HPD_config_get_ssl_port( int *_ssl_port );
int HPD_config_get_port( int *_port );



#endif
