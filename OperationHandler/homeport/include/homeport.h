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

#ifndef HOMEPORT_H
#define HOMEPORT_H

#include <stddef.h>

#include "hpd_datamanager.h"

struct ev_loop;
struct lr;

// TODO Get rid of all this const weirdness !

typedef struct HomePort HomePort;

typedef int  (*init_f)             (HomePort *homeport, void *data);
typedef void (*deinit_f)           (HomePort *homeport, void *data);

struct HomePort
{
   Configuration *configuration;
   struct ev_loop *loop;
};

// Homeport Service Control
HomePort  *homePortNew           (struct ev_loop *loop);
void       homePortFree          (HomePort *homeport);
int        homePortStart         (HomePort *homeport);
void       homePortStop          (HomePort *homeport);
int        homePortEasy          (init_f init, deinit_f deinit, void *data);

// Configurator Interface
Adapter   *homePortNewAdapter    (HomePort *homeport, const char *network, void *data, free_f free_data);
Device    *homePortNewDevice     (Adapter *adapter, const char *description, const char *vendorId, const char *productId,
                                  const char *version, const char *location, const char *type, void *data, free_f free_data);
Service   *homePortNewService    (Device *device, const char *description, int isActuator, const char *type, const char *unit,
                                  serviceGetFunction getFunction, servicePutFunction putFunction, Parameter *parameter, void* data, free_f free_data); 
Parameter *homePortNewParameter  (const char *max, const char *min, const char *scale, const char *step,
                                       const char *type, const char *unit, const char *values);
void       homePortFreeAdapter      (Adapter *adapter);
void       homePortFreeDevice       (Device *device); 
void       homePortFreeService      (Service *service);
void       homePortFreeParameter    (Parameter *parameter);
int        homePortAttachDevice     (HomePort *homeport, Device *device);
int        homePortDetachAllDevices (HomePort *homeport, Adapter *adapter);
int        homePortDetachDevice     (HomePort *homeport, Device *device);

// Find functions -- set parameter to NULL to "skip" it
#define homePortFindAdapter(_HP, _ID) homePortFindFirstAdapter(_HP, _ID, NULL)
#define homePortFindDevice(_A, _ID) homePortFindFirstDevice(_A, NULL, _ID, NULL, NULL, NULL, NULL, NULL)
#define homePortFindService(_D, _ID) homePortFindFirstAdapter(_D, NULL, NULL, NULL, NULL, _ID, NULL)
Adapter *homePortFindFirstAdapter (HomePort *homeport, const char *id, const char *network);
Device  *homePortFindFirstDevice  (Adapter *adapter, const char *description, const char *id, const char *vendorId,
                                   const char *productId, const char *version, const char *location, const char *type);
Service *homePortFindFirstService (Device *device, const char *description, const int  *isActuator, const char *type,
                                   const char *unit, const char *id, const char *uri);
Service *homePortServiceLookup(HomePort *homePort, const char *dtype, const char *did, const char *stype, const char *sid);

// Communication interface
void      homePortRespond               (Service *service, Request req, ErrorCode code, const char *val, size_t len);
void      homePortChanged               (Service *service, const char *val, size_t len);
void      homePortGet                   (Service *service, val_err_cb on_response, void *data);
void      homePortSet                   (Service *service, const char *val, size_t len, val_err_cb on_response, void *data);
Listener *homePortNewServiceListener    (Service *srv, val_cb on_change, void *data, free_f on_free);
Listener *homePortNewDeviceListener     (HomePort *hp, dev_cb on_attach, dev_cb on_detach, void *data, free_f on_free);
void      homePortFreeListener          (Listener *l);
void      homePortSubscribe             (Listener *l);
void      homePortUnsubscribe           (Listener *l);
void      homePortForAllAttachedDevices (Listener *l);

#endif