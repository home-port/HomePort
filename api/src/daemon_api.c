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

#include "daemon_api.h"
#include "daemon.h"
#include "old_model.h"
#include <string.h>
// Not the same as other queue.h
#include <bsd/sys/queue.h>

hpd_error_t hpd_alloc(hpd_t **hpd) {
    if (!hpd) return HPD_E_NULL;
    return daemon_alloc(hpd);
}

hpd_error_t hpd_free(hpd_t *hpd)
{
    if (!hpd) return HPD_E_NULL;
    return daemon_free(hpd);
}

hpd_error_t hpd_module(hpd_t *hpd, const char *id, hpd_module_def_t *module_def)
{
    hpd_module_t *module;
    if (!hpd || !id || !module_def) return HPD_E_NULL;
    if (hpd->configuration) return HPD_E_RUNNING;
    if (strchr(id, '-')) return HPD_E_ARGUMENT;
    HPD_TAILQ_FOREACH(module, &hpd->modules)
        if (strcmp(module->id, id) == 0) return HPD_E_NOT_UNIQUE;
    return daemon_add_module(hpd, id, module_def);
}

hpd_error_t hpd_add_option(hpd_module_t *context, const char *name, const char *arg, int flags, const char *doc)
{
    if (!context || !name) return HPD_E_NULL;
    hpd_t *hpd = context->hpd;
    if (!hpd->options) return HPD_E_RUNNING;
    // TODO Doesn't check whether name exist already
    return daemon_add_option(context, name, arg, flags, doc);
}

hpd_error_t hpd_start(hpd_t *hpd, int argc, char *argv[])
{
    if (!hpd) return HPD_E_NULL;
    return daemon_start(hpd, argc, argv);
}

hpd_error_t hpd_stop(hpd_t *hpd)
{
    if (!hpd) return HPD_E_NULL;
    if (!hpd->loop) return HPD_E_STOPPED;
    return daemon_stop(hpd);
}

hpd_error_t hpd_get_loop(hpd_t *hpd, hpd_ev_loop_t **loop)
{
    if (!hpd || !loop) return HPD_E_NULL;
    return daemon_get_loop(hpd, loop);
}
