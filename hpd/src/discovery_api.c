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
 * THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
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

#include "daemon.h"
#include "hpd/hpd_shared_api.h"
#include "discovery.h"
#include "log.h"
#include "model.h"

hpd_error_t hpd_adapter_id_alloc(hpd_adapter_id_t **id, const hpd_module_t *context, const char *aid)
{
    if (!context) return HPD_E_NULL;
    if (!id || !aid) LOG_RETURN_E_NULL(context->hpd);

    return discovery_alloc_aid(id, context, aid);
}

hpd_error_t hpd_adapter_id_copy(hpd_adapter_id_t **dst, const hpd_adapter_id_t *src)
{
    if (!src) return HPD_E_NULL;
    if (!dst) LOG_RETURN_E_NULL(src->context->hpd);

    return discovery_copy_aid(dst, src);
}

hpd_error_t hpd_adapter_id_free(hpd_adapter_id_t *id)
{
    if (!id) return HPD_E_NULL;

    return discovery_free_aid(id);
}

hpd_error_t hpd_device_id_alloc(hpd_device_id_t **id, const hpd_module_t *context, const char *aid, const char *did)
{
    if (!context) return HPD_E_NULL;
    if (!id || !aid || !did) LOG_RETURN_E_NULL(context->hpd);

    return discovery_alloc_did(id, context, aid, did);
}

hpd_error_t hpd_device_id_copy(hpd_device_id_t **dst, const hpd_device_id_t *src)
{
    if (!src) return HPD_E_NULL;
    if (!dst) LOG_RETURN_E_NULL(src->adapter.context->hpd);

    return discovery_copy_did(dst, src);
}

hpd_error_t hpd_device_id_free(hpd_device_id_t *id)
{
    if (!id) return HPD_E_NULL;

    return discovery_free_did(id);
}

hpd_error_t hpd_service_id_alloc(hpd_service_id_t **id, const hpd_module_t *context, const char *aid, const char *did, const char *sid)
{
    if (!context) return HPD_E_NULL;
    if (!id || !aid || !did || !sid) LOG_RETURN_E_NULL(context->hpd);

    return discovery_alloc_sid(id, context, aid, did, sid);
}

hpd_error_t hpd_service_id_copy(hpd_service_id_t **dst, const hpd_service_id_t *src)
{
    if (!src) return HPD_E_NULL;
    if (!dst) LOG_RETURN_E_NULL(src->device.adapter.context->hpd);

    return discovery_copy_sid(dst, src);
}

hpd_error_t hpd_service_id_free(hpd_service_id_t *id)
{
    if (!id) return HPD_E_NULL;

    return discovery_free_sid(id);
}

hpd_error_t hpd_parameter_id_alloc(hpd_parameter_id_t **id, const hpd_module_t *context, const char *aid,
                                   const char *did, const char *sid, const char *pid)
{
    if (!context) return HPD_E_NULL;
    if (!id || !aid || !did || !sid || !pid) LOG_RETURN_E_NULL(context->hpd);

    return discovery_alloc_pid(id, context, aid, did, sid, pid);
}

hpd_error_t hpd_parameter_id_copy(hpd_parameter_id_t **dst, const hpd_parameter_id_t *src)
{
    if (!src) return HPD_E_NULL;
    if (!dst) LOG_RETURN_E_NULL(src->service.device.adapter.context->hpd);

    return discovery_copy_pid(dst, src);
}

hpd_error_t hpd_parameter_id_free(hpd_parameter_id_t *id)
{
    if (!id) return HPD_E_NULL;

    return discovery_free_pid(id);
}

hpd_error_t hpd_adapter_alloc(hpd_adapter_t **adapter, const hpd_module_t *context, const char *id)
{
    if (!context) return HPD_E_NULL;
    if (!adapter || !id) LOG_RETURN_E_NULL(context->hpd);

    return discovery_alloc_adapter(adapter, context, id);
}

hpd_error_t hpd_adapter_free(hpd_adapter_t *adapter)
{
    if (!adapter) return HPD_E_NULL;
    
    if (adapter->configuration) {
        hpd_error_t rc;
        if ((rc = discovery_detach_adapter(adapter))) return rc;
    }
    // TODO Error here can result in a weird state...
    return discovery_free_adapter(adapter);
}

hpd_error_t hpd_adapter_attach(hpd_adapter_t *adapter)
{
    if (!adapter) return HPD_E_NULL;
    hpd_t *hpd = adapter->context->hpd;
    if (adapter->configuration) LOG_RETURN_ATTACHED(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    if (!discovery_is_adapter_id_unique(hpd, adapter))
        LOG_RETURN(hpd, HPD_E_NOT_UNIQUE, "Adapters ids must be unique.");
    
    return discovery_attach_adapter(hpd, adapter);
}

hpd_error_t hpd_adapter_detach(hpd_adapter_t *adapter)
{
    hpd_error_t rc;
    if (!adapter) return HPD_E_NULL;
    if (!adapter->configuration) LOG_RETURN_DETACHED(adapter->context->hpd);
    if ((rc = discovery_detach_adapter(adapter))) return rc;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_adapter_id_detach(const hpd_adapter_id_t *id, hpd_adapter_t **adapter)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->context->hpd;
    if (!adapter) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
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
    if (!adapter) return HPD_E_NULL;
    hpd_t *hpd = adapter->context->hpd;
    if (!key) LOG_RETURN_E_NULL(hpd);
    if (key[0] == '_') LOG_RETURN(hpd, HPD_E_ARGUMENT, "Keys starting with '_' is reserved for generated attributes");
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

hpd_error_t hpd_adapter_get_data(const hpd_adapter_t *adapter, void **data)
{
    if (!adapter) return HPD_E_NULL;
    if (!data) LOG_RETURN_E_NULL(adapter->context->hpd);
    return discovery_get_adapter_data(adapter, data);
}

hpd_error_t hpd_adapter_get_adapter_id_str(const hpd_adapter_t *adapter, const char **id)
{
    if (!adapter) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(adapter->context->hpd);
    return discovery_get_adapter_id(adapter, id);
}

hpd_error_t hpd_adapter_id_get_adapter_id_str(const hpd_adapter_id_t *aid, const char **id)
{
    if (!aid) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(aid->context->hpd);
    return discovery_get_aid_aid(aid, id);
}

hpd_error_t hpd_adapter_get_attr(const hpd_adapter_t *adapter, const char *key, const char **val)
{
    if (!adapter) return HPD_E_NULL;
    if (!key || !val) LOG_RETURN_E_NULL(adapter->context->hpd);
    return discovery_get_adapter_attr(adapter, key, val);
}

hpd_error_t hpd_adapter_id_get_attr(const hpd_adapter_id_t *id, const char *key, const char **val)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->context->hpd;
    if (!key || !val) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(id, &adapter))) return rc;
    return discovery_get_adapter_attr(adapter, key, val);
}

