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
 * @file hpd_server_sent_events.h
 * @brief  Methods for managing the Server Sent Events part of the Web Server
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */


#ifndef SERVER_SENT_EVENTS_H
#define SERVER_SENT_EVENTS_H

#include "hpd_web_server_interface.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <uuid/uuid.h>
#include <ev.h>

struct event_socket {
   char *url;
   void *req;
   struct ev_loop *loop;
   struct ev_timer timeout_watcher;
   struct event_socket *next;
   struct event_socket *prev;
};

void destroy_socket(struct event_socket *socket);
struct event_socket *subscribe_to_events(const char *body, struct
      ev_loop *loop);

void open_event_socket(struct event_socket *socket,
                       void *req);
void close_event_socket(struct event_socket *socket);

int notify_service_availability(Service* service_to_notify, int availability);
int send_event_of_value_change (Service *service, const char
      *updated_value);
int send_log_event(char *log_message);

#endif
