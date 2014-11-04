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

struct ev_loop;
struct lr;

typedef struct HomePort HomePort;
typedef struct Configuration Configuration;
typedef struct Adapter Adapter;
typedef struct Device Device;
typedef struct Service Service;
typedef struct Parameter Parameter;

typedef int    (*init_f)             (HomePort *homeport, void *data);
typedef void   (*deinit_f)           (HomePort *homeport, void *data);
typedef void   (*serviceGetFunction) (Service* service, void *request);
typedef size_t (*servicePutFunction) (Service* service, char *buffer, size_t max_buffer_size, char *put_value);

struct HomePort
{
  struct lr *rest_interface;
  Configuration *configuration;
  struct ev_loop *loop;
};

// Homeport Service Control
HomePort  *homePortNew           (struct ev_loop *loop, int port);
void       homePortFree          (HomePort *homeport);
int        homePortStart         (HomePort *homeport);
void       homePortStop          (HomePort *homeport);
int        homePortEasy          (init_f init, deinit_f deinit, void *data, int port);

// Configurator Interface
Adapter   *homePortNewAdapter    (HomePort *homeport, const char *network, void *data);
Device    *homePortNewDevice     (Adapter *adapter, const char *description, const char *vendorId, const char *productId,
                                  const char *version, const char *location, const char *type, void *data);
Service   *homePortNewService    (Device *device, const char *description, int isActuator, const char *type, const char *unit,
                                  serviceGetFunction getFunction, servicePutFunction putFunction, Parameter *parameter, void* data); 
Parameter *homePortNewParameter  (const char *max, const char *min, const char *scale, const char *step,
                                  const char *type, const char *unit, const char *values);
void       homePortFreeAdapter   (Adapter *adapter);
void       homePortFreeDevice    (Device *device); 
void       homePortFreeService   (Service *service);
void       homePortFreeParameter (Parameter *parameter);
int        homePortAttachDevice  (HomePort *homeport, Device *device);
int        homePortDetachDevice  (HomePort *homeport, Device *device);
Adapter   *homePortFindAdapter   (HomePort *homeport, char *adapter_id);
Device    *homePortFindDevice    (Adapter *adapter, char *device_id);
Service   *homePortFindService   (Device *device, char *service_id);

// Communication interface
void       homePortSendState     (Service *service, void *req_in, const char *val, size_t len);

#endif
