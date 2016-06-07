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

#include "hpd_curl_ev_module.h"
#include "curl_ev_intern.h"
#include "hpd_shared_api.h"
#include "hpd_common.h"
#include <ev.h>

typedef struct curl_ev_io curl_ev_io_t;

struct curl_ev_io {
    TAILQ_ENTRY(curl_ev_io) HPD_TAILQ_FIELD;
    ev_io watcher;
};

struct curl_ev {
    CURLM *mult_handle;
    ev_timer timer;
    TAILQ_HEAD(, curl_ev_io) io_watchers;
    TAILQ_HEAD(curl_ev_handles, curl_ev_handle) handles;
    hpd_ev_loop_t *loop;
    const hpd_module_t *context;
};

static hpd_error_t curl_ev_on_create(void **data, const hpd_module_t *context);
static hpd_error_t curl_ev_on_destroy(void *data);
static hpd_error_t curl_ev_on_start(void *data, hpd_t *hpd);
static hpd_error_t curl_ev_on_stop(void *data, hpd_t *hpd);
static hpd_error_t curl_ev_on_parse_opt(void *data, const char *name, const char *arg);

static CURLMcode curl_ev_socket_action(int sockfd);

hpd_module_def_t hpd_curl_ev = {
        curl_ev_on_create,
        curl_ev_on_destroy,
        curl_ev_on_start,
        curl_ev_on_stop,
        curl_ev_on_parse_opt
};

static curl_ev_t *curl_ev = NULL;

#define CURL_EV_CHECK() do { \
    if (!curl_ev) { \
        HPD_LOG_ERROR(handle->context, "Curl not initialised"); \
        HPD_LOG_RETURN(handle->context, HPD_E_STATE, "Did you remember to add the curl module to hpd?"); \
    } \
} while(0)

static void curl_ev_on_io(hpd_ev_loop_t *loop, ev_io *w, int revents)
{
    CURLMcode cmc;
    if ((cmc = curl_ev_socket_action(w->fd)))
        HPD_LOG_ERROR(curl_ev->context, "curl_ev_socket_action() failed [code: %i]", cmc);
}

static void curl_ev_on_timeout(hpd_ev_loop_t *loop, ev_timer *w, int revents)
{
    ev_timer_stop(loop, w);
    CURLMcode cmc;
    if ((cmc = curl_ev_socket_action(CURL_SOCKET_TIMEOUT)))
        HPD_LOG_ERROR(curl_ev->context, "curl_ev_socket_action() failed [code: %i]", cmc);
}

static CURLMcode curl_ev_on_update_socket(CURL *easy, curl_socket_t s, int what, void *userp, void *socketp)
{
    CURLMcode cmc;
    const hpd_module_t *context = curl_ev->context;

    // Assign / Alloc
    curl_ev_io_t *w = NULL;
    switch (what) {
        case CURL_POLL_IN:
        case CURL_POLL_OUT:
        case CURL_POLL_INOUT:
            if (socketp) {
                w = socketp;
                ev_io_stop(curl_ev->loop, &w->watcher);
            } else {
                HPD_CALLOC(w, 1, curl_ev_io_t);
                if ((cmc = curl_multi_assign(curl_ev->mult_handle, s, w))) {
                    HPD_LOG_ERROR(context, "Curl multi return an error [code: %i]", cmc);
                    free(w);
                    return cmc;
                }
                TAILQ_INSERT_TAIL(&curl_ev->io_watchers, w, HPD_TAILQ_FIELD);
                ev_init(&w->watcher, curl_ev_on_io);
            }
            break;
        case CURL_POLL_REMOVE:
            w = socketp;
            break;
        default:
            HPD_LOG_ERROR(context, "Should not be here");
            return CURLM_INTERNAL_ERROR;
    }

    // Configure
    switch (what) {
        case CURL_POLL_IN:
            ev_io_set(&w->watcher, s, EV_READ);
            break;
        case CURL_POLL_OUT:
            ev_io_set(&w->watcher, s, EV_WRITE);
            break;
        case CURL_POLL_INOUT:
            ev_io_set(&w->watcher, s, EV_READ | EV_WRITE);
            break;
        case CURL_POLL_REMOVE:
            break;
    }

    // Start/stop
    switch (what) {
        case CURL_POLL_IN:
        case CURL_POLL_OUT:
        case CURL_POLL_INOUT:
            ev_io_start(curl_ev->loop, &w->watcher);
            break;
        case CURL_POLL_REMOVE:
            ev_io_stop(curl_ev->loop, &w->watcher);
            TAILQ_REMOVE(&curl_ev->io_watchers, w, HPD_TAILQ_FIELD);
            free(w);
            break;
    }

    return CURLM_OK;

    alloc_error:
        return CURLM_OUT_OF_MEMORY;
}

