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

#include "request.h"
#include "discovery.h"
#include "daemon.h"
#include "event.h"
#include "value.h"
#include "log.h"
#include "model.h"

static hpd_error_t daemon_options_parse(hpd_t *hpd, int argc, char **argv);

static void daemon_on_signal(hpd_ev_loop_t *loop, ev_signal *w, int revents)
{
    ev_break(loop, EVBREAK_ALL);
}

static int daemon_on_parse_opt(int key, char *arg, struct argp_state *state)
{
    hpd_error_t rc;
    hpd_t *hpd = state->input;

    if (key >= 0xff && (key - 0xff) < hpd->module_options_count) {
        const hpd_module_t *module = hpd->option2module[key-0xff];
        const char *name = hpd->option2name[key - 0xff];
        while ((name++)[0] != '-');
        if (module->def.on_parse_opt) {
            switch ((rc = module->def.on_parse_opt(module->data, name, arg))) {
                case HPD_E_SUCCESS:
                    return 0;
                case HPD_E_ARGUMENT:
                    LOG_DEBUG(hpd, "Module '%s' did not recognise the option '%s'.", module->id, name);
                    return ARGP_ERR_UNKNOWN;
                default:
                    // TODO Wrong return type
                    return rc;
            }
        } else {
            LOG_DEBUG(hpd, "Module '%s' does not have options.", module->id);
            return ARGP_ERR_UNKNOWN;
        }
    }

    switch (key) {
        case 'q': {
            hpd->log_level = HPD_L_NONE;
            return 0;
        }
        case 'v': {
            hpd->log_level = HPD_L_VERBOSE;
            return 0;
        }
        case 'c': {
            // TODO Skipping some checks here...
            LOG_WARN(hpd, "Configuration files are still very much in an early alpha state.");
            char *buffer = NULL;
            FILE *fp = fopen(arg, "rb");
            if (fp) {
                fseek(fp, 0, SEEK_END);
                // TODO Puts a limit on file size (recheck the casting bit)
                size_t len = (size_t) ftell(fp);
                fseek(fp, 0, SEEK_SET);
                HPD_CALLOC(buffer, len+1, char);
                fread(buffer, 1, len, fp);
                fclose(fp);
            }
            if (buffer) {
                int argc = 1;
                char **argv = NULL;
                HPD_REALLOC(argv, argc, char *);
                argv[0] = hpd->argv0;
                for (char *a = strtok(buffer, " \n\t"); a; a = strtok(NULL, " \n\t")) {
                    HPD_REALLOC(argv, argc + 1, char *);
                    argv[argc] = a;
                    argc++;
                }
                daemon_options_parse(hpd, argc, argv); // TODO Check error
                free(argv);
            }
            return 0;
        }
        default:
            return ARGP_ERR_UNKNOWN;
    }

    alloc_error:
    // TODO What ?
    // TODO Free up a lot of stuff...
    return 0;
}

static hpd_error_t daemon_add_global_option(hpd_t *hpd, const char *name, int key, const char *arg, int flags, const char *doc)
{
    char *name_alloc = NULL, *arg_alloc = NULL, *doc_alloc = NULL;
    hpd_argp_option_t option = { 0 };
    hpd_argp_option_t empty = { 0 };

    if (name) HPD_STR_CPY(name_alloc, name);
    if (arg) HPD_STR_CPY(arg_alloc, arg);
    if (doc) HPD_STR_CPY(doc_alloc, doc);
    option.name = name_alloc;
    option.key = key;
    option.arg = arg_alloc;
    option.flags = flags;
    option.doc = doc_alloc;

    HPD_REALLOC(hpd->options, hpd->options_count+2, hpd_argp_option_t);
    hpd->options[hpd->options_count] = option;
    hpd->options[hpd->options_count+1] = empty;
    hpd->options_count++;

    return HPD_E_SUCCESS;

    alloc_error:
    free(name_alloc);
    free(arg_alloc);
    free(doc_alloc);
    LOG_RETURN_E_ALLOC(hpd);
}