hpd_error_t hpd_adapter_get_attrs(const hpd_adapter_t *adapter, ...)
{
    hpd_error_t rc;
    if (!adapter) return HPD_E_NULL;

    va_list vp;
    va_start(vp, adapter);
    rc = discovery_get_adapter_attrs_v(adapter, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_adapter_id_get_attrs(const hpd_adapter_id_t *id, ...)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(id, &adapter))) return rc;

    va_list vp;
    va_start(vp, id);
    rc = discovery_get_adapter_attrs_v(adapter, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_adapter_first_attr(const hpd_adapter_t *adapter, const hpd_pair_t **pair)
{
    if (!adapter) return HPD_E_NULL;
    if (!pair) LOG_RETURN_E_NULL(adapter->context->hpd);
    return discovery_first_adapter_attr(adapter, pair);
}

hpd_error_t hpd_adapter_id_first_attr(const hpd_adapter_id_t *id, const hpd_pair_t **pair)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->context->hpd;
    if (!pair) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(id, &adapter))) return rc;

    return discovery_first_adapter_attr(adapter, pair);
}

hpd_error_t hpd_adapter_next_attr(const hpd_adapter_t *adapter, const hpd_pair_t **pair)
{
    if (!adapter) return HPD_E_NULL;
    if (!pair || !(*pair)) LOG_RETURN_E_NULL(adapter->context->hpd);
    return discovery_next_adapter_attr(pair);
}

hpd_error_t hpd_adapter_id_next_attr(const hpd_adapter_id_t *id, const hpd_pair_t **pair)
{
    if (!id) return HPD_E_NULL;
    if (!pair || !(*pair)) LOG_RETURN_E_NULL(id->context->hpd);
    return discovery_next_adapter_attr(pair);
}

/**
 * A note on reusing IDs: An other module or remote client may have outstanding references to the old ID, thus IDs
 * should only be reused in cases when the new device is either the old device, or a directly replacement, with the
 * same functionality.
 */
hpd_error_t hpd_device_alloc(hpd_device_t **device, const hpd_module_t *context, const char *id)
{
    if (!context) return HPD_E_NULL;
    if (!device || !id) LOG_RETURN_E_NULL(context->hpd);
    return discovery_alloc_device(device, context, id);
}

hpd_error_t hpd_device_free(hpd_device_t *device)
{
    if (!device) return HPD_E_NULL;
    if (device->adapter) {
        hpd_error_t rc;
        if ((rc = discovery_detach_device(device))) return rc;
    }
    // TODO Error here can result in a weird state...
    return discovery_free_device(device);
}

hpd_error_t hpd_device_attach(hpd_adapter_t *adapter, hpd_device_t *device)
{
    if (!adapter) return HPD_E_NULL;
    hpd_t *hpd = adapter->context->hpd;
    if (!device) LOG_RETURN_E_NULL(hpd);
    if (device->adapter) LOG_RETURN_ATTACHED(hpd);
    if (!discovery_is_device_id_unique(adapter, device))
        LOG_RETURN(hpd, HPD_E_NOT_UNIQUE, "Device ids must be unique within the adapter.");
    return discovery_attach_device(adapter, device);
}

hpd_error_t hpd_device_id_attach(const hpd_adapter_id_t *id, hpd_device_t *device)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->context->hpd;
    if (!device) LOG_RETURN_E_NULL(hpd);
    if (device->adapter) LOG_RETURN_ATTACHED(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(id, &adapter))) return rc;
    if (!discovery_is_device_id_unique(adapter, device))
        LOG_RETURN(hpd, HPD_E_NOT_UNIQUE, "Device ids must be unique within the adapter.");
    return discovery_attach_device(adapter, device);
}

hpd_error_t hpd_device_detach(hpd_device_t *device)
{
    if (!device) return HPD_E_NULL;
    if (!device->adapter) LOG_RETURN_DETACHED(device->context->hpd);
    return discovery_detach_device(device);
}

hpd_error_t hpd_device_id_detach(const hpd_device_id_t *id, hpd_device_t **device)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->adapter.context->hpd;
    if (!device) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
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
    if (!device) return HPD_E_NULL;
    hpd_t *hpd = device->context->hpd;
    if (!key) LOG_RETURN_E_NULL(hpd);
    if (key[0] == '_') LOG_RETURN(hpd, HPD_E_ARGUMENT, "Keys starting with '_' is reserved for generated attributes");
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

hpd_error_t hpd_device_get_data(const hpd_device_t *device, void **data)
{
    if (!device) return HPD_E_NULL;
    if (!data) LOG_RETURN_E_NULL(device->context->hpd);
    return discovery_get_device_data(device, data);
}

