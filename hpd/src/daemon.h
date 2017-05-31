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

#ifndef HOMEPORT_DAEMON_H
#define HOMEPORT_DAEMON_H

#include "hpd/hpd_types.h"
#include "hpd/common/hpd_common.h"
#include "hpd/common/hpd_queue.h"
#include <ev.h>
#include <argp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hpd_configuration hpd_configuration_t;
typedef struct hpd_modules hpd_modules_t;
typedef struct argp_option hpd_argp_option_t;
typedef struct hpd_ev_async hpd_ev_async_t;
typedef struct hpd_ev_asyncs hpd_ev_asyncs_t;

TAILQ_HEAD(hpd_modules, hpd_module);
TAILQ_HEAD(hpd_ev_asyncs, hpd_ev_async);

struct hpd {
    hpd_ev_loop_t *loop;
    hpd_configuration_t *configuration;
    ev_signal sigint_watcher;
    ev_signal sigterm_watcher;
    hpd_modules_t modules;
    int module_options_count;
    int options_count;
    hpd_argp_option_t *options;
    const hpd_module_t **option2module;
    const char **option2name;
    hpd_ev_asyncs_t request_watchers;
    hpd_ev_asyncs_t respond_watchers;
    hpd_ev_asyncs_t changed_watchers;
    hpd_ev_asyncs_t device_watchers;
    char *argv0;
    hpd_log_level_t log_level;
#ifdef THREAD_SAFE
    pthread_mutex_t log_mutex;
#endif
};

struct hpd_ev_async {
    TAILQ_ENTRY(hpd_ev_async) HPD_TAILQ_FIELD;
    // TODO Using async watches wrongly here, one global watcher (maybe per queue) would be better
    ev_async watcher;
    union {
        hpd_request_t *request;
        hpd_response_t *response;
        struct {
            hpd_t *hpd;
            union {
                struct {
                    hpd_service_id_t *service;
                    hpd_value_t *value;
                };
                hpd_device_id_t *device;
            };
        };
    };
};

typedef struct hpd_module {
    hpd_t *hpd;
    TAILQ_ENTRY(hpd_module) HPD_TAILQ_FIELD;
    hpd_module_def_t def;
    char *id;
    void *data;
} hpd_module_t;

hpd_error_t daemon_alloc(hpd_t **hpd);
hpd_error_t daemon_free(hpd_t *hpd);
hpd_error_t daemon_add_module(hpd_t *hpd, const char *id, const hpd_module_def_t *module_def);
hpd_error_t daemon_add_option(const hpd_module_t *context, const char *name, const char *arg, int flags,
                              const char *doc);
hpd_error_t daemon_start(hpd_t *hpd, int argc, char *argv[]);
hpd_error_t daemon_stop(const hpd_t *hpd);
hpd_error_t daemon_get_id(const hpd_module_t *context, const char **id);
hpd_error_t daemon_get_loop(const hpd_t *hpd, hpd_ev_loop_t **loop);

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_DAEMON_H
