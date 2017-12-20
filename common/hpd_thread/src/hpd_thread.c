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

#include <hpd/common/hpd_thread.h>
#include <ev.h>
#include <hpd/hpd_shared_api.h>
#include <pthread.h>
#include <hpd/common/hpd_common.h>

#define HPD_LOG_RETURN_PTHREAD(CONTEXT, RC) HPD_LOG_RETURN((CONTEXT), HPD_E_UNKNOWN, "pthread failed [code: %i].", (RC))

typedef struct hpd_thread_lock hpd_thread_lock_t;

struct hpd_thread_lock {};

static void on_ev_async(EV_P_ ev_async *w, int revents)
{
    
}

hpd_error_t hpd_thread_lock_create(const hpd_module_t *context, hpd_thread_lock_t **lock)
{
    HPD_CALLOC(*lock, 1, hpd_thread_lock_t);
    return HPD_E_SUCCESS;

    alloc_error:
    HPD_LOG_RETURN_E_ALLOC(context);
}

hpd_error_t hpd_thread_destroy(hpd_thread_lock_t *lock)
{
    free(lock);
    return HPD_E_SUCCESS;
}

hpd_error_t hpd_thread_lock(hpd_thread_lock_t *lock)
{
    int stat;
    hpd_error_t rc;

    hpd_ev_loop_t *loop;
    if ((rc = hpd_get_loop(context, &loop))) return rc;

    pthread_mutex_t mutex;
    if ((stat = pthread_mutex_init(&mutex, NULL)))
        HPD_LOG_RETURN_PTHREAD(context, stat);

    if ((stat = pthread_mutex_lock(&mutex)))
        HPD_LOG_RETURN_PTHREAD(context, stat);

    ev_async async;
    ev_async_init(&async, on_ev_async);
    ev_async_send(loop, &async);

    // TODO THIS !
    return HPD_E_UNKNOWN;
}

hpd_error_t hpd_thread_unlock(hpd_thread_lock_t *lock)
{
    if ((stat = pthread_mutex_destroy(&mutex)))
        HPD_LOG_RETURN_PTHREAD(context, stat);

    // TODO THIS !
    return HPD_E_UNKNOWN;
}
