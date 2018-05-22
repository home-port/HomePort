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

#include "hpd-0.6/hpd_api.h"
#include "daemon.h"
#include "log.h"

hpd_error_t hpd_alloc(hpd_t **hpd) {
    if (!hpd) return HPD_E_NULL;
    return daemon_alloc(hpd);
}

hpd_error_t hpd_free(hpd_t *hpd)
{
    if (!hpd) return HPD_E_NULL;
    return daemon_free(hpd);
}

hpd_error_t hpd_get_loop(const hpd_module_t *context, hpd_ev_loop_t **loop)
{
    if (!context) return HPD_E_NULL;
    if (!loop) LOG_RETURN_E_NULL(context->hpd);
    return daemon_get_loop(context->hpd, loop);
}

hpd_error_t hpd_module(hpd_t *hpd, const char *id, const hpd_module_def_t *module_def)
{
    hpd_module_t *module;

    if (!hpd) return HPD_E_NULL;
    if (!id || !module_def) LOG_RETURN_E_NULL(hpd);
    if (hpd->configuration) LOG_RETURN(hpd, HPD_E_STATE, "Cannot add module while hpd is running.");
    if (strchr(id, '-')) LOG_RETURN(hpd, HPD_E_ARGUMENT, "Module ids may not contain '-'.");
    if (strcmp(id, "hpd") == 0) LOG_RETURN(hpd, HPD_E_ARGUMENT, "Module ids cannot be 'hpd'.");
    TAILQ_FOREACH(module, &hpd->modules, HPD_TAILQ_FIELD)
        if (strcmp(module->id, id) == 0)
            LOG_RETURN(hpd, HPD_E_NOT_UNIQUE, "Module ids must be unique.");

    return daemon_add_module(hpd, id, module_def);
}

hpd_error_t hpd_module_add_option(const hpd_module_t *context, const char *name, const char *arg, int flags,
                                  const char *doc)
{
    if (!context) return HPD_E_NULL;
    if (!name) LOG_RETURN_E_NULL(context->hpd);
    hpd_t *hpd = context->hpd;
    if (!hpd->options) LOG_RETURN(hpd, HPD_E_STATE, "Can only add options during on_create().");
    size_t id_len = strlen(context->id);
    size_t name_index = id_len + 1;
    for (int i = 0; i < hpd->module_options_count; i++) {
        size_t name_len = strlen(hpd->options[i].name);
        if (name_len > id_len && strncmp(hpd->options[i].name, context->id, id_len) == 0) {
            if (name_index < name_len && strcmp(&hpd->options[i].name[name_index], name) == 0)
                LOG_RETURN(hpd, HPD_E_NOT_UNIQUE, "Option names must be unique within the module.");
        }
    }

    return daemon_add_option(context, name, arg, flags, doc);
}

hpd_error_t hpd_module_get_id(const hpd_module_t *context, const char **id)
{
    if (!context) return HPD_E_NULL;
    if (!id) LOG_RETURN_E_NULL(context->hpd);

    return daemon_get_id(context, id);
}

hpd_error_t hpd_module_get_def(const hpd_module_t *context, const hpd_module_def_t **mdef)
{
    if (!context) return HPD_E_NULL;
    if (!mdef) LOG_RETURN_E_NULL(context->hpd);

    return daemon_get_mdef(context, mdef);
}

hpd_error_t hpd_start(hpd_t *hpd, int argc, char *argv[])
{
    if (!hpd) return HPD_E_NULL;

    return daemon_start(hpd, argc, argv);
}

hpd_error_t hpd_stop(hpd_t *hpd)
{
    if (!hpd) return HPD_E_NULL;
    if (!hpd->loop) LOG_RETURN_HPD_STOPPED(hpd);

    return daemon_stop(hpd);
}
