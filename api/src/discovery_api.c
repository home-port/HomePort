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

#include <daemon.h>
#include <old_model.h>
#include "hpd_shared_api.h"
#include "discovery.h"

hpd_error_t hpd_adapter_id_alloc(hpd_adapter_id_t **id, hpd_t *hpd, const char *aid)
{
    if (!id || !hpd || !aid) return HPD_E_NULL;
    return discovery_alloc_aid(id, hpd, aid);
}

hpd_error_t hpd_adapter_id_free(hpd_adapter_id_t *id)
{
    if (!id) return HPD_E_NULL;
    return discovery_free_aid(id);
}

hpd_error_t hpd_device_id_alloc(hpd_device_id_t **id, hpd_t *hpd, const char *aid, const char *did)
{
    if (!id || !hpd || !aid || !did) return HPD_E_NULL;
    return discovery_alloc_did(id, hpd, aid, did);
}

hpd_error_t hpd_device_id_free(hpd_device_id_t *id)
{
    if (!id) return HPD_E_NULL;
    return discovery_free_did(id);
}

hpd_error_t hpd_service_id_alloc(hpd_service_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid)
{
    if (!id || !hpd || !aid || !did || !sid) return HPD_E_NULL;
    return discovery_alloc_sid(id, hpd, aid, did, sid);
}

hpd_error_t hpd_service_id_free(hpd_service_id_t *id)
{
    if (!id) return HPD_E_NULL;
    return discovery_free_sid(id);
}

hpd_error_t hpd_parameter_id_alloc(hpd_parameter_id_t **id, hpd_t *hpd, const char *aid, const char *did,
                                   const char *sid, const char *pid)
{
    if (!id || !hpd || !aid || !did || !sid || !pid) return HPD_E_NULL;
    return discovery_alloc_pid(id, hpd, aid, did, sid, pid);
}

hpd_error_t hpd_parameter_id_free(hpd_parameter_id_t *id)
{
    if (!id) return HPD_E_NULL;
    return discovery_free_pid(id);
}

hpd_error_t hpd_adapter_alloc(hpd_adapter_t **adapter, const char *id)
{
    if (!adapter || !id) return HPD_E_NULL;
    return discovery_alloc_adapter(adapter, id);
}

hpd_error_t hpd_adapter_free(hpd_adapter_t *adapter)
{
    if (!adapter) return HPD_E_NULL;
    return discovery_free_adapter(adapter);
}

hpd_error_t hpd_adapter_attach(hpd_t *hpd, hpd_adapter_t *adapter)
{
    if (!hpd || !adapter) return HPD_E_NULL;
    if (!hpd->configuration) return HPD_E_STOPPED;
    if (!discovery_is_adapter_id_unique(hpd, adapter)) return HPD_E_NOT_UNIQUE;
    return discovery_attach_adapter(hpd, adapter);
}

hpd_error_t hpd_adapter_detach(hpd_adapter_id_t *id, hpd_adapter_t **adapter)
{
    if (!id || !adapter) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_adapter_t *a;
    if ((rc = discovery_find_adapter(id, &a))) return rc;
    if ((rc = discovery_detach_adapter(a))) return rc;
    (*adapter) = a;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_adapter_set_data(hpd_adapter_t *adapter, void *data, hpd_free_f on_free)
{
    if (!adapter) return HPD_E_NULL;
    return discovery_set_adapter_data(adapter, data, on_free);
}

hpd_error_t hpd_adapter_set_attr(hpd_adapter_t *adapter, const char *key, const char *val)
{
    if (!adapter || !key) return HPD_E_NULL;
    return discovery_set_adapter_attr(adapter, key, val);
}

hpd_error_t hpd_adapter_set_attrs(hpd_adapter_t *adapter, ...)
{
    if (!adapter) return HPD_E_NULL;

    va_list vp;
    va_start(vp, adapter);
    hpd_error_t rc = discovery_set_adapter_attrs_v(adapter, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_adapter_get_data(hpd_adapter_id_t *id, void **data)
{
    if (!id || !data) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(id, &adapter))) return rc;
    return discovery_get_adapter_data(adapter, data);
}

hpd_error_t hpd_adapter_get_id(hpd_adapter_id_t *aid, const char **id)
{
    if (!aid || !id) return HPD_E_NULL;
    if (!aid->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(aid, &adapter))) return rc;
    return discovery_get_adapter_id(adapter, id);
}

hpd_error_t hpd_adapter_get_attr(hpd_adapter_id_t *id, const char *key, const char **val)
{
    if (!id || !key || !val) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(id, &adapter))) return rc;
    return discovery_get_adapter_attr(adapter, key, val);
}

