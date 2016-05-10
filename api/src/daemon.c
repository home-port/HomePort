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

#include "request.h"
#include "discovery.h"
#include "daemon.h"
#include "event.h"
#include "value.h"
#include "log.h"

static void sig_cb(hpd_ev_loop_t *loop, ev_signal *w, int revents)
{
    ev_break(loop, EVBREAK_ALL);
}

static int parse_opt(int key, char *arg, struct argp_state *state)
{
    hpd_error_t rc;
    hpd_t *hpd = state->input;

    if (key >= 0xff && (key - 0xff) < hpd->options_count) {
        hpd_module_t *module = hpd->option2module[key-0xff];
        const char *name = hpd->options[key - 0xff].name;
        while ((name++)[0] != '-');
        switch ((rc = module->def.on_parse_opt(module->data, name, arg))) {
            case HPD_E_SUCCESS:
                return 0;
            case HPD_E_ARGUMENT:
                LOG_DEBUG("Module '%s' did not recognise the option '%s'.", module->id, name);
                return ARGP_ERR_UNKNOWN;
            default:
                return rc;
        }
    }

    return 0;
}

static hpd_error_t daemon_alloc_conf(configuration_t **conf, void *data)
{
    HPD_CALLOC(*conf, 1, configuration_t);
    TAILQ_INIT(&(*conf)->adapters);
    TAILQ_INIT(&(*conf)->listeners);
    (*conf)->data = (data);
    return HPD_E_SUCCESS;

    alloc_error:
        LOG_RETURN_E_ALLOC();
}

static hpd_error_t daemon_free_conf(configuration_t *conf) {
    hpd_error_t rc;
    HPD_TAILQ_MAP_REMOVE(&conf->adapters, discovery_free_adapter, hpd_adapter_t, rc);
    HPD_TAILQ_MAP_REMOVE(&conf->listeners, event_free_listener, hpd_listener_t, rc);
    free(conf);
    return HPD_E_SUCCESS;

    map_error:
        // TODO This is bad ...
        return rc;
}

hpd_error_t daemon_alloc(hpd_t **hpd)
{
    HPD_CALLOC(*hpd, 1, hpd_t);
    TAILQ_INIT(&(*hpd)->modules);
    TAILQ_INIT(&(*hpd)->request_watchers);
    TAILQ_INIT(&(*hpd)->respond_watchers);
    TAILQ_INIT(&(*hpd)->changed_watchers);
    TAILQ_INIT(&(*hpd)->attached_watchers);
    TAILQ_INIT(&(*hpd)->detached_watchers);
    ev_signal_init(&(*hpd)->sigint_watcher, sig_cb, SIGINT);
    ev_signal_init(&(*hpd)->sigterm_watcher, sig_cb, SIGTERM);
    (*hpd)->sigint_watcher.data = hpd;
    (*hpd)->sigterm_watcher.data = hpd;
    return HPD_E_SUCCESS;

    alloc_error:
        if (*hpd) {
            if ((*hpd)->options) free((*hpd)->options);
            free(*hpd);
        }
    LOG_RETURN_E_ALLOC();
}

