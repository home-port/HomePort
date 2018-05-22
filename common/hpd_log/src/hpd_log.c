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

// TODO Error check, entire file

#include <hpd-0.6/modules/hpd_log.h>
#include <hpd-0.6/hpd_application_api.h>
#include <hpd-0.6/common/hpd_common.h>

static hpd_error_t log_on_create(void **data, const hpd_module_t *context);
static hpd_error_t log_on_destroy(void *data);
static hpd_error_t log_on_start(void *data);
static hpd_error_t log_on_stop(void *data);
static hpd_error_t log_on_parse_opt(void *data, const char *name, const char *arg);

hpd_module_def_t hpd_log = {
        log_on_create,
        log_on_destroy,
        log_on_start,
        log_on_stop,
        log_on_parse_opt
};

typedef struct log log_t;

struct log {
    const hpd_module_t *context;
    char *fn;
    FILE *file;
    hpd_listener_t *listener;
};

static void log_on_log(void *data, const char *msg)
{
    log_t *log = data;
    if (fprintf(log->file, "%s", msg) < 0) {
        // TODO Could consider to reopen the file here...
        return;
    }

    if (!fflush(log->file)) {
        return;
    }
}

static hpd_error_t log_on_create(void **data, const hpd_module_t *context)
{
    hpd_error_t rc;

    if (!context) return HPD_E_NULL;

    if ((rc = hpd_module_add_option(context, "file", "file", 0, "Append all log messages to file")))
        return rc;

    log_t *log;
    HPD_CALLOC(log, 1, log_t);
    log->context = context;

    (*data) = log;
    return HPD_E_SUCCESS;

    alloc_error:
    HPD_LOG_RETURN_E_ALLOC(context);
}

static hpd_error_t log_on_destroy(void *data)
{
    log_t *log = data;
    free(log->fn);
    free(log);
    return HPD_E_SUCCESS;
}

static hpd_error_t log_on_start(void *data)
{
    hpd_error_t rc, rc2;
    log_t *log = data;

    if (log->fn) {
        log->file = fopen(log->fn, "a");
        if (!log->file) HPD_LOG_RETURN(log->context, HPD_E_UNKNOWN, "Failed to open file '%s' for writing", log->fn);

        if ((rc = hpd_listener_alloc(&log->listener, log->context))) goto error_close;
        if ((rc = hpd_listener_set_log_callback(log->listener, log_on_log))) goto error_free;
        if ((rc = hpd_listener_set_data(log->listener, log, NULL))) goto error_free;
        if ((rc = hpd_subscribe(log->listener))) goto error_free;

        HPD_LOG_INFO(log->context, "Logging to file '%s'...", log->fn);
    }

    return HPD_E_SUCCESS;

    error_free:
    if ((rc2 = hpd_listener_free(log->listener))) HPD_LOG_ERROR(log->context, "Free failed [code: %d]", rc2);

    error_close:
    fclose(log->file);
    return rc;
}

static hpd_error_t log_on_stop(void *data)
{
    hpd_error_t rc;
    log_t *log = data;

    if (log->file) {
        if (!fclose(log->file)) HPD_LOG_RETURN(log->context, HPD_E_UNKNOWN, "Failed to close file '%s'", log->fn);
        if ((rc = hpd_listener_free(log->listener))) return rc;
    }

    return HPD_E_SUCCESS;
}

static hpd_error_t log_on_parse_opt(void *data, const char *name, const char *arg)
{
    log_t *log = data;

    if (strcmp(name, "file") == 0) {
        HPD_STR_CPY(log->fn, arg);
        return HPD_E_SUCCESS;
    }

    return HPD_E_ARGUMENT;

    alloc_error:
    HPD_LOG_RETURN_E_ALLOC(log->context);
}