static CURLMcode curl_ev_on_update_timer(CURLM *multi, long timeout_ms, void *userp)
{
    CURLMcode cmc;
    
    if (timeout_ms == 0) {
        if ((cmc = curl_ev_socket_action(CURL_SOCKET_TIMEOUT))) {
            HPD_LOG_ERROR(curl_ev->context, "curl_ev_socket_action() failed [code: %i]", cmc);
            return cmc;
        }
    } else if (timeout_ms > 0) {
        curl_ev->timer.repeat = timeout_ms / 1000.0;
        ev_timer_again(curl_ev->loop, &curl_ev->timer);
    } else {
        ev_timer_stop(curl_ev->loop, &curl_ev->timer);
    }

    return CURLM_OK;
}

static hpd_error_t curl_ev_add_next()
{
    CURLMcode cmc, cmc2;
    const hpd_module_t *context = curl_ev->context;

    if (!curl_ev->mult_handle) {
        HPD_LOG_VERBOSE(context, "Not started, saving handle for later...");
        return HPD_E_SUCCESS;
    }

    curl_ev_handle_t *handle = TAILQ_FIRST(&curl_ev->handles);
    if (handle) {
        if ((cmc = curl_multi_add_handle(curl_ev->mult_handle, handle->handle))) goto add_error;
        if ((cmc = curl_ev_socket_action(CURL_SOCKET_TIMEOUT))) goto action_error;
    }

    return HPD_E_SUCCESS;

    action_error:
    if ((cmc2 = curl_multi_remove_handle(curl_ev->mult_handle, handle->handle)))
        HPD_LOG_ERROR(context, "Curl remove handle return an error [code: %i]", cmc2);
    add_error:
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Curl multi return an error [code: %i]", cmc);
}

static CURLMcode curl_ev_socket_action(int sockfd)
{
    int unused;
    CURLMcode cmc;
    hpd_error_t rc;
    const hpd_module_t *context = curl_ev->context;

    if ((cmc = curl_multi_socket_action(curl_ev->mult_handle, sockfd, 0, &unused)))
        HPD_LOG_RETURN(context, cmc, "Curl multi return an error [code: %i]", cmc);

    CURLMsg *m;
    while ((m = curl_multi_info_read(curl_ev->mult_handle, &unused))) {
        switch (m->msg) {
            case CURLMSG_NONE: {
                break;
            }
            case CURLMSG_DONE: {
                CURL *easy_handle = m->easy_handle;
                curl_ev_handle_t *handle;
                TAILQ_FOREACH(handle, &curl_ev->handles, HPD_TAILQ_FIELD) {
                    if (handle->handle == easy_handle) {
                        CURLcode cc = m->data.result;
                        if (cc != CURLE_OK)
                            HPD_LOG_WARN(context, "Curl handle error: %s [code: %i]", curl_easy_strerror(cc), cc);
                        if (handle->on_done)
                            handle->on_done(handle->data, cc);
                        if ((rc = curl_ev_remove_handle(handle))) {
                            HPD_LOG_ERROR(context, "Failed to remove handle [code: %i]", rc);
                            return CURLM_INTERNAL_ERROR;
                        }
                        if ((rc = curl_ev_cleanup(handle))) {
                            HPD_LOG_ERROR(context, "Failed to remove handle [code: %i]", rc);
                            return CURLM_INTERNAL_ERROR;
                        }
                        break;
                    }
                }
                break;
            }
            case CURLMSG_LAST: {
                HPD_LOG_ERROR(context, "Should not be here");
                return CURLM_INTERNAL_ERROR;
            }
        }
    }

    return CURLM_OK;
}

