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
 * @file hpd_avahi_publish_client.h
 * @brief  Methods for managing Avahi publishment using Avahi Client
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef AVAHI_PUBLISH_CLIENT_H
#define AVAHI_PUBLISH_CLIENT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/alternative.h>
#include <avahi-common/error.h>
#include <avahi-common/thread-watch.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include "hpd_services.h"
#include "utlist.h"

/**
 * An Avahi Entry Group Element containing an Entry Group and a Service 
 */
typedef struct EntryGroupElement EntryGroupElement;
struct EntryGroupElement 
{
	AvahiEntryGroup *avahiEntryGroup;/**<The Entry Group*/
	Service *service;/**<The Service*/
	EntryGroupElement *prev;/**<A pointer to the previous EntryGroupElement*/
	EntryGroupElement *next;/**<A pointer to the next EntryGroupElement*/
};


int avahi_client_create_service ( Service *service_to_create );
int avahi_client_start();
int avahi_client_remove_service( Service *service_to_remove );
void avahi_client_quit();

#endif
