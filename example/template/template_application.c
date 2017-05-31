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

/// [file]
#include "template_application.h"
#include <hpd/hpd_application_api.h>

static hpd_error_t template_app_on_create(void **data, const hpd_module_t *context);
static hpd_error_t template_app_on_destroy(void *data);
static hpd_error_t template_app_on_start(void *data);
static hpd_error_t template_app_on_stop(void *data);
static hpd_error_t template_app_on_parse_opt(void *data, const char *name, const char *arg);

struct hpd_module_def template_app_def = {
        template_app_on_create,
        template_app_on_destroy,
        template_app_on_start,
        template_app_on_stop,
        template_app_on_parse_opt,
};

static hpd_error_t template_app_on_create(void **data, const hpd_module_t *context)
{
    return HPD_E_SUCCESS;
}

static hpd_error_t template_app_on_destroy(void *data)
{
    return HPD_E_SUCCESS;
}

static hpd_error_t template_app_on_start(void *data)
{
    return HPD_E_SUCCESS;
}

static hpd_error_t template_app_on_stop(void *data)
{
    return HPD_E_SUCCESS;
}

static hpd_error_t template_app_on_parse_opt(void *data, const char *name, const char *arg)
{
    return HPD_E_ARGUMENT;
}
/// [file]