hpd_error_t hpd_device_get_adapter_id_str(const hpd_device_t *device, const char **id)
{
    if (!device) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(device->context->hpd);
    return discovery_get_device_adapter_id(device, id);
}

hpd_error_t hpd_device_get_device_id_str(const hpd_device_t *device, const char **id)
{
    if (!device) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(device->context->hpd);
    return discovery_get_device_id(device, id);
}

hpd_error_t hpd_device_id_get_adapter_id_str(const hpd_device_id_t *did, const char **id)
{
    if (!did) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(did->adapter.context->hpd);
    return discovery_get_did_aid(did, id);
}

hpd_error_t hpd_device_id_get_device_id_str(const hpd_device_id_t *did, const char **id)
{
    if (!did) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(did->adapter.context->hpd);
    return discovery_get_did_did(did, id);
}

hpd_error_t hpd_device_id_get_attr(const hpd_device_id_t *id, const char *key, const char **val)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->adapter.context->hpd;
    if (!key || !val) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(id, &device))) return rc;
    return discovery_get_device_attr(device, key, val);
}

hpd_error_t hpd_device_get_attr(const hpd_device_t *device, const char *key, const char **val)
{
    if (!device) return HPD_E_NULL;
    if (!key || !val) LOG_RETURN_E_NULL(device->context->hpd);
    return discovery_get_device_attr(device, key, val);
}

hpd_error_t hpd_device_id_get_attrs(const hpd_device_id_t *id, ...)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->adapter.context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(id, &device))) return rc;

    va_list vp;
    va_start(vp, id);
    rc = discovery_get_device_attrs_v(device, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_device_get_attrs(const hpd_device_t *device, ...)
{
    hpd_error_t rc;
    if (!device) return HPD_E_NULL;

    va_list vp;
    va_start(vp, device);
    rc = discovery_get_device_attrs_v(device, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_device_id_first_attr(const hpd_device_id_t *id, const hpd_pair_t **pair)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->adapter.context->hpd;
    if (!pair) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(id, &device))) return rc;

    return discovery_first_device_attr(device, pair);
}

hpd_error_t hpd_device_first_attr(const hpd_device_t *device, const hpd_pair_t **pair)
{
    if (!device) return HPD_E_NULL;
    if (!pair) LOG_RETURN_E_NULL(device->context->hpd);
    return discovery_first_device_attr(device, pair);
}

hpd_error_t hpd_device_id_next_attr(const hpd_device_id_t *id, const hpd_pair_t **pair)
{
    if (!id) return HPD_E_NULL;
    if (!pair || !(*pair)) LOG_RETURN_E_NULL(id->adapter.context->hpd);
    return discovery_next_device_attr(pair);
}

hpd_error_t hpd_device_next_attr(const hpd_device_t *device, const hpd_pair_t **pair)
{
    if (!device) return HPD_E_NULL;
    if (!pair || !(*pair)) LOG_RETURN_E_NULL(device->context->hpd);
    return discovery_next_device_attr(pair);
}

hpd_error_t hpd_service_alloc(hpd_service_t **service, const hpd_module_t *context, const char *id)
{
    if (!context) return HPD_E_NULL;
    if (!service || !id) LOG_RETURN_E_NULL(context->hpd);
    return discovery_alloc_service(service, context, id);
}

hpd_error_t hpd_service_free(hpd_service_t *service)
{
    if (!service) return HPD_E_NULL;
    if (service->device) {
        hpd_error_t rc;
        if ((rc = discovery_detach_service(service))) return rc;
    }
    // TODO Error here can result in a weird state...
    return discovery_free_service(service);
}

hpd_error_t hpd_service_id_attach(const hpd_device_id_t *id, hpd_service_t *service)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->adapter.context->hpd;
    if (!service) LOG_RETURN_E_NULL(hpd);
    if (service->device) LOG_RETURN_ATTACHED(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(id, &device))) return rc;
    if (!discovery_is_service_id_unique(device, service))
        LOG_RETURN(hpd, HPD_E_NOT_UNIQUE, "Service ids must be unique within the device [id: %s].", service->id);
    return discovery_attach_service(device, service);
}

hpd_error_t hpd_service_attach(hpd_device_t *device, hpd_service_t *service)
{
    if (!device) return HPD_E_NULL;
    hpd_t *hpd = device->context->hpd;
    if (!device || !service) LOG_RETURN_E_NULL(hpd);
    if (service->device) LOG_RETURN_ATTACHED(hpd);
    if (!discovery_is_service_id_unique(device, service))
        LOG_RETURN(hpd, HPD_E_NOT_UNIQUE, "Service ids must be unique within the device [id: %s].", service->id);
    return discovery_attach_service(device, service);
}

hpd_error_t hpd_service_detach(hpd_service_t *service)
{
    if (!service) return HPD_E_NULL;
    if (!service->device) LOG_RETURN_DETACHED(service->context->hpd);
    return discovery_detach_service(service);
}

hpd_error_t hpd_service_id_detach(const hpd_service_id_t *id, hpd_service_t **service)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!service) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
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
    if (!service) return HPD_E_NULL;
    hpd_t *hpd = service->context->hpd;
    if (!key) LOG_RETURN_E_NULL(hpd);
    if (key[0] == '_') LOG_RETURN(hpd, HPD_E_ARGUMENT, "Keys starting with '_' is reserved for generated attributes");
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
    if (!service) return HPD_E_NULL;
    hpd_t *hpd = service->context->hpd;
    if (!action) LOG_RETURN_E_NULL(hpd);
    if (method <= HPD_M_NONE || method >= HPD_M_COUNT)
        LOG_RETURN(hpd, HPD_E_ARGUMENT, "Unknown method given to %s().", __func__);
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

hpd_error_t hpd_service_get_data(const hpd_service_t *service, void **data)
{
    if (!service) return HPD_E_NULL;
    if (!data) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_get_service_data(service, data);
}