static hpd_error_t daemon_loop_create(hpd_t *hpd)
{
    // Create event loop
    hpd->loop = ev_loop_new(EVFLAG_AUTO);
    if (!hpd->loop) LOG_RETURN_E_ALLOC(hpd);
    return HPD_E_SUCCESS;
}

static void daemon_loop_destroy(hpd_t *hpd)
{
    // Destroy event loop
    ev_loop_destroy(hpd->loop);
    hpd->loop = NULL;
}

static void daemon_loop_run(hpd_t *hpd)
{
    // Start signal watchers
    ev_signal_start(hpd->loop, &hpd->sigint_watcher);
    ev_signal_start(hpd->loop, &hpd->sigterm_watcher);

    // Run until we are stopped
    // TODO Return value of ev_run can be used to run until all watchers have stopped (e.g. nice shutdown)
    ev_run(hpd->loop, 0);

    // Stop signal watchers
    ev_signal_stop(hpd->loop, &hpd->sigterm_watcher);
    ev_signal_stop(hpd->loop, &hpd->sigint_watcher);
}

static hpd_error_t daemon_modules_create(const hpd_t *hpd)
{
    // Call on_create() on modules
    hpd_error_t rc;
    hpd_module_t *module;
    TAILQ_FOREACH(module, &hpd->modules, HPD_TAILQ_FIELD) {
        if (module->def.on_create && (rc = module->def.on_create(&module->data, module)) != HPD_E_SUCCESS) {
            LOG_DEBUG(hpd, "Module %s failed to create", module->id);
            goto module_create_error;
        }
    }
    return HPD_E_SUCCESS;

    module_create_error:
    for (; module; module = TAILQ_PREV(module, hpd_modules, HPD_TAILQ_FIELD))
        if (module->def.on_destroy) module->def.on_destroy(module->data);
    return rc;
}

static hpd_error_t daemon_modules_destroy(const hpd_t *hpd)
{
    // Call on_destroy on modules
    hpd_error_t rc;
    hpd_module_t *module;
    TAILQ_FOREACH_REVERSE(module, &hpd->modules, hpd_modules, HPD_TAILQ_FIELD)
        if (module->data) {
            if (module->def.on_destroy && (rc = module->def.on_destroy(module->data)) != HPD_E_SUCCESS)
                goto module_destroy_error;
            module->data = NULL;
        }
    return HPD_E_SUCCESS;

    module_destroy_error:
    for (; module; module = TAILQ_PREV(module, hpd_modules, HPD_TAILQ_FIELD))
        if (module->def.on_destroy) module->def.on_destroy(module->data);
    return rc;
}

static hpd_error_t daemon_modules_start(hpd_t *hpd)
{
    hpd_error_t rc, rc2;
    hpd_module_t *module;

    // Call on_start() on modules
    TAILQ_FOREACH(module, &hpd->modules, HPD_TAILQ_FIELD) {
        if (module->def.on_start) {
            if ((rc = module->def.on_start(module->data)) != HPD_E_SUCCESS) {
                LOG_DEBUG(hpd, "Module %s failed to start", module->id);
                goto module_error;
            } else {
                LOG_INFO(hpd, "Module %s started.", module->id);
            }
        }
    }

    return HPD_E_SUCCESS;

    module_error:
    for (module = TAILQ_PREV(module, hpd_modules, HPD_TAILQ_FIELD); module; module = TAILQ_PREV(module, hpd_modules, HPD_TAILQ_FIELD))
        if (module->def.on_stop && (rc2 = module->def.on_stop(module->data)))
            LOG_ERROR(hpd, "Failed to stop module [code: %i].", rc2);
    return rc;
}

