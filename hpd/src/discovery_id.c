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

#include "discovery.h"
#include "hpd/common/hpd_common.h"
#include "daemon.h"
#include "log.h"
#include "model.h"

/**
 * HPD_E_ALLOC: id might be partially updated
 */
hpd_error_t discovery_set_aid(hpd_adapter_id_t *id, hpd_t *hpd, const char *aid)
{
    id->hpd = hpd;
    HPD_STR_CPY(id->aid, aid);
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC();
}

/**
 * HPD_E_ALLOC: id might be partially updated
 */
hpd_error_t discovery_set_did(hpd_device_id_t *id, hpd_t *hpd, const char *aid, const char *did)
{
    hpd_error_t rc;
    if ((rc = discovery_set_aid(&id->adapter, hpd, aid))) return rc;
    HPD_STR_CPY(id->did, did);
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC();
}

/**
 * HPD_E_ALLOC: id might be partially updated
 */
hpd_error_t discovery_set_sid(hpd_service_id_t *id, hpd_t *hpd, const char *aid, const char *did, const char *sid)
{
    hpd_error_t rc;
    if ((rc = discovery_set_did(&id->device, hpd, aid, did))) return rc;
    HPD_STR_CPY(id->sid, sid);
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC();
}

/**
 * HPD_E_ALLOC: id might be partially updated
 */
hpd_error_t discovery_set_pid(hpd_parameter_id_t *id, hpd_t *hpd, const char *aid, const char *did, const char *sid, const char *pid)
{
    hpd_error_t rc;
    if ((rc = discovery_set_sid(&id->service, hpd, aid, did, sid))) return rc;
    HPD_STR_CPY(id->pid, pid);
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC();
}

hpd_error_t discovery_alloc_aid(hpd_adapter_id_t **id, hpd_t *hpd, const char *aid)
{
    hpd_error_t rc;
    HPD_CALLOC(*id, 1, hpd_adapter_id_t);
    if ((rc = discovery_set_aid(*id, hpd, aid))) {
        discovery_free_aid(*id);
        (*id) = NULL;
        return rc;
    }
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC();
}

hpd_error_t discovery_alloc_did(hpd_device_id_t **id, hpd_t *hpd, const char *aid, const char *did)
{
    hpd_error_t rc;
    HPD_CALLOC(*id, 1, hpd_device_id_t);
    if ((rc = discovery_set_did(*id, hpd, aid, did))) {
        discovery_free_did(*id);
        (*id) = NULL;
        return rc;
    }
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC();
}

hpd_error_t discovery_alloc_sid(hpd_service_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid)
{
    hpd_error_t rc;
    HPD_CALLOC(*id, 1, hpd_service_id_t);
    if ((rc = discovery_set_sid(*id, hpd, aid, did, sid))) {
        discovery_free_sid(*id);
        (*id) = NULL;
        return rc;
    }
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC();
}

hpd_error_t discovery_alloc_pid(hpd_parameter_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid,
                                const char *pid)
{
    hpd_error_t rc;
    HPD_CALLOC(*id, 1, hpd_parameter_id_t);
    if ((rc = discovery_set_pid(*id, hpd, aid, did, sid, pid))) {
        discovery_free_pid(*id);
        (*id) = NULL;
        return rc;
    }
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC();
}

hpd_error_t discovery_copy_aid(hpd_adapter_id_t **dst, const hpd_adapter_id_t *src)
{
    return discovery_alloc_aid(dst, src->hpd, src->aid);
}

hpd_error_t discovery_copy_did(hpd_device_id_t **dst, const hpd_device_id_t *src)
{
    return discovery_alloc_did(dst, src->adapter.hpd, src->adapter.aid, src->did);
}

hpd_error_t discovery_copy_sid(hpd_service_id_t **dst, const hpd_service_id_t *src)
{
    return discovery_alloc_sid(dst, src->device.adapter.hpd, src->device.adapter.aid, src->device.did, src->sid);
}

hpd_error_t discovery_copy_pid(hpd_parameter_id_t **dst, const hpd_parameter_id_t *src)
{
    return discovery_alloc_pid(dst, src->service.device.adapter.hpd, src->service.device.adapter.aid, src->service.device.did, src->service.sid, src->pid);
}