hpd_error_t hpd_service_id_get_adapter_id_str(const hpd_service_id_t *sid, const char **id)
{
    if (!sid) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(sid->device.adapter.context->hpd);
    return discovery_get_sid_aid(sid, id);
}

hpd_error_t hpd_service_id_get_device_id_str(const hpd_service_id_t *sid, const char **id)
{
    if (!sid) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(sid->device.adapter.context->hpd);
    return discovery_get_sid_did(sid, id);
}

hpd_error_t hpd_service_id_get_service_id_str(const hpd_service_id_t *sid, const char **id)
{
    if (!sid) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(sid->device.adapter.context->hpd);
    return discovery_get_sid_sid(sid, id);
}

hpd_error_t hpd_service_get_adapter_id_str(const hpd_service_t *service, const char **id)
{
    if (!service) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_get_service_adapter_id(service, id);
}

hpd_error_t hpd_service_get_device_id_str(const hpd_service_t *service, const char **id)
{
    if (!service) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_get_service_device_id(service, id);
}

hpd_error_t hpd_service_get_service_id_str(const hpd_service_t *service, const char **id)
{
    if (!service) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_get_service_id(service, id);
}


hpd_error_t hpd_service_id_get_attr(const hpd_service_id_t *id, const char *key, const char **val)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!key || !val) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;
    return discovery_get_service_attr(service, key, val);
}

hpd_error_t hpd_service_id_get_attrs(const hpd_service_id_t *id, ...)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;

    va_list vp;
    va_start(vp, id);
    rc = discovery_get_service_attrs_v(service, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_service_id_has_action(const hpd_service_id_t *id, const hpd_method_t method, hpd_bool_t *boolean)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!boolean) LOG_RETURN_E_NULL(hpd);
    if (method <= HPD_M_NONE || method >= HPD_M_COUNT) LOG_RETURN(hpd, HPD_E_ARGUMENT, "Unknown method given to %s().", __func__);
    if (!hpd->configuration)
        LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;
    (*boolean) = discovery_has_service_action(service, method);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_service_id_first_action(const hpd_service_id_t *id, const hpd_action_t **action)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!action) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;
    return discovery_first_action_in_service(service, action);
}

hpd_error_t hpd_service_id_next_action(const hpd_service_id_t *id, const hpd_action_t **action)
{
    if (!id) return HPD_E_NULL;
    if (!action || !(*action)) LOG_RETURN_E_NULL(id->device.adapter.context->hpd);
    return discovery_next_action_in_service(action);
}

hpd_error_t hpd_service_id_first_attr(const hpd_service_id_t *id, const hpd_pair_t **pair)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!pair) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;

    return discovery_first_service_attr(service, pair);
}

hpd_error_t hpd_service_id_next_attr(const hpd_service_id_t *id, const hpd_pair_t **pair)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!pair || !(*pair)) LOG_RETURN_E_NULL(hpd);

    return discovery_next_service_attr(pair);
}

hpd_error_t hpd_service_get_attr(const hpd_service_t *service, const char *key, const char **val)
{
    if (!service) return HPD_E_NULL;
    if (!key || !val) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_get_service_attr(service, key, val);
}

