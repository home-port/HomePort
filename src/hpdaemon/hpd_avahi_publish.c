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
 * @file hpd_avahi_publish.c
 * @brief  Methods for managing Avahi publishment
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */


#include "hpd_avahi_publish.h"
#include "hpd_error.h"
/**
 * Start either an Avahi server either an Avahi client. If starting an Avahi
 * client, the device needs to have an Avahi daemon running.
 *
 * @param host_name If starting a server, set its host name
 *
 * @param domain_name If starting a server, set its domain name. .local if NULL
 *
 * @return 
 *
 */
int 
avahi_start ( char* host_name, char* domain_name )
{

#if AVAHI_CLIENT
	return avahi_client_start ();
#else
	return avahi_core_start( host_name, domain_name );
#endif
}

/**
 * Stops either an Avahi server either an Avahi client. If starting an Avahi
 * client, the device needs to have an Avahi daemon running.
 *
 * @return 
 *
 */
void 
avahi_quit ()
{
#if AVAHI_CLIENT
	avahi_client_quit ();
#else
	avahi_core_quit();
#endif
}

/**
 * Creates an Avahi service given a Service structure
 *
 * @param service_to_create the service to create
 *
 * @return 
 *
 */
void 
avahi_create_service ( Service * service_to_create )
{
#if AVAHI_CLIENT
	avahi_client_create_service ( service_to_create );
#else
	avahi_core_create_service( service_to_create );
#endif
}

/**
 * Removes a service from Avahi given a Service structure
 *
 * @param service_to_remove the service to remove
 *
 * @return 0 if successful -1 if failed
 *
 */
int 
avahi_remove_service( Service * service_to_remove )
{
#if AVAHI_CLIENT
	return avahi_client_remove_service ( service_to_remove );
#else
	return avahi_core_remove_service( service_to_remove );
#endif
}
