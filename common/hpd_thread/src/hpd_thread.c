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
#include <semaphore.h>

#define HPD_LOG_RETURN_PTHREAD_CODE(CONTEXT, RC) HPD_LOG_RETURN((CONTEXT), HPD_E_UNKNOWN, "pthread failed [code: %i].", (RC))
#define HPD_LOG_RETURN_PTHREAD(CONTEXT, RC) do { HPD_LOG_DEBUG((CONTEXT), "pthread failed [code: %i].", (RC)); return; } while(0)

typedef struct thread thread_t;

struct thread {
    const hpd_module_t *context;
    ev_async async;
    hpd_ev_loop_t *loop;
    pthread_mutex_t mutex;
    sem_t sem_do_work;
    sem_t sem_cont_loop;
};

static hpd_error_t thread_on_create(void **data, const hpd_module_t *context);
static hpd_error_t thread_on_destroy(void *data);
static hpd_error_t thread_on_start(void *data);
static hpd_error_t thread_on_stop(void *data);
static hpd_error_t thread_on_parse_opt(void *data, const char *name, const char *arg);

hpd_module_def_t hpd_thread = {
        thread_on_create,
        thread_on_destroy,
        thread_on_start,
        thread_on_stop,
        thread_on_parse_opt
};

static thread_t *thread = NULL;

static void thread_on_ev_async(EV_P_ ev_async *w, int revents)
{
    int stat;

    if ((stat = sem_post(&thread->sem_do_work)))
        HPD_LOG_RETURN_PTHREAD(thread->context, stat);

    HPD_LOG_VERBOSE(thread->context, "Event loop paused");

    if ((stat = sem_wait(&thread->sem_cont_loop)))
        HPD_LOG_RETURN_PTHREAD(thread->context, stat);

    HPD_LOG_VERBOSE(thread->context, "Resuming event loop");
}

static hpd_error_t thread_on_create(void **data, const hpd_module_t *context)
{
    int stat;

    if (!context) return HPD_E_NULL;
    if (thread)
        HPD_LOG_RETURN(context, HPD_E_STATE, "Only one instance of hpd_thread module allowed");

    HPD_CALLOC(thread, 1, thread_t);
    thread->context = context;
    ev_async_init(&thread->async, thread_on_ev_async);
    if ((stat = pthread_mutex_init(&thread->mutex, NULL)))
        HPD_LOG_RETURN_PTHREAD_CODE(thread->context, stat);
    if ((stat = sem_init(&thread->sem_do_work, 0, 0)))
        HPD_LOG_RETURN_PTHREAD_CODE(thread->context, stat);
    if ((stat = sem_init(&thread->sem_cont_loop, 0, 0)))
        HPD_LOG_RETURN_PTHREAD_CODE(thread->context, stat);

    return HPD_E_SUCCESS;

    alloc_error:
    HPD_LOG_RETURN_E_ALLOC(context);
}

static hpd_error_t thread_on_destroy(void *data)
{
    int stat;

    if (!thread) return HPD_E_NULL;

    if ((stat = pthread_mutex_destroy(&thread->mutex)))
        HPD_LOG_RETURN_PTHREAD_CODE(thread->context, stat);
    if ((stat = sem_destroy(&thread->sem_do_work)))
        HPD_LOG_RETURN_PTHREAD_CODE(thread->context, stat);
    if ((stat = sem_destroy(&thread->sem_cont_loop)))
        HPD_LOG_RETURN_PTHREAD_CODE(thread->context, stat);

    free(thread);
    thread = NULL;

    return HPD_E_SUCCESS;
}

static hpd_error_t thread_on_start(void *data)
{
    if (!thread) return HPD_E_NULL;

    hpd_error_t rc;
    if ((rc = hpd_get_loop(thread->context, &thread->loop))) return rc;
    ev_async_start(thread->loop, &thread->async);

    return HPD_E_SUCCESS;
}

static hpd_error_t thread_on_stop(void *data)
{
    if (!thread) return HPD_E_NULL;
    return HPD_E_SUCCESS;
}

static hpd_error_t thread_on_parse_opt(void *data, const char *name, const char *arg)
{
    if (!thread) return HPD_E_NULL;
    return HPD_E_ARGUMENT;
}

hpd_error_t hpd_thread_lock(const hpd_module_t *context)
{
    if (!thread) {
        HPD_LOG_ERROR(context, "hpd_thread not initialised");
        HPD_LOG_RETURN(context, HPD_E_STATE, "Did you remember to add the hpd_thread module to hpd?");
    }

    int stat;

    if ((stat = pthread_mutex_lock(&thread->mutex)))
        HPD_LOG_RETURN_PTHREAD_CODE(context, stat);

    ev_async_send(thread->loop, &thread->async);

    if ((stat = sem_wait(&thread->sem_do_work)))
        HPD_LOG_RETURN_PTHREAD_CODE(context, stat);

    return HPD_E_SUCCESS;
}

hpd_error_t hpd_thread_unlock(const hpd_module_t *context)
{
    if (!thread) {
        HPD_LOG_ERROR(context, "hpd_thread not initialised");
        HPD_LOG_RETURN(context, HPD_E_STATE, "Did you remember to add the hpd_thread module to hpd?");
    }

    int stat;

    if ((stat = sem_post(&thread->sem_cont_loop)))
        HPD_LOG_RETURN_PTHREAD_CODE(context, stat);

    if ((stat = pthread_mutex_unlock(&thread->mutex)))
        HPD_LOG_RETURN_PTHREAD_CODE(context, stat);

    return HPD_E_SUCCESS;
}