hpd_error_t hpd_service_get_attrs(const hpd_service_t *service, ...)
{
    hpd_error_t rc;
    if (!service) return HPD_E_NULL;

    va_list vp;
    va_start(vp, service);
    rc = discovery_get_service_attrs_v(service, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_service_has_action(const hpd_service_t *service, const hpd_method_t method, hpd_bool_t *boolean)
{
    if (!service) return HPD_E_NULL;
    hpd_t *hpd = service->context->hpd;
    if (!boolean) LOG_RETURN_E_NULL(hpd);
    if (method <= HPD_M_NONE || method >= HPD_M_COUNT) LOG_RETURN(hpd, HPD_E_ARGUMENT, "Unknown method given to %s().", __func__);
    (*boolean) = discovery_has_service_action(service, method);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_service_first_action(const hpd_service_t *service, const hpd_action_t **action)
{
    if (!service) return HPD_E_NULL;
    if (!action) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_first_action_in_service(service, action);
}

hpd_error_t hpd_service_next_action(const hpd_service_t *service, const hpd_action_t **action)
{
    if (!service) return HPD_E_NULL;
    if (!action || !(*action)) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_next_action_in_service(action);
}

hpd_error_t hpd_service_first_attr(const hpd_service_t *service, const hpd_pair_t **pair)
{
    if (!service) return HPD_E_NULL;
    if (!service || !pair) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_first_service_attr(service, pair);
}

hpd_error_t hpd_service_next_attr(const hpd_service_t *service, const hpd_pair_t **pair)
{
    if (!service) return HPD_E_NULL;
    if (!pair || !(*pair)) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_next_service_attr(pair);
}

hpd_error_t hpd_parameter_alloc(hpd_parameter_t **parameter, const hpd_module_t *context, const char *id)
{
    if (!context) return HPD_E_NULL;
    if (!parameter || !id) LOG_RETURN_E_NULL(context->hpd);
    return discovery_alloc_parameter(parameter, context, id);
}

hpd_error_t hpd_parameter_free(hpd_parameter_t *parameter)
{
    if (!parameter) return HPD_E_NULL;
    if (parameter->service) {
        hpd_error_t rc;
        if ((rc = discovery_detach_parameter(parameter))) return rc;
    }
    // TODO Error here can result in a weird state...
    return discovery_free_parameter(parameter);
}

hpd_error_t hpd_parameter_attach(hpd_service_t *service, hpd_parameter_t *parameter)
{
    if (!service) return HPD_E_NULL;
    hpd_t *hpd = service->context->hpd;
    if (!parameter) LOG_RETURN_E_NULL(hpd);
    if (parameter->service) LOG_RETURN_ATTACHED(hpd);
    if (!discovery_is_parameter_id_unique(service, parameter))
        LOG_RETURN(hpd, HPD_E_NOT_UNIQUE, "Parameter ids must be unique within the service.");
    return discovery_attach_parameter(service, parameter);
}

hpd_error_t hpd_parameter_id_attach(const hpd_service_id_t *id, hpd_parameter_t *parameter)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->device.adapter.context->hpd;
    if (!parameter) LOG_RETURN_E_NULL(hpd);
    if (parameter->service) LOG_RETURN_ATTACHED(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(id, &service))) return rc;
    if (!discovery_is_parameter_id_unique(service, parameter))
        LOG_RETURN(hpd, HPD_E_NOT_UNIQUE, "Parameter ids must be unique within the service.");
    return discovery_attach_parameter(service, parameter);
}

hpd_error_t hpd_parameter_detach(hpd_parameter_t *parameter)
{
    if (!parameter) return HPD_E_NULL;
    if (!parameter->service) LOG_RETURN_DETACHED(parameter->context->hpd);
    return discovery_detach_parameter(parameter);
}

hpd_error_t hpd_parameter_id_detach(const hpd_parameter_id_t *id, hpd_parameter_t **parameter)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->service.device.adapter.context->hpd;
    if (!parameter) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_parameter_t *p;
    if ((rc = discovery_find_parameter(id, &p))) return rc;
    if ((rc = discovery_detach_parameter(p))) return rc;
    (*parameter) = p;
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_parameter_set_attr(hpd_parameter_t *parameter, const char *key, const char *val)
{
    if (!parameter) return HPD_E_NULL;
    hpd_t *hpd = parameter->context->hpd;
    if (!key) LOG_RETURN_E_NULL(hpd);
    if (key[0] == '_') LOG_RETURN(hpd, HPD_E_ARGUMENT, "Keys starting with '_' is reserved for generated attributes");
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

hpd_error_t hpd_parameter_id_get_adapter_id_str(const hpd_parameter_id_t *pid, const char **id)
{
    if (!pid) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(pid->service.device.adapter.context->hpd);
    return discovery_get_pid_aid(pid, id);
}

hpd_error_t hpd_parameter_id_get_device_id_str(const hpd_parameter_id_t *pid, const char **id)
{
    if (!pid) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(pid->service.device.adapter.context->hpd);
    return discovery_get_pid_did(pid, id);
}

hpd_error_t hpd_parameter_id_get_service_id_str(const hpd_parameter_id_t *pid, const char **id)
{
    if (!pid) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(pid->service.device.adapter.context->hpd);
    return discovery_get_pid_sid(pid, id);
}

hpd_error_t hpd_parameter_id_get_parameter_id_str(const hpd_parameter_id_t *pid, const char **id)
{
    if (!pid) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(pid->service.device.adapter.context->hpd);
    return discovery_get_pid_pid(pid, id);
}

hpd_error_t hpd_parameter_id_get_attr(const hpd_parameter_id_t *id, const char *key, const char **val)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->service.device.adapter.context->hpd;
    if (!key || !val) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_parameter_t *parameter;
    if ((rc = discovery_find_parameter(id, &parameter))) return rc;
    return discovery_get_parameter_attr(parameter, key, val);
}

hpd_error_t hpd_parameter_id_get_attrs(const hpd_parameter_id_t *id, ...)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->service.device.adapter.context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);
    hpd_error_t rc;
    hpd_parameter_t *parameter;
    if ((rc = discovery_find_parameter(id, &parameter))) return rc;

    va_list vp;
    va_start(vp, id);
    rc = discovery_get_parameter_attrs_v(parameter, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_parameter_id_first_attr(const hpd_parameter_id_t *id, const hpd_pair_t **pair)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->service.device.adapter.context->hpd;
    if (!pair) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_parameter_t *parameter;
    if ((rc = discovery_find_parameter(id, &parameter))) return rc;

    return discovery_first_parameter_attr(parameter, pair);
}

hpd_error_t hpd_parameter_id_next_attr(const hpd_parameter_id_t *id, const hpd_pair_t **pair)
{
    if (!id) return HPD_E_NULL;
    hpd_t *hpd = id->service.device.adapter.context->hpd;
    if (!pair || !(*pair)) LOG_RETURN_E_NULL(hpd);

    return discovery_next_parameter_attr(pair);
}

hpd_error_t hpd_parameter_get_adapter_id_str(const hpd_parameter_t *parameter, const char **id)
{
    if (!parameter) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_get_parameter_adapter_id(parameter, id);
}

hpd_error_t hpd_parameter_get_device_id_str(const hpd_parameter_t *parameter, const char **id)
{
    if (!parameter) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_get_parameter_device_id(parameter, id);
}

hpd_error_t hpd_parameter_get_service_id_str(const hpd_parameter_t *parameter, const char **id)
{
    if (!parameter) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_get_parameter_service_id(parameter, id);
}

hpd_error_t hpd_parameter_get_parameter_id_str(const hpd_parameter_t *parameter, const char **id)
{
    if (!parameter) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_get_parameter_id(parameter, id);
}

hpd_error_t hpd_parameter_get_attr(const hpd_parameter_t *parameter, const char *key, const char **val)
{
    if (!parameter) return HPD_E_NULL;
    if (!key || !val) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_get_parameter_attr(parameter, key, val);
}

hpd_error_t hpd_parameter_get_attrs(const hpd_parameter_t *parameter, ...)
{
    hpd_error_t rc;
    if (!parameter) return HPD_E_NULL;

    va_list vp;
    va_start(vp, parameter);
    rc = discovery_get_parameter_attrs_v(parameter, vp);
    va_end(vp);

    return rc;
}

hpd_error_t hpd_parameter_first_attr(const hpd_parameter_t *parameter, const hpd_pair_t **pair)
{
    if (!parameter) return HPD_E_NULL;
    if (!pair) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_first_parameter_attr(parameter, pair);
}