hpd_error_t curl_ev_add_handle(curl_ev_handle_t *handle)
{
    if (!handle) HPD_LOG_RETURN_E_NULL(curl_ev->context);
    CURL_EV_CHECK();

    hpd_error_t rc;
    
    curl_ev_handle_t *h;
    TAILQ_FOREACH(h, &curl_ev->handles, HPD_TAILQ_FIELD)
        if (h == handle)
            HPD_LOG_RETURN(curl_ev->context, HPD_E_ARGUMENT, "Cannot add handle more than once");

    if (TAILQ_EMPTY(&curl_ev->handles)) {
        TAILQ_INSERT_TAIL(&curl_ev->handles, handle, HPD_TAILQ_FIELD);
        if ((rc = curl_ev_add_next())) {
            TAILQ_REMOVE(&curl_ev->handles, handle, HPD_TAILQ_FIELD);
            return rc;
        }
    } else {
        TAILQ_INSERT_TAIL(&curl_ev->handles, handle, HPD_TAILQ_FIELD); // Line duplication due to if condition
    }
    handle->curl_ev = curl_ev;
    return HPD_E_SUCCESS;
}

hpd_error_t curl_ev_remove_handle(curl_ev_handle_t *handle)
{
    if (!handle) HPD_LOG_RETURN_E_NULL(curl_ev->context);
    CURL_EV_CHECK();

    hpd_error_t rc;
    CURLMcode cmc;

    if (TAILQ_FIRST(&curl_ev->handles) == handle) {
        if ((cmc = curl_multi_remove_handle(curl_ev->mult_handle, handle->handle)))
            HPD_LOG_RETURN(curl_ev->context, HPD_E_UNKNOWN, "Curl multi return an error [code: %i]", cmc);
        TAILQ_REMOVE(&curl_ev->handles, handle, HPD_TAILQ_FIELD);
        if ((rc = curl_ev_add_next()))
            HPD_LOG_RETURN(curl_ev->context, HPD_E_SUCCESS, "Curl add next failed [code: %i]", rc);
    } else {
        TAILQ_REMOVE(&curl_ev->handles, handle, HPD_TAILQ_FIELD);
    }

    handle->curl_ev = NULL;
    return HPD_E_SUCCESS;
}

static hpd_error_t curl_ev_on_create(void **data, const hpd_module_t *context)
{
    if (!context) return HPD_E_NULL;
    if (curl_ev)
        HPD_LOG_RETURN(context, HPD_E_STATE, "Only one ínstance of curl_ev module allowed");

    HPD_CALLOC(curl_ev, 1, curl_ev_t);
    curl_ev->context = context;
    
    TAILQ_INIT(&curl_ev->handles);
    TAILQ_INIT(&curl_ev->io_watchers);

    ev_init(&curl_ev->timer, curl_ev_on_timeout);
    
    return HPD_E_SUCCESS;
    
    alloc_error:
        curl_global_cleanup();
        HPD_LOG_RETURN_E_ALLOC(context);
}