hpd_error_t hpd_adapter_get_attrs(hpd_adapter_id_t *id, ...)
{
    if (!id) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(id, &adapter))) return rc;

    va_list vp;
    va_start(vp, id);
    rc = discovery_get_adapter_attrs_v(adapter, vp);
    va_end(vp);

    return rc;
}

/**
 * A note on reusing IDs: An other module or remote client may have outstanding references to the old ID, thus IDs
 * should only be reused in cases when the new device is either the old device, or a directly replacement, with the
 * same functionality.
 */
hpd_error_t hpd_device_alloc(hpd_device_t **device, const char *id)
{
    if (!device || !id) return HPD_E_NULL;
    return discovery_alloc_device(device, id);
}

hpd_error_t hpd_device_free(hpd_device_t *device)
{
    if (!device) return HPD_E_NULL;
    return discovery_free_device(device);
}

hpd_error_t hpd_device_attach(hpd_adapter_id_t *id, hpd_device_t *device) {
    if (!id || !device) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(id, &adapter))) return rc;
    if (!discovery_is_device_id_unique(adapter, device)) return HPD_E_NOT_UNIQUE;
    return discovery_attach_device(adapter, device);
}

hpd_error_t hpd_device_detach(hpd_device_id_t *id, hpd_device_t **device)
{
    if (!id || !device) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_device_t *d;
    if ((rc = discovery_find_device(id, &d))) return rc;
    if ((rc = discovery_detach_device(d))) return rc;
    (*device) = d;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_device_set_data(hpd_device_t *device, void *data, hpd_free_f on_free)
{
    if (!device) return HPD_E_NULL;
    return discovery_set_device_data(device, data, on_free);
}

hpd_error_t hpd_device_set_attr(hpd_device_t *device, const char *key, const char *val)
{
    if (!device || !key) return HPD_E_NULL;
    return discovery_set_device_attr(device, key, val);
}

hpd_error_t hpd_device_set_attrs(hpd_device_t *device, ...)
{
    if (!device) return HPD_E_NULL;

    va_list vp;
    va_start(vp, device);
    hpd_error_t rc = discovery_set_device_attrs_v(device, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_device_get_data(hpd_device_id_t *id, void **data)
{
    if (!id || !data) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(id, &device))) return rc;
    return discovery_get_device_data(device, data);
}

hpd_error_t hpd_device_get_id(hpd_device_id_t *did, const char **id)
{
    if (!did || !id) return HPD_E_NULL;
    if (!did->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(did, &device))) return rc;
    return discovery_get_device_id(device, id);
}

hpd_error_t hpd_device_get_attr(hpd_device_id_t *id, const char *key, const char **val)
{
    if (!id || !key || !val) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(id, &device))) return rc;
    return discovery_get_device_attr(device, key, val);
}