static hpd_error_t daemon_modules_stop(hpd_t *hpd)
{
    hpd_error_t rc, rc2;
    hpd_module_t *module;

    // Call on_stop() on modules
    TAILQ_FOREACH_REVERSE(module, &hpd->modules, hpd_modules, HPD_TAILQ_FIELD) {
        if (module->def.on_stop) {
            if ((rc = module->def.on_stop(module->data)) != HPD_E_SUCCESS) {
                goto module_error;
            } else {
                LOG_INFO(hpd, "Module %s stopped.", module->id);
            }
        }
    }

    return HPD_E_SUCCESS;

    module_error:
    for (module = TAILQ_PREV(module, hpd_modules, HPD_TAILQ_FIELD); module; module = TAILQ_PREV(module, hpd_modules, HPD_TAILQ_FIELD))
        if (module->def.on_stop && (rc2 = module->def.on_stop(module->data)))
            LOG_ERROR(hpd, "Failed to stop module [code: %i].", rc2);
    return rc;
}

static hpd_error_t daemon_options_create(hpd_t *hpd)
{
    hpd_error_t rc;

    HPD_CALLOC(hpd->options, 1, hpd_argp_option_t);
    if ((rc = daemon_add_global_option(hpd, "conf", 'c', "file", 0, "Load arguments from configuration file"))) goto error;
    if ((rc = daemon_add_global_option(hpd, "quiet", 'q', NULL, 0, "Quiet mode"))) goto error;
    if ((rc = daemon_add_global_option(hpd, "verbose", 'v', NULL, 0, "Verbose mode"))) goto error;

    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC(hpd);

    error:
    LOG_DEBUG(hpd, "Failed to add global option");
    free(hpd->options);
    hpd->options = NULL;
    return rc;
}

static void daemon_options_destroy(hpd_t *hpd)
{
    // Deallocate option memory
    hpd_argp_option_t empty = { 0 };
    for (hpd_argp_option_t *option = hpd->options; memcmp(option, &empty, sizeof(hpd_argp_option_t)); option++) {
        free((void *) option->name);
        free((void *) option->arg);
        free((void *) option->doc);
    }
    free(hpd->options);
    hpd->options = NULL;
    free(hpd->option2module);
    hpd->option2module = NULL;
    free(hpd->option2name);
    hpd->option2name = NULL;
}

static hpd_error_t daemon_options_parse(hpd_t *hpd, int argc, char **argv)
{
    // Parse options
    struct argp argp = {hpd->options, daemon_on_parse_opt };
    if (argp_parse(&argp, argc, argv, 0, 0, hpd)) {
        LOG_DEBUG(hpd, "Error while parsing arguments.");
        return HPD_E_ARGUMENT;
    }
    return HPD_E_SUCCESS;
}

static hpd_error_t daemon_runtime_create(hpd_t *hpd)
{
    HPD_CALLOC(hpd->configuration, 1, hpd_configuration_t);
    TAILQ_INIT(&hpd->configuration->adapters);
    TAILQ_INIT(&hpd->configuration->listeners);
    hpd->configuration->hpd = hpd;
    return HPD_E_SUCCESS;

    alloc_error:
    LOG_RETURN_E_ALLOC(hpd);
}

static hpd_error_t daemon_runtime_destroy(hpd_t *hpd)
{
    hpd_error_t rc;
    HPD_TAILQ_MAP_REMOVE(&hpd->configuration->adapters, discovery_free_adapter, hpd_adapter_t, rc);
    HPD_TAILQ_MAP_REMOVE(&hpd->configuration->listeners, event_free_listener, hpd_listener_t, rc);
    free(hpd->configuration);
    hpd->configuration = NULL;
    return HPD_E_SUCCESS;

    map_error:
    LOG_RETURN(hpd, rc, "Free function returned an error [code: %i]", rc);
}

