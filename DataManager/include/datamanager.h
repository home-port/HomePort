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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <mxml.h>
#include <jansson.h>

#include "hpd_datamanager.h"

/* Function to handle configurations */
Configuration *configurationNew();
void           configurationFree(Configuration *config);
int            configurationAddListener(Configuration *configuration, Listener *l);
int            configurationRemoveListener(Configuration *configuration, Listener *l);

/* Function to handle adapters */
Adapter     *adapterNew          (Configuration *configuration, const char *network, void *data, free_f free_data);
void         adapterFree         (Adapter *adapter);

/* Function to handle devices */
Device*      deviceNew           (Adapter *adapter, const char *description, const char *vendorId, const char *productId,
                                  const char *version, const char *location, const char *type, void *data, free_f free_data);
void         deviceFree          (Device *device); 

/* Function to handle services */
Service*     serviceNew            (Device *device, const char *description, int isActuator, const char *type, const char *unit,
                                    serviceGetFunction getFunction, servicePutFunction putFunction, Parameter *parameter, void* data, free_f free_data); 
void         serviceFree           (Service *service);
int          serviceGenerateIds    (Service *service);
int          serviceAddListener    (Service *service, Listener *l);
int          serviceRemoveListener (Service *service, Listener *l);

/* Function to handle parameters */
Parameter* parameterNew  (const char *max, const char *min, const char *scale, const char *step,
                          const char *type, const char *unit, const char *values);
void       parameterFree (Parameter *parameter);

/* Find functions - set a parameter to NULL to "skip" it */
Adapter *configurationFindFirstAdapter(Configuration *configuration, const char *id, const char *network);
Device  *adapterFindFirstDevice       (Adapter *adapter, const char *description, const char *id, const char *vendorId,
                                       const char *productId, const char *version, const char *location, const char *type);
Service *deviceFindFirstService(Device *device, const char *description, const int *isActuator, const char *type,
                                const char *unit, const char *id);
Service *configurationServiceLookup(Configuration *configuration, const char *dtype, const char *did, const char *stype, const char *sid);
Service *adapterServiceLookup(Adapter *adapter, const char *dtype, const char *did, const char *stype, const char *sid);

#endif