hpd_error_t hpd_device_get_attrs(hpd_device_id_t *id, ...)
{
    if (!id) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(id, &device))) return rc;

    va_list vp;
    va_start(vp, id);
    rc = discovery_get_device_attrs_v(device, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_service_alloc(hpd_service_t **service, const char *id)
{
    if (!service || !id) return HPD_E_NULL;
    return discovery_alloc_service(service, id);
}

hpd_error_t hpd_service_free(hpd_service_t *service)
{
    if (!service) return HPD_E_NULL;
    return discovery_free_service(service);
}

hpd_error_t hpd_service_attach(hpd_device_t *device, hpd_service_t *service)
{
    if (!device || !service) return HPD_E_NULL;
    if (!discovery_is_service_id_unique(device, service)) return HPD_E_NOT_UNIQUE;
    return discovery_attach_service(device, service);
}

hpd_error_t hpd_service_detach(hpd_service_id_t *id, hpd_service_t **service)
{
    if (!id || !service) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_service_t *s;
    if ((rc = discovery_find_service(id, &s))) return rc;
    if ((rc = discovery_detach_service(s))) return rc;
    (*service) = s;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_service_set_data(hpd_service_t *service, void *data, hpd_free_f on_free)
{
    if (!service) return HPD_E_NULL;
    return discovery_set_service_data(service, data, on_free);
}

hpd_error_t hpd_service_set_attr(hpd_service_t *service, const char *key, const char *val)
{
    if (!service || !key) return HPD_E_NULL;
    return discovery_set_service_attr(service, key, val);
}

hpd_error_t hpd_service_set_attrs(hpd_service_t *service, ...)
{
    if (!service) return HPD_E_NULL;

    va_list vp;
    va_start(vp, service);
    hpd_error_t rc = discovery_set_service_attrs_v(service, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_service_set_action(hpd_service_t *service, const hpd_method_t method, hpd_action_f action)
{
    if (!service || !action) return HPD_E_NULL;
    if (method <= HPD_M_NONE || method >= HPD_M_COUNT) return HPD_E_ARGUMENT;
    return discovery_set_service_action(service, method, action);
}

hpd_error_t hpd_service_set_actions(hpd_service_t *service, ...)
{
    if (!service) return HPD_E_NULL;

    va_list vp;
    va_start(vp, service);
    hpd_error_t rc = discovery_set_service_actions_v(service, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_service_get_data(hpd_service_id_t *id, void **data)
{
    if (!id || !data) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;
    return discovery_get_service_data(service, data);
}

hpd_error_t hpd_service_get_id(hpd_service_id_t *sid, const char **id)
{
    if (!sid || !id) return HPD_E_NULL;
    if (!sid->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(sid, &service))) return rc;
    return discovery_get_service_id(service, id);
}

hpd_error_t hpd_service_get_attr(hpd_service_id_t *id, const char *key, const char **val)
{
    if (!id || !key || !val) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;
    return discovery_get_service_attr(service, key, val);
}

hpd_error_t hpd_service_get_attrs(hpd_service_id_t *id, ...)
{
    if (!id) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;

    va_list vp;
    va_start(vp, id);
    rc = discovery_get_service_attrs_v(service, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_service_has_action(hpd_service_id_t *id, const hpd_method_t method, char *boolean)
{
    if (!id || !boolean) return HPD_E_NULL;
    if (method <= HPD_M_NONE || method >= HPD_M_COUNT) return HPD_E_ARGUMENT;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;
    (*boolean) = discovery_has_service_action(service, method);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_service_first_action(hpd_service_id_t *id, hpd_action_t **action)
{
    if (!id || !action) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;
    return discovery_first_action_in_service(service, action);
}

hpd_error_t hpd_service_next_action(hpd_action_t **action)
{
    if (!action || !(*action)) return HPD_E_NULL;
    return discovery_next_action_in_service(action);
}

hpd_error_t hpd_parameter_alloc(hpd_parameter_t **parameter, const char *id)
{
    if (!parameter || !id) return HPD_E_NULL;
    return discovery_alloc_parameter(parameter, id);
}

hpd_error_t hpd_parameter_free(hpd_parameter_t *parameter)
{
    if (!parameter) return HPD_E_NULL;
    return discovery_free_parameter(parameter);
}

hpd_error_t hpd_parameter_attach(hpd_service_t *service, hpd_parameter_t *parameter)
{
    if (!service || !parameter) return HPD_E_NULL;
    if (!discovery_is_parameter_id_unique(service, parameter)) return HPD_E_NOT_UNIQUE;
    return discovery_attach_parameter(service, parameter);
}

hpd_error_t hpd_parameter_detach(hpd_parameter_id_t *id, hpd_parameter_t **parameter)
{
    if (!id || !parameter) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_parameter_t *p;
    if ((rc = discovery_find_parameter(id, &p))) return rc;
    if ((rc = discovery_detach_parameter(p))) return rc;
    (*parameter) = p;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_parameter_set_attr(hpd_parameter_t *parameter, const char *key, const char *val)
{
    if (!parameter || !key) return HPD_E_NULL;
    return discovery_set_parameter_attr(parameter, key, val);
}

hpd_error_t hpd_parameter_set_attrs(hpd_parameter_t *parameter, ...)
{
    if (!parameter) return HPD_E_NULL;

    va_list vp;
    va_start(vp, parameter);
    hpd_error_t rc = discovery_set_parameter_attrs_v(parameter, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_parameter_get_id(hpd_parameter_id_t *pid, const char **id)
{
    if (!pid || !id) return HPD_E_NULL;
    if (!pid->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_parameter_t *parameter;
    if ((rc = discovery_find_parameter(pid, &parameter))) return rc;
    return discovery_get_parameter_id(parameter, id);
}

hpd_error_t hpd_parameter_get_attr(hpd_parameter_id_t *id, const char *key, const char **val)
{
    if (!id || !key || !val) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_parameter_t *parameter;
    if ((rc = discovery_find_parameter(id, &parameter))) return rc;
    return discovery_get_parameter_attr(parameter, key, val);
}

hpd_error_t hpd_parameter_get_attrs(hpd_parameter_id_t *id, ...)
{
    if (!id) return HPD_E_NULL;
    if (!id->hpd->configuration) return HPD_E_STOPPED;
    hpd_error_t rc;
    hpd_parameter_t *parameter;
    if ((rc = discovery_find_parameter(id, &parameter))) return rc;

    va_list vp;
    va_start(vp, id);
    rc = discovery_get_parameter_attrs_v(parameter, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_action_get_method(hpd_action_t *action, hpd_method_t *method)
{
    if (!action || !method) return HPD_E_NULL;
    return discovery_get_action_method(action, method);
}