static hpd_error_t daemon_watchers_stop(hpd_t *hpd)
{
    hpd_error_t rc = HPD_E_SUCCESS, tmp;

    // Stop other watchers
    hpd_ev_async_t *async, *async_tmp;
    TAILQ_FOREACH_SAFE(async, &hpd->request_watchers, HPD_TAILQ_FIELD, async_tmp) {
        TAILQ_REMOVE(&hpd->request_watchers, async, HPD_TAILQ_FIELD);
        ev_async_stop(hpd->loop, &async->watcher);
        tmp = request_free_request(async->request);
        if (!rc) rc = tmp;
        else LOG_ERROR(hpd, "free function failed [code: %i]", tmp);
        free(async);
    }
    TAILQ_FOREACH_SAFE(async, &hpd->respond_watchers, HPD_TAILQ_FIELD, async_tmp) {
        TAILQ_REMOVE(&hpd->respond_watchers, async, HPD_TAILQ_FIELD);
        ev_async_stop(hpd->loop, &async->watcher);
        tmp = request_free_response(async->response);
        if (!rc) rc = tmp;
        else LOG_ERROR(hpd, "free function failed [code: %i]", tmp);
        free(async);
    }
    TAILQ_FOREACH_SAFE(async, &hpd->changed_watchers, HPD_TAILQ_FIELD, async_tmp) {
        TAILQ_REMOVE(&hpd->changed_watchers, async, HPD_TAILQ_FIELD);
        ev_async_stop(hpd->loop, &async->watcher);
        tmp = discovery_free_sid(async->service);
        if (!rc) rc = tmp;
        else LOG_ERROR(hpd, "free function failed [code: %i]", tmp);
        tmp = value_free(async->value);
        if (!rc) rc = tmp;
        else LOG_ERROR(hpd, "free function failed [code: %i]", tmp);
        free(async);
    }
    TAILQ_FOREACH_SAFE(async, &hpd->device_watchers, HPD_TAILQ_FIELD, async_tmp) {
        TAILQ_REMOVE(&hpd->device_watchers, async, HPD_TAILQ_FIELD);
        ev_async_stop(hpd->loop, &async->watcher);
        tmp = discovery_free_did(async->device);
        if (!rc) rc = tmp;
        else LOG_ERROR(hpd, "free function failed [code: %i]", tmp);
        free(async);
    }

    return rc;
}

hpd_error_t daemon_alloc(hpd_t **hpd)
{
    HPD_CALLOC(*hpd, 1, hpd_t);
    TAILQ_INIT(&(*hpd)->modules);
    TAILQ_INIT(&(*hpd)->request_watchers);
    TAILQ_INIT(&(*hpd)->respond_watchers);
    TAILQ_INIT(&(*hpd)->changed_watchers);
    TAILQ_INIT(&(*hpd)->device_watchers);
    ev_signal_init(&(*hpd)->sigint_watcher, daemon_on_signal, SIGINT);
    ev_signal_init(&(*hpd)->sigterm_watcher, daemon_on_signal, SIGTERM);
    (*hpd)->sigint_watcher.data = hpd;
    (*hpd)->sigterm_watcher.data = hpd;
    (*hpd)->log_level = HPD_L_INFO;

    return HPD_E_SUCCESS;

    alloc_error:
    if (*hpd) {
        if ((*hpd)->options) free((*hpd)->options);
        free(*hpd);
    }
    return HPD_E_NULL;
}

hpd_error_t daemon_free(hpd_t *hpd)
{
    hpd_module_t *module, *module_tmp;
    TAILQ_FOREACH_SAFE(module, &hpd->modules, HPD_TAILQ_FIELD, module_tmp) {
        TAILQ_REMOVE(&hpd->modules, module, HPD_TAILQ_FIELD);
        free(module->id);
        free(module);
    }
    free(hpd);
    return HPD_E_SUCCESS;
}

hpd_error_t daemon_add_module(hpd_t *hpd, const char *id, const hpd_module_def_t *module_def)
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
    LOG_RETURN_E_ALLOC(hpd);
}

