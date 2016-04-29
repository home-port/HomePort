/*
 * Copyright 2011 Aalborg University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVidED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 */

#include "event.h"
#include "old_model.h"

hpd_error_t event_free_listener(hpd_listener_t *listener)
{
    if (listener->on_free) listener->on_free(listener->data);
    free(listener);
    return HPD_E_SUCCESS;
}

hpd_error_t event_inform_adapter_attached(hpd_adapter_t *adapter)
{
    // TODO Send it over the event loop
    // TODO Remember adapter may die after this function !!!
//    ADAPTER_INFORM(adapter, on_attach);
    return HPD_E_SUCCESS;
}

hpd_error_t event_inform_adapter_detached(hpd_adapter_t *adapter)
{
    // TODO Send it over the event loop
    // TODO Remember adapter may die after this function !!!
//    ADAPTER_INFORM(adapter, on_detach);
    return HPD_E_SUCCESS;
}

hpd_error_t event_inform_device_attached(hpd_device_t *device)
{
    // TODO Send it over the event loop
    // TODO Remember adapter may die after this function !!!
//    DEVICE_INFORM(device, on_attach);
    return HPD_E_SUCCESS;
}

hpd_error_t event_inform_device_detached(hpd_device_t *device)
{
    // TODO Send it over the event loop
    // TODO Remember adapter may die after this function !!!
//    DEVICE_INFORM(device, on_detach);
    return HPD_E_SUCCESS;
}
