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

#include "discovery.h"
#include "hpd_common.h"
#include "daemon.h"

hpd_error_t discovery_alloc_aid(hpd_adapter_id_t **id, hpd_t *hpd, const char *aid)
{
    HPD_CALLOC(*id, 1, hpd_adapter_id_t);
    (*id)->hpd = hpd;
    HPD_STR_CPY((*id)->aid, aid);
    return HPD_E_SUCCESS;

    alloc_error:
    if (*id) discovery_free_aid(*id);
    return HPD_E_ALLOC;
}

hpd_error_t discovery_alloc_did(hpd_device_id_t **id, hpd_t *hpd, const char *aid, const char *did)
{
    HPD_CALLOC(*id, 1, hpd_device_id_t);
    (*id)->hpd = hpd;
    HPD_STR_CPY((*id)->aid, aid);
    HPD_STR_CPY((*id)->did, did);
    return HPD_E_SUCCESS;

    alloc_error:
    if (*id) discovery_free_did(*id);
    return HPD_E_ALLOC;
}

hpd_error_t discovery_alloc_sid(hpd_service_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid)
{
    HPD_CALLOC(*id, 1, hpd_service_id_t);
    (*id)->hpd = hpd;
    HPD_STR_CPY((*id)->aid, aid);
    HPD_STR_CPY((*id)->did, did);
    HPD_STR_CPY((*id)->sid, sid);
    return HPD_E_SUCCESS;

    alloc_error:
    if (*id) discovery_free_sid(*id);
    return HPD_E_ALLOC;
}

hpd_error_t discovery_alloc_pid(hpd_parameter_id_t **id, hpd_t *hpd, const char *aid, const char *did, const char *sid,
                                const char *pid)
{
    HPD_CALLOC(*id, 1, hpd_parameter_id_t);
    (*id)->hpd = hpd;
    HPD_STR_CPY((*id)->aid, aid);
    HPD_STR_CPY((*id)->did, did);
    HPD_STR_CPY((*id)->sid, sid);
    HPD_STR_CPY((*id)->pid, pid);
    return HPD_E_SUCCESS;

    alloc_error:
    if (*id) discovery_free_pid(*id);
    return HPD_E_ALLOC;
}

hpd_error_t discovery_copy_aid(hpd_adapter_id_t **dst, hpd_adapter_id_t *src)
{
    return discovery_alloc_aid(dst, src->hpd, src->aid);
}

hpd_error_t discovery_copy_did(hpd_device_id_t **dst, hpd_device_id_t *src)
{
    return discovery_alloc_did(dst, src->hpd, src->aid, src->did);
}

hpd_error_t discovery_copy_sid(hpd_service_id_t **dst, hpd_service_id_t *src)
{
    return discovery_alloc_sid(dst, src->hpd, src->aid, src->did, src->sid);
}

hpd_error_t discovery_copy_pid(hpd_parameter_id_t **dst, hpd_parameter_id_t *src)
{
    return discovery_alloc_pid(dst, src->hpd, src->aid, src->did, src->sid, src->pid);
}


hpd_error_t discovery_free_aid(hpd_adapter_id_t *id)
{
    free(id->aid);
    free(id);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_free_did(hpd_device_id_t *id)
{
    free(id->aid);
    free(id->did);
    free(id);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_free_sid(hpd_service_id_t *id)
{
    free(id->aid);
    free(id);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_free_pid(hpd_parameter_id_t *id)
{
    free(id->aid);
    free(id->did);
    free(id->sid);
    free(id->pid);
    free(id);
    return HPD_E_SUCCESS;
}

#define FIND_ADAPTER(ID, ADAPTER) do { \
    hpd_adapter_t *a = NULL; \
    HPD_TAILQ_FOREACH(a, &(ID)->hpd->configuration->adapters) \
        if (strcmp(a->id, (ID)->aid) == 0) break; \
    if (!a) return HPD_E_NOT_FOUND; \
    (ADAPTER) = a; \
} while (0)

#define FIND_DEVICE(ID, DEVICE) do { \
    hpd_adapter_t *a = NULL; \
    FIND_ADAPTER(id, a); \
    hpd_device_t *d = NULL; \
    HPD_TAILQ_FOREACH(d, a->devices) \
        if (strcmp(d->id, (ID)->did) == 0) break; \
    if (!d) return HPD_E_NOT_FOUND; \
    (DEVICE) = d; \
} while (0)

#define FIND_SERVICE(ID, SERVICE) do { \
    hpd_device_t *d = NULL; \
    FIND_DEVICE(id, d); \
    hpd_service_t *s = NULL; \
    HPD_TAILQ_FOREACH(s, d->services) \
        if (strcmp(s->id, (ID)->sid) == 0) break; \
    if (!s) return HPD_E_NOT_FOUND; \
    (SERVICE) = s; \
} while (0)

#define FIND_PARAMETER(ID, PARAMETER) do { \
    hpd_service_t *s = NULL; \
    FIND_SERVICE(id, s); \
    hpd_parameter_t *p = NULL; \
    HPD_TAILQ_FOREACH(p, s->parameters) \
        if (strcmp(p->id, (ID)->pid) == 0) break; \
    if (!p) return HPD_E_NOT_FOUND; \
    (PARAMETER) = p; \
} while (0)

hpd_error_t discovery_find_adapter(hpd_adapter_id_t *id, hpd_adapter_t **adapter)
{
    FIND_ADAPTER(id, *adapter);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_find_device(hpd_device_id_t *id, hpd_device_t **device)
{
    FIND_DEVICE(id, *device);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_find_service(hpd_service_id_t *id, hpd_service_t **service)
{
    FIND_SERVICE(id, *service);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_find_parameter(hpd_parameter_id_t *id, hpd_parameter_t **parameter)
{
    FIND_PARAMETER(id, *parameter);
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_adapter_hpd(hpd_adapter_id_t *aid, hpd_t **hpd)
{
    (*hpd) = aid->hpd;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_device_hpd(hpd_device_id_t *did, hpd_t **hpd)
{
    (*hpd) = did->hpd;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_device_adapter(hpd_device_id_t *did, hpd_adapter_id_t **aid)
{
    return discovery_alloc_aid(aid, did->hpd, did->aid);
}

hpd_error_t discovery_get_service_hpd(hpd_service_id_t *sid, hpd_t **hpd)
{
    (*hpd) = sid->hpd;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_service_adapter(hpd_service_id_t *sid, hpd_adapter_id_t **aid)
{
    return discovery_alloc_aid(aid, sid->hpd, sid->aid);
}

hpd_error_t discovery_get_service_device(hpd_service_id_t *sid, hpd_device_id_t **did)
{
    return discovery_alloc_did(did, sid->hpd, sid->aid, sid->did);
}

hpd_error_t discovery_get_parameter_hpd(hpd_parameter_id_t *pid, hpd_t **hpd)
{
    (*hpd) = pid->hpd;
    return HPD_E_SUCCESS;
}

hpd_error_t discovery_get_parameter_adapter(hpd_parameter_id_t *pid, hpd_adapter_id_t **aid)
{
    return discovery_alloc_aid(aid, pid->hpd, pid->aid);
}

hpd_error_t discovery_get_parameter_device(hpd_parameter_id_t *pid, hpd_device_id_t **did)
{
    return discovery_alloc_did(did, pid->hpd, pid->aid, pid->did);
}

hpd_error_t discovery_get_parameter_service(hpd_parameter_id_t *pid, hpd_service_id_t **sid)
{
    return discovery_alloc_sid(sid, pid->hpd, pid->aid, pid->did, pid->sid);
}

