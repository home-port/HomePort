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

#include "daemon.h"

static void sig_cb(hpd_ev_loop_t *loop, ev_signal *w, int revents)
{
    ev_break(loop, EVBREAK_ALL);
}

static int parse_opt(int key, char *arg, struct argp_state *state)
{
    hpd_t *hpd = state->input;

    if (key >= 0xff && (key - 0xff) < hpd->options_count) {
        hpd_module_t *module = hpd->option2module[key-0xff];
        const char *name = hpd->options[key - 0xff].name;
        while ((name++)[0] != '-');
        module->def.on_parse_opt(module->data, name, arg);
    }

    return 0;
}

hpd_error_t daemon_alloc(hpd_t **hpd)
{
    CALLOC(*hpd, 1, hpd_t);
    TAILQ_INIT(&(*hpd)->modules);
    ev_signal_init(&(*hpd)->sigint_watcher, sig_cb, SIGINT);
    ev_signal_init(&(*hpd)->sigterm_watcher, sig_cb, SIGTERM);
    (*hpd)->sigint_watcher.data = hpd;
    (*hpd)->sigterm_watcher.data = hpd;
    (*hpd)->loop = ev_loop_new(EVFLAG_AUTO);
    return HPD_E_SUCCESS;

    alloc_error:
        if (*hpd) {
            if ((*hpd)->options) free((*hpd)->options);
            free(*hpd);
        }
    return HPD_E_ALLOC;
}

hpd_error_t daemon_free(hpd_t *hpd)
{
    hpd_module_t *module, *tmp;
    HPD_TAILQ_FOREACH_SAFE(module, &hpd->modules, tmp) {
        TAILQ_REMOVE(&hpd->modules, module, HPD_TAILQ_FIELD);
        free(module->id);
        free(module);
    }
    ev_loop_destroy(hpd->loop);
    free(hpd);
    return HPD_E_SUCCESS;
}

hpd_error_t daemon_add_module(hpd_t *hpd, const char *id, hpd_module_def_t *module_def)
{
    hpd_module_t *module;
    CALLOC(module, 1, hpd_module_t);
    module->hpd = hpd;
    module->def = *module_def;
    STR_CPY(module->id, id);
    TAILQ_INSERT_TAIL(&hpd->modules, module, HPD_TAILQ_FIELD);
    return HPD_E_SUCCESS;

    alloc_error:
    if (module) free(module);
    return HPD_E_ALLOC;
}

hpd_error_t daemon_add_option(hpd_module_t *context, const char *name, const char *arg, int flags, const char *doc)
{
    hpd_t *hpd = context->hpd;
    argp_option_t option = { 0 };
    argp_option_t empty = { 0 };
    char *name_alloc, *arg_alloc = NULL, *doc_alloc = NULL;

    CALLOC(name_alloc, strlen(name)+strlen(context->id)+2, char);
    strcpy(name_alloc, context->id);
    strcat(name_alloc, "-");
    strcat(name_alloc, name);
    if (arg) STR_CPY(arg_alloc, arg);
    if (doc) STR_CPY(doc_alloc, doc);
    option.name = name_alloc;
    option.key = 0xff + hpd->options_count;
    option.arg = arg_alloc;
    option.flags = flags;
    option.doc = doc_alloc;

    REALLOC(hpd->options, hpd->options_count+2, argp_option_t);
    REALLOC(hpd->option2module, hpd->options_count+1, hpd_module_t *);

    hpd->option2module[hpd->options_count] = context;
    hpd->options[hpd->options_count] = option;
    hpd->options[hpd->options_count+1] = empty;
    hpd->options_count++;

    return HPD_E_SUCCESS;

    alloc_error:
        if (name_alloc) free(name_alloc);
    if (arg_alloc) free(arg_alloc);
    if (doc_alloc) free(doc_alloc);
    return HPD_E_ALLOC;
}

hpd_error_t daemon_start(hpd_t *hpd, int argc, char *argv[])
{
    hpd_error_t rc;
    hpd_module_t *module;

    // Allocate run-time memory
    CONF_ALLOC(hpd->configuration, hpd);

    // Allocate option memory
    CALLOC(hpd->options, 1, argp_option_t);

    // Call on_create() on modules
    HPD_TAILQ_FOREACH(module, &hpd->modules)
        if ((rc = module->def.on_create(&module->data, module)) != HPD_E_SUCCESS)
            goto module_create_error;

    // Parse options
    struct argp argp = { hpd->options, parse_opt };
    if (argp_parse(&argp, argc, argv, 0, 0, hpd)) {
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
    free(hpd->option2module);
    hpd->options = NULL;
    hpd->option2module = NULL;

    // Call on_start() on modules
    HPD_TAILQ_FOREACH(module, &hpd->modules)
        if ((rc = module->def.on_start(module->data, hpd)) != HPD_E_SUCCESS)
            goto module_start_error;

    // Start signal watchers
    ev_signal_start(hpd->loop, &hpd->sigint_watcher);
    ev_signal_start(hpd->loop, &hpd->sigterm_watcher);

    // Run until we are stopped
    ev_run(hpd->loop, 0);

    // Stop signal watchers
    ev_signal_stop(hpd->loop, &hpd->sigterm_watcher);
    ev_signal_stop(hpd->loop, &hpd->sigint_watcher);

    // Call on_stop() on modules
    TAILQ_FOREACH_REVERSE(module, &hpd->modules, modules, HPD_TAILQ_FIELD)
        if ((rc = module->def.on_stop(module->data, hpd)) != HPD_E_SUCCESS)
            goto module_stop_error;

    // Call on_destroy on modules
    TAILQ_FOREACH_REVERSE(module, &hpd->modules, modules, HPD_TAILQ_FIELD)
        if (module->data) {
            if ((rc = module->def.on_destroy(module->data)) != HPD_E_SUCCESS)
                goto module_destroy_error;
            module->data = NULL;
        }

    // Deallocate run-time memory
    CONF_FREE(hpd->configuration);
    hpd->configuration = NULL;

    // Return
    return HPD_E_SUCCESS;

    // Return in a nice state if an error occur
    module_start_error:
    module_stop_error:
    for (module = TAILQ_PREV(module, modules, HPD_TAILQ_FIELD); module; module = TAILQ_PREV(module, modules, HPD_TAILQ_FIELD))
        module->def.on_stop(module->data, hpd);
    arg_error:
    module = TAILQ_LAST(&hpd->modules, modules);
    module_create_error:
    module_destroy_error:
    for (; module; module = TAILQ_PREV(module, modules, HPD_TAILQ_FIELD))
        if (module->data) module->def.on_destroy(module->data);
    return rc;

    alloc_error:
    if (hpd->configuration) {
        CONF_FREE(hpd->configuration);
        hpd->configuration = NULL;
    }
    if (hpd->options) {
        free(hpd->options);
        hpd->options = NULL;
    }
    return HPD_E_ALLOC;
}

hpd_error_t daemon_stop(const hpd_t *hpd)
{
    ev_break(hpd->loop, EVBREAK_ALL);
    return HPD_E_SUCCESS;
}

hpd_error_t daemon_get_loop(const hpd_t *hpd, hpd_ev_loop_t **loop)
{
    (*loop) = (hpd)->loop;
    return HPD_E_SUCCESS;
}