hpd_error_t hpd_parameter_next_attr(const hpd_parameter_t *parameter, const hpd_pair_t **pair)
{
    if (!parameter) return HPD_E_NULL;
    if (!pair || !(*pair)) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_next_parameter_attr(pair);
}

hpd_error_t hpd_action_get_method(const hpd_action_t *action, hpd_method_t *method)
{
    if (!action) return HPD_E_NULL;
    if (!method) LOG_RETURN_E_NULL(action->service->context->hpd);
    return discovery_get_action_method(action, method);
}

hpd_error_t hpd_action_get_action(const hpd_action_t *action, hpd_action_f *cb)
{
    if (!action) return HPD_E_NULL;
    if (!cb) LOG_RETURN_E_NULL(action->service->context->hpd);
    return discovery_get_action_action(action, cb);
}

hpd_error_t hpd_device_id_get_adapter_id(const hpd_device_id_t *did, const hpd_adapter_id_t **aid)
{
    if (!did) return HPD_E_NULL;
    if (!aid) LOG_RETURN_E_NULL(did->adapter.context->hpd);
    return discovery_get_did_adapter(did, aid);
}

hpd_error_t hpd_service_id_get_adapter_id(const hpd_service_id_t *sid, const hpd_adapter_id_t **aid)
{
    if (!sid) return HPD_E_NULL;
    if (!aid) LOG_RETURN_E_NULL(sid->device.adapter.context->hpd);
    return discovery_get_sid_adapter(sid, aid);
}

hpd_error_t hpd_service_id_get_device_id(const hpd_service_id_t *sid, const hpd_device_id_t **did)
{
    if (!sid) return HPD_E_NULL;
    if (!did) LOG_RETURN_E_NULL(sid->device.adapter.context->hpd);
    return discovery_get_sid_device(sid, did);
}

hpd_error_t hpd_parameter_id_get_adapter_id(const hpd_parameter_id_t *pid, const hpd_adapter_id_t **aid)
{
    if (!pid) return HPD_E_NULL;
    if (!aid) LOG_RETURN_E_NULL(pid->service.device.adapter.context->hpd);
    return discovery_get_pid_adapter(pid, aid);
}

hpd_error_t hpd_parameter_id_get_device_id(const hpd_parameter_id_t *pid, const hpd_device_id_t **did)
{
    if (!pid) return HPD_E_NULL;
    if (!did) LOG_RETURN_E_NULL(pid->service.device.adapter.context->hpd);
    return discovery_get_pid_device(pid, did);
}

hpd_error_t hpd_parameter_id_get_service_id(const hpd_parameter_id_t *pid, const hpd_service_id_t **sid)
{
    if (!pid) return HPD_E_NULL;
    if (!sid) LOG_RETURN_E_NULL(pid->service.device.adapter.context->hpd);
    return discovery_get_pid_service(pid, sid);
}

hpd_error_t hpd_first_adapter_id(const hpd_module_t *context, hpd_adapter_id_t **adapter_id)
{
    if (!context) return HPD_E_NULL;
    hpd_t *hpd = context->hpd;
    if (!adapter_id) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_first_hpd_adapter(hpd, &adapter))) return rc;

    if (!adapter) {
        (*adapter_id) = NULL;
        return HPD_E_SUCCESS;
    }

    return discovery_alloc_aid(adapter_id, context, adapter->id);
}

hpd_error_t hpd_first_device_id(const hpd_module_t *context, hpd_device_id_t **device_id)
{
    if (!context) return HPD_E_NULL;
    if (!device_id) LOG_RETURN_E_NULL(context->hpd);
    if (!context->hpd->configuration) LOG_RETURN_HPD_STOPPED(context->hpd);

    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_first_hpd_device(context->hpd, &device))) return rc;

    if (!device) {
        (*device_id) = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_adapter_t *adapter = device->adapter;
    return discovery_alloc_did(device_id, context, adapter->id, device->id);
}

hpd_error_t hpd_first_service_id(const hpd_module_t *context, hpd_service_id_t **service_id)
{
    if (!context) return HPD_E_NULL;
    if (!service_id) LOG_RETURN_E_NULL(context->hpd);
    if (!context->hpd->configuration) LOG_RETURN_HPD_STOPPED(context->hpd);

    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_first_hpd_service(context->hpd, &service))) return rc;

    if (!service) {
        (*service_id) = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_device_t *device = service->device;
    hpd_adapter_t *adapter = device->adapter;
    return discovery_alloc_sid(service_id, context, adapter->id, device->id, service->id);
}

hpd_error_t hpd_adapter_id_first_device_id(const hpd_adapter_id_t *adapter_id, hpd_device_id_t **device_id)
{
    if (!adapter_id) return HPD_E_NULL;
    hpd_t *hpd = adapter_id->context->hpd;
    if (!device_id) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(adapter_id, &adapter))) return rc;

    hpd_device_t *device;
    if ((rc = discovery_first_adapter_device(adapter, &device))) return rc;

    if (!device) {
        (*device_id) = NULL;
        return HPD_E_SUCCESS;
    }
    
    return discovery_alloc_did(device_id, adapter_id->context, adapter->id, device->id);
}

hpd_error_t hpd_adapter_id_first_service_id(const hpd_adapter_id_t *adapter_id, hpd_service_id_t **service_id)
{
    if (!adapter_id) return HPD_E_NULL;
    hpd_t *hpd = adapter_id->context->hpd;
    if (!service_id) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(adapter_id, &adapter))) return rc;

    hpd_service_t *service;
    if ((rc = discovery_first_adapter_service(adapter, &service))) return rc;

    if (!service) {
        (*service_id) = NULL;
        return HPD_E_SUCCESS;
    }
    
    hpd_device_t *device = service->device;
    return discovery_alloc_sid(service_id, adapter_id->context, adapter->id, device->id, service->id);
}