hpd_error_t daemon_free(hpd_t *hpd)
{
    hpd_module_t *module, *module_tmp;
    HPD_TAILQ_FOREACH_SAFE(module, &hpd->modules, module_tmp) {
        TAILQ_REMOVE(&hpd->modules, module, HPD_TAILQ_FIELD);
        free(module->id);
        free(module);
    }
    hpd_ev_async_t *async, *async_tmp;
    HPD_TAILQ_FOREACH_SAFE(async, &hpd->request_watchers, async_tmp) {
        TAILQ_REMOVE(&hpd->request_watchers, async, HPD_TAILQ_FIELD);
        ev_async_stop(hpd->loop, &async->watcher);
        request_free_request(async->request);
        free(async);
    }
    HPD_TAILQ_FOREACH_SAFE(async, &hpd->respond_watchers, async_tmp) {
        TAILQ_REMOVE(&hpd->respond_watchers, async, HPD_TAILQ_FIELD);
        ev_async_stop(hpd->loop, &async->watcher);
        request_free_response(async->response);
        free(async);
    }
    HPD_TAILQ_FOREACH_SAFE(async, &hpd->changed_watchers, async_tmp) {
        TAILQ_REMOVE(&hpd->changed_watchers, async, HPD_TAILQ_FIELD);
        ev_async_stop(hpd->loop, &async->watcher);
        discovery_free_sid(async->service);
        value_free(async->value);
        free(async);
    }
    HPD_TAILQ_FOREACH_SAFE(async, &hpd->attached_watchers, async_tmp) {
        TAILQ_REMOVE(&hpd->attached_watchers, async, HPD_TAILQ_FIELD);
        ev_async_stop(hpd->loop, &async->watcher);
        discovery_free_did(async->device);
        free(async);
    }
    HPD_TAILQ_FOREACH_SAFE(async, &hpd->detached_watchers, async_tmp) {
        TAILQ_REMOVE(&hpd->detached_watchers, async, HPD_TAILQ_FIELD);
        ev_async_stop(hpd->loop, &async->watcher);
        discovery_free_did(async->device);
        free(async);
    }
    free(hpd);
    return HPD_E_SUCCESS;
}

