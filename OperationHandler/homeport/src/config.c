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
#include "lr_callbacks.h"
#include "hpd_error.h"
#include "hpd_configuration.h"
#include "hpd_adapter.h"
#include "hpd_device.h"
#include "hpd_service.h"
#include "libREST.h"
#include "utlist.h"

static int
registerService(HomePort *homeport, Service *service)
{
   char *uri = service->uri;
  int rc;
  Service *s = lr_lookup_service(homeport->rest_interface, uri);
  if (s) {
    printf("A similar service is already registered in the unsecure server\n");
    return HPD_E_SERVICE_ALREADY_REGISTER;
  }

  printf("Registering service\n");
  rc = lr_register_service(homeport->rest_interface,
      uri,
      lrcb_getState, NULL, lrcb_setState, NULL,
      NULL, service);
  if(rc) {
    printf("Failed to register non secure service\n");
    return HPD_E_MHD_ERROR;
  }

  return HPD_E_SUCCESS;
}

static int
unregisterService( HomePort *homeport, char* uri )
{
  Service *s = lr_lookup_service(homeport->rest_interface, uri);
  if( s == NULL )
    return HPD_E_SERVICE_NOT_REGISTER;

  s = lr_unregister_service ( homeport->rest_interface, uri );
  if( s == NULL )
    return HPD_E_MHD_ERROR;

  return HPD_E_SUCCESS;
}


int
homePortAddAdapter(HomePort *homeport, Adapter *adapter)
{
  return configurationAddAdapter(homeport->configuration, adapter);
}

int
homePortRemoveAdapter(HomePort *homeport, Adapter *adapter)
{
  return configurationRemoveAdapter(homeport->configuration, adapter);
}

int 
homePortAttachDevice( HomePort *homeport, Adapter *adapter, Device *device )
{
  if( homeport == NULL || adapter == NULL || device == NULL )
  {
    return HPD_E_NULL_POINTER;
  }
  Service *iterator;
  int rc;

  if( configurationFindAdapter(homeport->configuration, adapter->id) == NULL )
  {
    printf("Adapter not in the systen\n");
    return -1;
  }

  if( ( rc = adapterAddDevice(adapter, device) ) )
  {
    return rc;
  }

  DL_FOREACH( device->service_head, iterator )
  {
    int stat = serviceGenerateUri(iterator);
    if (stat != HPD_E_SUCCESS) return stat;

    rc = registerService(homeport, iterator );
    if( rc < HPD_E_SUCCESS )
    {
      return rc;
    }
  }

  return HPD_E_SUCCESS;
}

int 
homePortDetachDevice( HomePort *homeport, Device *device )
{
  if( homeport == NULL || device == NULL )
    return HPD_E_NULL_POINTER;

  Service *iterator;
  int rc;

  DL_FOREACH( device->service_head, iterator )
  {
    rc = unregisterService( homeport, iterator->uri );
    if(rc < HPD_E_SUCCESS)
    {
      return rc;
    }
  }

  adapterRemoveDevice(device);

  return HPD_E_SUCCESS;
}