hpd_error_t hpd_device_id_first_service_id(const hpd_device_id_t *device_id, hpd_service_id_t **service_id)
{
    if (!device_id) return HPD_E_NULL;
    const hpd_module_t *context = device_id->adapter.context;
    hpd_t *hpd = context->hpd;
    if (!service_id) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(device_id, &device))) return rc;

    hpd_service_t *service;
    if ((rc = discovery_first_device_service(device, &service))) return rc;

    if (!service) {
        (*service_id) = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_adapter_t *adapter = device->adapter;
    return discovery_alloc_sid(service_id, context, adapter->id, device->id, service->id);
}

hpd_error_t hpd_service_id_first_parameter_id(const hpd_service_id_t *service_id, hpd_parameter_id_t **parameter_id)
{
    if (!service_id) return HPD_E_NULL;
    const hpd_module_t *context = service_id->device.adapter.context;
    hpd_t *hpd = context->hpd;
    if (!parameter_id) LOG_RETURN_E_NULL(hpd);
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(service_id, &service))) return rc;

    hpd_parameter_t *parameter;
    if ((rc = discovery_first_service_parameter(service, &parameter))) return rc;

    if (!parameter) {
        (*parameter_id) = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_device_t *device = service->device;
    hpd_adapter_t *adapter = device->adapter;
    return discovery_alloc_pid(parameter_id, context, adapter->id, device->id, service->id, parameter->id);
}


hpd_error_t hpd_next_adapter_id(hpd_adapter_id_t **adapter_id)
{
    if (!adapter_id || !(*adapter_id)) return HPD_E_NULL;
    const hpd_module_t *context = (*adapter_id)->context;
    hpd_t *hpd = context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_adapter_t *adapter;
    if ((rc = discovery_find_adapter(*adapter_id, &adapter))) return rc;

    if ((rc = discovery_next_hpd_adapter(&adapter))) return rc;

    if (!adapter) {
        discovery_free_aid(*adapter_id);
        *adapter_id = NULL;
        return HPD_E_SUCCESS;
    }

    if ((rc = discovery_set_aid(*adapter_id, context, adapter->id))) {
        discovery_free_aid(*adapter_id);
        *adapter_id = NULL;
    }
    return rc;
}

hpd_error_t hpd_next_device_id(hpd_device_id_t **device_id)
{
    if (!device_id || !(*device_id)) return HPD_E_NULL;
    const hpd_module_t *context = (*device_id)->adapter.context;
    hpd_t *hpd = context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(*device_id, &device))) return rc;

    if ((rc = discovery_next_hpd_device(&device))) return rc;

    if (!device) {
        discovery_free_did(*device_id);
        *device_id = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_adapter_t *adapter = device->adapter;
    if ((rc = discovery_set_did(*device_id, context, adapter->id, device->id))) {
        discovery_free_did(*device_id);
        *device_id = NULL;
    }
    return rc;
}

hpd_error_t hpd_next_service_id(hpd_service_id_t **service_id)
{
    if (!service_id || !(*service_id)) return HPD_E_NULL;
    const hpd_module_t *context = (*service_id)->device.adapter.context;
    hpd_t *hpd = context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(*service_id, &service))) return rc;

    if ((rc = discovery_next_hpd_service(&service))) return rc;

    if (!service) {
        discovery_free_sid(*service_id);
        *service_id = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_device_t *device = service->device;
    hpd_adapter_t *adapter = device->adapter;
    if ((rc = discovery_set_sid(*service_id, context, adapter->id, device->id, service->id))) {
        discovery_free_sid(*service_id);
        *service_id = NULL;
    }
    return rc;
}

hpd_error_t hpd_adapter_id_next_device_id(hpd_device_id_t **device_id)
{
    if (!device_id || !(*device_id)) return HPD_E_NULL;
    const hpd_module_t *context = (*device_id)->adapter.context;
    hpd_t *hpd = context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_device_t *device;
    if ((rc = discovery_find_device(*device_id, &device))) return rc;

    if ((rc = discovery_next_adapter_device(&device))) return rc;

    if (!device) {
        discovery_free_did(*device_id);
        *device_id = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_adapter_t *adapter = device->adapter;
    if ((rc = discovery_set_did(*device_id, context, adapter->id, device->id))) {
        discovery_free_did(*device_id);
        *device_id = NULL;
    }
    return rc;
}

hpd_error_t hpd_adapter_id_next_service_id(hpd_service_id_t **service_id)
{
    if (!service_id || !(*service_id)) return HPD_E_NULL;
    const hpd_module_t *context = (*service_id)->device.adapter.context;
    hpd_t *hpd = context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(*service_id, &service))) return rc;

    if ((rc = discovery_next_adapter_service(&service))) return rc;

    if (!service) {
        discovery_free_sid(*service_id);
        *service_id = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_device_t *device = service->device;
    hpd_adapter_t *adapter = device->adapter;
    if ((rc = discovery_set_sid(*service_id, context, adapter->id, device->id, service->id))) {
        discovery_free_sid(*service_id);
        *service_id = NULL;
    }
    return rc;
}