static hpd_error_t curl_ev_on_destroy(void *data)
{
    if (!curl_ev) return HPD_E_NULL;

    hpd_error_t rc;

    while (!TAILQ_EMPTY(&curl_ev->handles)) {
        curl_ev_handle_t *handle = TAILQ_LAST(&curl_ev->handles, curl_ev_handles);
        TAILQ_REMOVE(&curl_ev->handles, handle, HPD_TAILQ_FIELD);
        handle->curl_ev = NULL;
        if ((rc = curl_ev_cleanup(handle))) {
            HPD_LOG_ERROR(curl_ev->context, "Failed to remove handle [code: %i]", rc);
            return rc;
        }
    }
    
    free(curl_ev);
    curl_ev = NULL;
    
    return HPD_E_SUCCESS;
}

static hpd_error_t curl_ev_on_start(void *data, hpd_t *hpd)
{
    if (!curl_ev) return HPD_E_NULL;
    if (!hpd) HPD_LOG_RETURN_E_NULL(curl_ev->context);
    
    hpd_error_t rc;
    const hpd_module_t *context = curl_ev->context;

    CURLcode cc;
    if ((cc = curl_global_init(CURL_GLOBAL_ALL)))
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Curl failed to initialise [code: %i]", cc);
    // TODO Check supported features in curl_version_info

    hpd_ev_loop_t *loop;
    if ((rc = hpd_get_loop(hpd, &loop))) goto hpd_error;
    curl_ev->loop = loop;

    CURLMcode cmc;
    curl_ev->mult_handle = curl_multi_init();
    if ((cmc = curl_multi_setopt(curl_ev->mult_handle, CURLMOPT_SOCKETFUNCTION, curl_ev_on_update_socket))) goto curl_m_error;
    if ((cmc = curl_multi_setopt(curl_ev->mult_handle, CURLMOPT_SOCKETDATA, curl_ev))) goto curl_m_error;
    if ((cmc = curl_multi_setopt(curl_ev->mult_handle, CURLMOPT_TIMERFUNCTION, curl_ev_on_update_timer))) goto curl_m_error;
    if ((cmc = curl_multi_setopt(curl_ev->mult_handle, CURLMOPT_TIMERDATA, curl_ev))) goto curl_m_error;

    if ((rc = curl_ev_add_next())) goto next_error;

    return HPD_E_SUCCESS;

    hpd_error:
        curl_global_cleanup();
        return rc;
    curl_m_error:
        curl_ev->loop = NULL;
        curl_global_cleanup();
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Curl multi return an error [code: %i]", cmc);
    next_error:
        curl_ev->loop = NULL;
        curl_global_cleanup();
        return rc;
}

static hpd_error_t curl_ev_on_stop(void *data, hpd_t *hpd)
{
    if (!curl_ev) return HPD_E_NULL;
    
    CURLMcode cmc;
    const hpd_module_t *context = curl_ev->context;

    // Stop current handle
    curl_ev_handle_t *handle = TAILQ_FIRST(&curl_ev->handles);
    if (handle && (cmc = curl_multi_remove_handle(curl_ev->mult_handle, handle->handle)))
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Curl multi return an error [code: %i]", cmc);
    
    // Kill watchers
    ev_timer_stop(curl_ev->loop, &curl_ev->timer);
    curl_ev_io_t *io, *io_tmp;
    TAILQ_FOREACH_SAFE(io, &curl_ev->io_watchers, HPD_TAILQ_FIELD, io_tmp) {
        ev_io_stop(curl_ev->loop, &io->watcher);
        TAILQ_REMOVE(&curl_ev->io_watchers, io, HPD_TAILQ_FIELD);
        free(io);
    }

    // Stop curl multi
    if ((cmc = curl_multi_cleanup(curl_ev->mult_handle)))
        HPD_LOG_RETURN(context, HPD_E_UNKNOWN, "Curl multi return an error [code: %i]", cmc);
    
    return HPD_E_SUCCESS;
}

static hpd_error_t curl_ev_on_parse_opt(void *data, const char *name, const char *arg)
{
    if (!curl_ev) return HPD_E_NULL;
    
    return HPD_E_ARGUMENT;
}
