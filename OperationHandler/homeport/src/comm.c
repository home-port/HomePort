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

#include "homeport.h"
#include "datamanager.h"
#include "json.h"
#include "xml.h"
#include "libREST.h"
#include "utlist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void homePortGet(Service *service, val_err_cb on_response, void *data)
{
   Request req = { on_response, data };
   service->getFunction(service, req);
}

void homePortSet(Service *service, const char *val, size_t len, val_err_cb on_response, void *data)
{
   Request req = { on_response, data };
   service->putFunction(service, req, val, len);
}

void homePortRespond(Service *service, Request req, ErrorCode code, const char *val, size_t len)
{
   if (req.on_response)
      req.on_response(service, req.data, code, val, len);
}

void homePortChanged(Service *service, const char *val, size_t len)
{
  Listener *l;

  DL_FOREACH(service->listener_head, l)
  {
     l->on_change(service, l->data, val, len);
  }
}

Listener *homePortNewServiceListener(Service *srv, val_cb on_change, void *data, free_f on_free)
{
   Listener *l = malloc(sizeof(Listener));
   l->type = SERVICE_LISTENER;
   l->subscribed = 0;
   l->service = srv;
   l->on_change = on_change;
   l->data = data;
   l->on_free = on_free;
   l->next = NULL;
   l->prev = NULL;
   return l;
}

Listener *homePortNewDeviceListener(HomePort *hp, dev_cb on_attach, dev_cb on_detach, void *data, free_f on_free)
{
   Listener *l = malloc(sizeof(Listener));
   l->type = DEVICE_LISTENER;
   l->subscribed = 0;
   l->homeport = hp;
   l->on_attach = on_attach;
   l->on_detach = on_detach;
   l->data = data;
   l->on_free = on_free;
   l->next = NULL;
   l->prev = NULL;
   return l;
}

void homePortFreeListener(Listener *l)
{
   if (l->subscribed) {
      fprintf(stderr, "Please unsubscribe listener before freeing it\n");
      return;
   }
   if (l->on_free) l->on_free(l->data);
   free(l);
}

void homePortSubscribe(Listener *l)
{
   if (l->subscribed) {
      fprintf(stderr, "Listener already subscribed, ignoring request\n");
      return;
   }

   switch (l->type) {
      case SERVICE_LISTENER:
         serviceAddListener(l->service, l);
         break;
      case DEVICE_LISTENER:
         configurationAddListener(l->homeport->configuration, l);
         break;
   }
   l->subscribed = 1;
}

void homePortUnsubscribe(Listener *l)
{
   if (!l->subscribed) {
      return;
   }

   switch (l->type) {
      case SERVICE_LISTENER:
         serviceRemoveListener(l->service, l);
         break;
      case DEVICE_LISTENER:
         configurationRemoveListener(l->homeport->configuration, l);
         break;
   }
   l->subscribed = 0;
}

void homePortForAllAttachedDevices (Listener *l)
{
   Configuration *c = l->homeport->configuration;
   Adapter *a;
   Device *d;

   if (l->type != DEVICE_LISTENER) {
      fprintf(stderr, "Listener must be a device listener\n");
      return;
   }

   if (!l->on_attach) {
      fprintf(stderr, "Listener does not have an on_attach function, skipping\n");
      return;
   }
   
   DL_FOREACH(c->adapter_head, a) {
      DL_FOREACH(a->device_head, d) {
         if (d->attached) l->on_attach(l->data, d);
      }
   }
}