// TODO ALL next functions should free their iterator on errors...
hpd_error_t hpd_device_id_next_service_id(hpd_service_id_t **service_id)
{
    if (!service_id || !(*service_id)) return HPD_E_NULL;
    const hpd_module_t *context = (*service_id)->device.adapter.context;
    hpd_t *hpd = context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_service_t *service;
    if ((rc = discovery_find_service(*service_id, &service))) return rc;

    if ((rc = discovery_next_device_service(&service))) return rc;

    if (!service) {
        discovery_free_sid(*service_id);
        *service_id = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_device_t *device = service->device;
    hpd_adapter_t *adapter = device->adapter;
    if ((rc = discovery_set_sid(*service_id, context, adapter->id, device->id, service->id))) {
        discovery_free_sid(*service_id);
        *service_id = NULL;
    }
    return rc;
}

hpd_error_t hpd_service_id_next_parameter_id(hpd_parameter_id_t **parameter_id)
{
    if (!parameter_id || !(*parameter_id)) return HPD_E_NULL;
    const hpd_module_t *context = (*parameter_id)->service.device.adapter.context;
    hpd_t *hpd = context->hpd;
    if (!hpd->configuration) LOG_RETURN_HPD_STOPPED(hpd);

    hpd_error_t rc;
    hpd_parameter_t *parameter;
    if ((rc = discovery_find_parameter(*parameter_id, &parameter))) return rc;

    if ((rc = discovery_next_service_parameter(&parameter))) return rc;

    if (!parameter) {
        discovery_free_pid(*parameter_id);
        *parameter_id = NULL;
        return HPD_E_SUCCESS;
    }

    hpd_service_t *service = parameter->service;
    hpd_device_t *device = service->device;
    hpd_adapter_t *adapter = device->adapter;
    if ((rc = discovery_set_pid(*parameter_id, context, adapter->id, device->id, service->id, parameter->id))) {
        discovery_free_pid(*parameter_id);
        *parameter_id = NULL;
    }
    return rc;
}

hpd_error_t hpd_device_get_adapter(const hpd_device_t *device, const hpd_adapter_t **adapter)
{
    if (!device) return HPD_E_NULL;
    if (!adapter) LOG_RETURN_E_NULL(device->context->hpd);
    return discovery_get_device_adapter(device, adapter);
}

hpd_error_t hpd_service_get_adapter(const hpd_service_t *service, const hpd_adapter_t **adapter)
{
    if (!service) return HPD_E_NULL;
    if (!adapter) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_get_service_adapter(service, adapter);
}

hpd_error_t hpd_service_get_device(const hpd_service_t *service, const hpd_device_t **device)
{
    if (!service) return HPD_E_NULL;
    if (!device) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_get_service_device(service, device);
}

hpd_error_t hpd_parameter_get_adapter(const hpd_parameter_t *parameter, const hpd_adapter_t **adapter)
{
    if (!parameter) return HPD_E_NULL;
    if (!adapter) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_get_parameter_adapter(parameter, adapter);
}

hpd_error_t hpd_parameter_get_device(const hpd_parameter_t *parameter, const hpd_device_t **device)
{
    if (!parameter) return HPD_E_NULL;
    if (!device) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_get_parameter_device(parameter, device);
}

hpd_error_t hpd_parameter_get_service(const hpd_parameter_t *parameter, const hpd_service_t **service)
{
    if (!parameter) return HPD_E_NULL;
    if (!service) LOG_RETURN_E_NULL(parameter->context->hpd);
    return discovery_get_parameter_service(parameter, service);
}

hpd_error_t hpd_adapter_get_device(const hpd_adapter_t *adapter, const char *id, hpd_device_t **device)
{
    if (!adapter) return HPD_E_NULL;
    if (!id || !device) LOG_RETURN_E_NULL(adapter->context->hpd);
    return  discovery_get_adapter_device(adapter, id, device);
}

hpd_error_t hpd_device_get_service(const hpd_device_t *device, const char *id, hpd_service_t **service)
{
    if (!device) return HPD_E_NULL;
    if (!id || !service) LOG_RETURN_E_NULL(device->context->hpd);
    return  discovery_get_device_service(device, id, service);
}

hpd_error_t hpd_service_get_parameter(const hpd_service_t *service, const char *id, hpd_parameter_t **parameter)
{
    if (!service) return HPD_E_NULL;
    if (!id || !parameter) LOG_RETURN_E_NULL(service->context->hpd);
    return  discovery_get_service_parameter(service, id, parameter);
}

hpd_error_t hpd_adapter_first_device(const hpd_adapter_t *adapter, hpd_device_t **device)
{
    if (!adapter) return HPD_E_NULL;
    if (!device) LOG_RETURN_E_NULL(adapter->context->hpd);
    return discovery_first_adapter_device(adapter, device);
}

hpd_error_t hpd_adapter_first_service(const hpd_adapter_t *adapter, hpd_service_t **service)
{
    if (!adapter) return HPD_E_NULL;
    if (!service) LOG_RETURN_E_NULL(adapter->context->hpd);
    return discovery_first_adapter_service(adapter, service);
}

hpd_error_t hpd_device_first_service(const hpd_device_t *device, hpd_service_t **service)
{
    if (!device) return HPD_E_NULL;
    if (!service) LOG_RETURN_E_NULL(device->context->hpd);
    return discovery_first_device_service(device, service);
}

hpd_error_t hpd_service_first_parameter(const hpd_service_t *service, hpd_parameter_t **parameter)
{
    if (!service) return HPD_E_NULL;
    if (!parameter) LOG_RETURN_E_NULL(service->context->hpd);
    return discovery_first_service_parameter(service, parameter);
}

hpd_error_t hpd_adapter_next_device(hpd_device_t **device)
{
    if (!device || !(*device)) return HPD_E_NULL;
    return discovery_next_adapter_device(device);
}

hpd_error_t hpd_adapter_next_service(hpd_service_t **service)
{
    if (!service || !(*service)) return HPD_E_NULL;
    return discovery_next_adapter_service(service);
}

hpd_error_t hpd_device_next_service(hpd_service_t **service)
{
    if (!service || !(*service)) return HPD_E_NULL;
    return discovery_next_device_service(service);
}

hpd_error_t hpd_service_next_parameter(hpd_parameter_t **parameter)
{
    if (!parameter || !(*parameter)) return HPD_E_NULL;
    return discovery_next_service_parameter(parameter);
}
