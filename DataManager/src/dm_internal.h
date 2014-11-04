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

#ifndef DM_INTERNAL_H
#define DM_INTERNAL_H

#include "datamanager.h"

/* Function to handle configurations */
int            configurationAddAdapter(Configuration *config, Adapter *adapter);
int            configurationRemoveAdapter(Adapter *adapter );

/* Function to handle adapters */
int          adapterAddDevice    (Adapter *adapter, Device *device);
int          adapterRemoveDevice (Device *device);
mxml_node_t *adapterToXml        (Adapter *adapter, mxml_node_t *parent);
json_t      *adapterToJson       (Adapter *adapter);
int          adapterGenerateId   (Adapter *adapter);

/* Function to handle devices */
mxml_node_t *deviceToXml         (Device *device, mxml_node_t *parent);
json_t      *deviceToJson        (Device *device);
int          deviceAddService    (Device *device, Service *service);
int          deviceRemoveService (Service *service);
int          deviceGenerateId    (Device *device);

/* Function to handle services */
mxml_node_t* serviceToXml       (Service *service, mxml_node_t *parent);
json_t*      serviceToJson      (Service *service);
int          serviceGenerateId  (Service *service);


#endif