hpd_error_t daemon_add_option(const hpd_module_t *context, const char *name, const char *arg, int flags,
                              const char *doc)
{
    hpd_error_t rc;
    hpd_t *hpd = context->hpd;

    char *name_cat;
    HPD_CALLOC(name_cat, strlen(name)+strlen(context->id)+2, char);
    strcpy(name_cat, context->id);
    strcat(name_cat, "-");
    strcat(name_cat, name);

    int key = 0xff + hpd->module_options_count;

    if ((rc = daemon_add_global_option(hpd, name_cat, key, arg, flags, doc))) goto error;

    HPD_REALLOC(hpd->option2module, hpd->module_options_count+1, const hpd_module_t *);
    hpd->option2module[hpd->module_options_count] = context;

    HPD_REALLOC(hpd->option2name, hpd->module_options_count+1, const char *);
    hpd->option2name[hpd->module_options_count] = hpd->options[hpd->options_count-1].name;

    hpd->module_options_count++;

    free(name_cat);
    return HPD_E_SUCCESS;

    error:
    free(name_cat);
    return rc;

    alloc_error:
    // TODO This leaves everything in a bit of a weird state
    free(name_cat);
    LOG_RETURN_E_ALLOC(hpd);
}

hpd_error_t daemon_start(hpd_t *hpd, int argc, char *argv[])
{
    hpd_error_t rc, rc2;

    hpd->argv0 = argv[0];

    // Allocate run-time and option memory
    LOG_INFO(hpd, "Starting...");
    if ((rc = daemon_runtime_create(hpd)))
        goto runtime_create_error;
    if ((rc = daemon_options_create(hpd)))
        goto options_create_error;
    if ((rc = daemon_modules_create(hpd)))
        goto modules_create_error;
    if ((rc = daemon_options_parse(hpd, argc, argv)))
        goto options_parse_error;
    daemon_options_destroy(hpd);
    if ((rc = daemon_loop_create(hpd)))
        goto loop_create_error;
    if ((rc = daemon_modules_start(hpd)))
        goto modules_start_error;
    LOG_INFO(hpd, "Started.");
    daemon_loop_run(hpd);
    LOG_INFO(hpd, "Stopping...");
    if ((rc = daemon_modules_stop(hpd)))
        goto modules_stop_error;
    if ((rc = daemon_watchers_stop(hpd)))
        goto watchers_stop_error;
    daemon_loop_destroy(hpd);
    if ((rc = daemon_modules_destroy(hpd)))
        goto modules_destroy_error;
    if ((rc = daemon_runtime_destroy(hpd)))
        goto runtime_destroy_error;
    LOG_INFO(hpd, "Stopped.");

    // Return
    return HPD_E_SUCCESS;

    // Return in a nice state if an error occur
    options_parse_error:
    if ((rc2 = daemon_modules_destroy(hpd))) LOG_ERROR(hpd, "Failed to destroy modules [code: %i]", rc2);
    modules_create_error:
    daemon_options_destroy(hpd);
    options_create_error:
    if ((rc2 = daemon_runtime_destroy(hpd))) LOG_ERROR(hpd, "Failed to destroy runtime [code: %i]", rc2);
    runtime_create_error:
    return rc;
    modules_start_error:
    modules_stop_error:
    if ((rc2 = daemon_watchers_stop(hpd))) LOG_ERROR(hpd, "Failed to stop watchers [code: %i]", rc2);
    watchers_stop_error:
    daemon_loop_destroy(hpd);
    loop_create_error:
    if ((rc2 = daemon_modules_destroy(hpd))) LOG_ERROR(hpd, "Failed to destroy modules [code: %i]", rc2);
    modules_destroy_error:
    if ((rc2 = daemon_runtime_destroy(hpd))) LOG_ERROR(hpd, "Failed to destroy runtime [code: %i]", rc2);
    runtime_destroy_error:
    return rc;
}

hpd_error_t daemon_stop(const hpd_t *hpd)
{
    ev_break(hpd->loop, EVBREAK_ALL);
    return HPD_E_SUCCESS;
}

hpd_error_t daemon_get_id(const hpd_module_t *context, const char **id)
{
    (*id) = context->id;
    return HPD_E_SUCCESS;
}

hpd_error_t daemon_get_loop(const hpd_t *hpd, hpd_ev_loop_t **loop)
{
    (*loop) = hpd->loop;
    return HPD_E_SUCCESS;
}