hpd_error_t daemon_add_module(hpd_t *hpd, const char *id, hpd_module_def_t *module_def)
{
    hpd_module_t *module;
    HPD_CALLOC(module, 1, hpd_module_t);
    module->hpd = hpd;
    module->def = *module_def;
    HPD_STR_CPY(module->id, id);
    TAILQ_INSERT_TAIL(&hpd->modules, module, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;

    alloc_error:
    if (module) free(module);
    LOG_RETURN_E_ALLOC();
}

hpd_error_t daemon_add_option(hpd_module_t *context, const char *name, const char *arg, int flags, const char *doc)
{
    hpd_t *hpd = context->hpd;
    argp_option_t option = { 0 };
    argp_option_t empty = { 0 };
    char *name_alloc, *arg_alloc = NULL, *doc_alloc = NULL;

    HPD_CALLOC(name_alloc, strlen(name)+strlen(context->id)+2, char);
    strcpy(name_alloc, context->id);
    strcat(name_alloc, "-");
    strcat(name_alloc, name);
    if (arg) HPD_STR_CPY(arg_alloc, arg);
    if (doc) HPD_STR_CPY(doc_alloc, doc);
    option.name = name_alloc;
    option.key = 0xff + hpd->options_count;
    option.arg = arg_alloc;
    option.flags = flags;
    option.doc = doc_alloc;

    HPD_REALLOC(hpd->options, hpd->options_count+2, argp_option_t);
    HPD_REALLOC(hpd->option2module, hpd->options_count+1, hpd_module_t *);

    hpd->option2module[hpd->options_count] = context;
    hpd->options[hpd->options_count] = option;
    hpd->options[hpd->options_count+1] = empty;
    hpd->options_count++;

    return HPD_E_SUCCESS;

    alloc_error:
        if (name_alloc) free(name_alloc);
    if (arg_alloc) free(arg_alloc);
    if (doc_alloc) free(doc_alloc);
    LOG_RETURN_E_ALLOC();
}

hpd_error_t daemon_start(hpd_t *hpd, int argc, char *argv[])
{
    hpd_error_t rc;
    hpd_module_t *module;

    // Allocate run-time and option memory
    if ((rc = daemon_alloc_conf(&hpd->configuration, hpd))) goto return_error;
    rc = HPD_E_ALLOC;
    HPD_CALLOC(hpd->options, 1, argp_option_t);

    // Call on_create() on modules
    HPD_TAILQ_FOREACH(module, &hpd->modules)
        if ((rc = module->def.on_create(&module->data, module)) != HPD_E_SUCCESS)
            goto module_create_error;

    // Parse options
    struct argp argp = {hpd->options, parse_opt };
    if (argp_parse(&argp, argc, argv, 0, 0, hpd)) {
        LOG_DEBUG("Error while parsing arguments.");
        rc = HPD_E_ARGUMENT;
        goto arg_error;
    }

    // Deallocate option memory
    argp_option_t empty = { 0 };
    for (argp_option_t *option = hpd->options; memcmp(option, &empty, sizeof(argp_option_t)); option++) {
        free((void *) option->name);
        free((void *) option->arg);
        free((void *) option->doc);
    }
    free(hpd->options);
    hpd->options = NULL;
    free(hpd->option2module);
    hpd->option2module = NULL;

    // Create event loop
    hpd->loop = ev_loop_new(EVFLAG_AUTO);
    if (!hpd->loop) {
        rc = HPD_E_ALLOC;
        goto ev_new_error;
    }

    // Call on_start() on modules
    HPD_TAILQ_FOREACH(module, &hpd->modules)
        if ((rc = module->def.on_start(module->data, hpd)) != HPD_E_SUCCESS)
            goto module_start_error;

    // Start signal watchers
    ev_signal_start(hpd->loop, &hpd->sigint_watcher);
    ev_signal_start(hpd->loop, &hpd->sigterm_watcher);

    // Run until we are stopped
    // TODO Return value of ev_run can be used to run until all watchers have stopped (e.g. nice shutdown)
    ev_run(hpd->loop, 0);

    // Stop signal watchers
    ev_signal_stop(hpd->loop, &hpd->sigterm_watcher);
    ev_signal_stop(hpd->loop, &hpd->sigint_watcher);

    // Call on_stop() on modules
    TAILQ_FOREACH_REVERSE(module, &hpd->modules, modules, HPD_TAILQ_FIELD)
        if ((rc = module->def.on_stop(module->data, hpd)) != HPD_E_SUCCESS)
            goto module_stop_error;

    // Destroy event loop
    ev_loop_destroy(hpd->loop);
    hpd->loop = NULL;

    // Call on_destroy on modules
    TAILQ_FOREACH_REVERSE(module, &hpd->modules, modules, HPD_TAILQ_FIELD)
        if (module->data) {
            if ((rc = module->def.on_destroy(module->data)) != HPD_E_SUCCESS)
                goto module_destroy_error;
            module->data = NULL;
        }

    // Deallocate run-time memory
    if ((rc = daemon_free_conf(hpd->configuration))) goto return_error;
    hpd->configuration = NULL;

    // Return
    return HPD_E_SUCCESS;

    // Return in a nice state if an error occur
    module_start_error:
    module_stop_error:
        for (module = TAILQ_PREV(module, modules, HPD_TAILQ_FIELD); module; module = TAILQ_PREV(module, modules, HPD_TAILQ_FIELD))
            module->def.on_stop(module->data, hpd);
    ev_new_error:
    arg_error:
        module = TAILQ_LAST(&hpd->modules, modules);
    module_create_error:
    module_destroy_error:
        for (; module; module = TAILQ_PREV(module, modules, HPD_TAILQ_FIELD))
            if (module->data) module->def.on_destroy(module->data);
    alloc_error:
        if (hpd->configuration) {
            daemon_free_conf(hpd->configuration);
            hpd->configuration = NULL;
        }
        if (hpd->options) {
            free(hpd->options);
            hpd->options = NULL;
        }
    return_error:
        switch (rc) {
            case HPD_E_ALLOC:
                LOG_RETURN_E_ALLOC();
            default:
                return rc;
        }
}

hpd_error_t daemon_stop(const hpd_t *hpd)
{
    ev_break(hpd->loop, EVBREAK_ALL);
    return HPD_E_SUCCESS;
}

hpd_error_t daemon_get_id(hpd_module_t *context, const char **id)
{
    (*id) = context->id;
    return HPD_E_SUCCESS;
}

hpd_error_t daemon_get_loop(const hpd_t *hpd, hpd_ev_loop_t **loop)
{
    (*loop) = hpd->loop;
    return HPD_E_SUCCESS;
}