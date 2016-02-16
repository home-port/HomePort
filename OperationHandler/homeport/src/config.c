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
#include "hpd_error.h"
#include "utlist.h"

int
homePortAttachDevice( HomePort *homeport, Device *device )
{
    // Pre-checks
    if( homeport == NULL || device == NULL )
        return HPD_E_NULL_POINTER;
    if (device->attached) return HPD_E_DEVICE_ATTACHED;

    device->attached = 1;

    // Inform
    Listener *l;
    DL_FOREACH(homeport->configuration->listener_head, l)
    {
        l->on_attach(l->data, device);
    }
    DL_FOREACH(device->adapter->listener_head, l)
    {
        l->on_attach(l->data, device);
    }

    return HPD_E_SUCCESS;
}

int
homePortDetachDevice( HomePort *homeport, Device *device )
{
    if( homeport == NULL || device == NULL )
        return HPD_E_NULL_POINTER;

    if (!device->attached) return HPD_E_DEVICE_NOT_ATTACHED;
    device->attached = 0;

    Service *iterator;

    DL_FOREACH( device->service_head, iterator )
    {
        // Free listeners
        Listener *l, *tmp;
        DL_FOREACH_SAFE(iterator->listener_head, l, tmp) {
            homePortUnsubscribe(l);
            homePortFreeListener(l);
        }
    }

    // Inform
    Listener *l;
    DL_FOREACH(homeport->configuration->listener_head, l)
    {
        l->on_detach(l->data, device);
    }
    DL_FOREACH(device->adapter->listener_head, l)
    {
        l->on_detach(l->data, device);
    }

    return HPD_E_SUCCESS;
}

int
homePortDetachAllDevices(HomePort *homeport, Adapter *adapter)
{
    if( homeport == NULL || adapter == NULL )
        return HPD_E_NULL_POINTER;

    Device *iterator=NULL;
    DL_FOREACH( adapter->device_head, iterator )
    {
        homePortDetachDevice(homeport, iterator);
    }

    return HPD_E_SUCCESS;
}

int        homePortNewAdapter    (Adapter **adapter, HomePort *homeport, const char *id, const char *network, void *data, free_f free_data)
{
    return adapterNew(adapter, homeport->configuration, id, network, data, free_data);
}

int        homePortNewDevice     (Device **device, Adapter *adapter, const char *id, const char *description, const char *vendorId, const char *productId,
                                  const char *version, const char *location, const char *type, void *data, free_f free_data)
{
    return deviceNew(device, adapter, id, description, vendorId, productId, version, location, type, data, free_data);
}

int        homePortNewService    (Service **service, Device *device, const char *id, const char *description, const char *type, const char *unit,
        serviceGetFunction getFunction, servicePutFunction putFunction, Parameter *parameter, void* data, free_f free_data)
{
    if (device->attached) {
        fprintf(stderr, "Cannot add a service to an attached device. Please detach device first.\n");
        return HPD_E_DEVICE_ATTACHED;
    }
    return serviceNew(service, device, id, description, type, unit, getFunction, putFunction, parameter, data, free_data);
}

Parameter *homePortNewParameter  (const char *max, const char *min, const char *scale, const char *step,
                                  const char *type, const char *unit, const char *values)
{
    return parameterNew(max, min, scale, step, type, unit, values);
}

void       homePortFreeAdapter   (Adapter *adapter)
{
    Device *iterator=NULL;
    DL_FOREACH( adapter->device_head, iterator )
    {
        if (iterator->attached) {
            fprintf(stderr, "Cannot free adapter. Please detach all devices first.\n");
            return;
        }
    }

    Listener *l, *tmp;
    DL_FOREACH_SAFE(adapter->listener_head, l, tmp) {
        homePortUnsubscribe(l);
        homePortFreeListener(l);
    }

    adapterFree(adapter);
}

void       homePortFreeDevice    (Device *device)
{
    if (device->attached) {
        fprintf(stderr, "Cannot free an attached device. Please detach device first.\n");
        return;
    }
    deviceFree(device);
}

void       homePortFreeService   (Service *service)
{
    if (service->device && service->device->attached) {
        fprintf(stderr, "Cannot free a service on an attached device. Please detach device first.\n");
        return;
    }
    serviceFree(service);
}

void       homePortFreeParameter (Parameter *parameter)
{
    parameterFree(parameter);
}

Adapter *homePortFindFirstAdapter (HomePort *homeport, const char *id, const char *network)
{
    return configurationFindFirstAdapter(homeport->configuration, id, network);
}

Device  *homePortFindFirstDevice  (Adapter *adapter, const char *description, const char *id, const char *vendorId,
                                   const char *productId, const char *version, const char *location, const char *type)
{
    return adapterFindFirstDevice(adapter, description, id, vendorId, productId, version, location, type);
}

Service *homePortFindFirstService(Device *device, const char *description, const char *type,
                                  const char *unit, const char *id)
{
    return deviceFindFirstService(device, description, type, unit, id);
}

Service *homePortServiceLookup(HomePort *homePort, const char *aid, const char *did, const char *sid)
{
    return configurationServiceLookup(homePort->configuration, aid, did, sid);
}
