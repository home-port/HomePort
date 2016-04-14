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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

/* Function to handle configurations */
configuration_t *configurationNew();
void           configurationFree(configuration_t *config);
int            configurationAddListener(configuration_t *configuration, listener_t *l);
int            configurationRemoveListener(configuration_t *configuration, listener_t *l);
int            configurationAddAdapter(configuration_t *config, adapter_t *adapter);
int            configurationRemoveAdapter(adapter_t *adapter );

/* Function to handle adapters */
int          adapterNew            (adapter_t **adapter, configuration_t *configuration, const char *id, const char *network, void *data, free_f free_data);
void         adapterFree           (adapter_t *adapter);
int          adapterAddListener    (adapter_t *adapter, listener_t *l);
int          adapterRemoveListener (adapter_t *adapter, listener_t *l);
int          adapterAddDevice    (adapter_t *adapter, device_t *device);
int          adapterRemoveDevice (device_t *device);

/* Function to handle devices */
int          deviceNew           (device_t** device, adapter_t *adapter, const char *id, const char *description, const char *vendorId, const char *productId,
                                  const char *version, const char *location, const char *type, void *data, free_f free_data);
void         deviceFree          (device_t *device);
int          deviceAddService    (device_t *device, service_t *service);
int          deviceRemoveService (service_t *service);

/* Function to handle services */
int          serviceNew            (service_t **service, device_t *device, const char *id, const char *description, const char *type, const char *unit,
                                    action_f getFunction, action_f putFunction, parameter_t *parameter, void* data, free_f free_data);
void         serviceFree           (service_t *service);
int          serviceAddListener    (service_t *service, listener_t *l);
int          serviceRemoveListener (service_t *service, listener_t *l);

/* Function to handle parameters */
parameter_t* parameterNew  (const char *max, const char *min, const char *scale, const char *step,
                          const char *type, const char *unit, const char *values);
void       parameterFree (parameter_t *parameter);

/* Find functions - set a parameter to NULL to "skip" it */
adapter_t *configurationFindFirstAdapter(configuration_t *configuration, const char *id, const char *network);
device_t  *adapterFindFirstDevice       (adapter_t *adapter, const char *description, const char *id, const char *vendorId,
                                       const char *productId, const char *version, const char *location, const char *type);
service_t *deviceFindFirstService(device_t *device, const char *description, const char *type,
                                const char *unit, const char *id);
service_t *configurationServiceLookup(configuration_t *configuration, const char *aid, const char *did, const char *sid);
service_t *adapterServiceLookup(adapter_t *adapter, const char *did, const char *sid);

// Find functions shortcuts
#define configurationFindAdapter(_HP, _ID) configurationFindFirstAdapter(_HP, _ID, NULL)
#define adapterFindDevice(_A, _ID) adapterFindFirstDevice(_A, NULL, _ID, NULL, NULL, NULL, NULL, NULL)
#define deviceFindService(_D, _ID) deviceFindFirstService(_D, NULL, NULL, NULL, _ID)

#endif