hpd_error_t discovery_free_aid(hpd_adapter_id_t *id)
{
    free(id->aid);
    free(id);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_free_did(hpd_device_id_t *id)
{
    free(id->adapter.aid);
    free(id->did);
    free(id);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_free_sid(hpd_service_id_t *id)
{
    free(id->device.adapter.aid);
    free(id->device.did);
    free(id->sid);
    free(id);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_free_pid(hpd_parameter_id_t *id)
{
    free(id->service.device.adapter.aid);
    free(id->service.device.did);
    free(id->service.sid);
    free(id->pid);
    free(id);
    return HPD_E_SUCCESS;
}

#define DISCOVERY_FIND_ADAPTER(ID, ADAPTER) do { \
    (ADAPTER) = NULL; \
    TAILQ_FOREACH((ADAPTER), &(ID)->hpd->configuration->adapters, HPD_TAILQ_FIELD) \
        if (strcmp((ADAPTER)->id, (ID)->aid) == 0) break; \
    if (!(ADAPTER)) return HPD_E_NOT_FOUND; \
} while (0)

#define DISCOVERY_FIND_DEVICE(ID, DEVICE) do { \
    hpd_adapter_t *a = NULL; \
    DISCOVERY_FIND_ADAPTER(&(ID)->adapter, a); \
    (DEVICE) = NULL; \
    TAILQ_FOREACH((DEVICE), a->devices, HPD_TAILQ_FIELD) \
        if (strcmp((DEVICE)->id, (ID)->did) == 0) break; \
    if (!(DEVICE)) return HPD_E_NOT_FOUND; \
} while (0)

#define DISCOVERY_FIND_SERVICE(ID, SERVICE) do { \
    hpd_device_t *d = NULL; \
    DISCOVERY_FIND_DEVICE(&(ID)->device, d); \
    (SERVICE) = NULL; \
    TAILQ_FOREACH((SERVICE), d->services, HPD_TAILQ_FIELD) \
        if (strcmp((SERVICE)->id, (ID)->sid) == 0) break; \
    if (!(SERVICE)) return HPD_E_NOT_FOUND; \
} while (0)

#define DISCOVERY_FIND_PARAMETER(ID, PARAMETER) do { \
    hpd_service_t *s = NULL; \
    DISCOVERY_FIND_SERVICE(&(ID)->service, s); \
    (PARAMETER) = NULL; \
    TAILQ_FOREACH((PARAMETER), s->parameters, HPD_TAILQ_FIELD) \
        if (strcmp((PARAMETER)->id, (ID)->pid) == 0) break; \
    if (!(PARAMETER)) return HPD_E_NOT_FOUND; \
} while (0)

hpd_error_t discovery_find_adapter(const hpd_adapter_id_t *id, hpd_adapter_t **adapter)
{
    DISCOVERY_FIND_ADAPTER(id, *adapter);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_find_device(const hpd_device_id_t *id, hpd_device_t **device)
{
    DISCOVERY_FIND_DEVICE(id, *device);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_find_service(const hpd_service_id_t *id, hpd_service_t **service)
{
    DISCOVERY_FIND_SERVICE(id, *service);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_find_parameter(const hpd_parameter_id_t *id, hpd_parameter_t **parameter)
{
    DISCOVERY_FIND_PARAMETER(id, *parameter);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_aid_hpd(const hpd_adapter_id_t *aid, hpd_t **hpd)
{
    (*hpd) = aid->hpd;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_did_hpd(const hpd_device_id_t *did, hpd_t **hpd)
{
    (*hpd) = did->adapter.hpd;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_did_adapter(const hpd_device_id_t *did, const hpd_adapter_id_t **aid)
{
    (*aid) = &did->adapter;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_sid_hpd(const hpd_service_id_t *sid, hpd_t **hpd)
{
    (*hpd) = sid->device.adapter.hpd;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_sid_adapter(const hpd_service_id_t *sid, const hpd_adapter_id_t **aid)
{
    (*aid) = &sid->device.adapter;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_sid_device(const hpd_service_id_t *sid, const hpd_device_id_t **did)
{
    (*did) = &sid->device;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_pid_hpd(const hpd_parameter_id_t *pid, hpd_t **hpd)
{
    (*hpd) = pid->service.device.adapter.hpd;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_pid_adapter(const hpd_parameter_id_t *pid, const hpd_adapter_id_t **aid)
{
    (*aid) = &pid->service.device.adapter;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_pid_device(const hpd_parameter_id_t *pid, const hpd_device_id_t **did)
{
    (*did) = &pid->service.device;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_pid_service(const hpd_parameter_id_t *pid, const hpd_service_id_t **sid)
{
    (*sid) = &pid->service;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_aid_aid(const hpd_adapter_id_t *adapter, const char **id)
{
    (*id) = adapter->aid;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_did_aid(const hpd_device_id_t *device, const char **id)
{
    (*id) = device->adapter.aid;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_did_did(const hpd_device_id_t *device, const char **id)
{
    (*id) = device->did;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_sid_aid(const hpd_service_id_t *service, const char **id)
{
    (*id) = service->device.adapter.aid;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_sid_did(const hpd_service_id_t *service, const char **id)
{
    (*id) = service->device.did;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_sid_sid(const hpd_service_id_t *service, const char **id)
{
    (*id) = service->sid;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_pid_aid(const hpd_parameter_id_t *parameter, const char **id)
{
    (*id) = parameter->service.device.adapter.aid;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_pid_did(const hpd_parameter_id_t *parameter, const char **id)
{
    (*id) = parameter->service.device.did;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_pid_sid(const hpd_parameter_id_t *parameter, const char **id)
{
    (*id) = parameter->service.sid;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_pid_pid(const hpd_parameter_id_t *parameter, const char **id)
{
    (*id) = parameter->pid;
    return HPD_E_SUCCESS;
}

