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

#include "log.h"
#include "daemon.h"

hpd_error_t hpd_logf(const hpd_module_t *context, hpd_log_level_t level, const char *file, int line, const char *fmt, ...)
{
    if (!context) return HPD_E_NULL;
    if (!fmt) LOG_RETURN_E_NULL(context->hpd);
    hpd_error_t rc;
    va_list vp;
    va_start(vp, fmt);
    rc = log_vlogf(context->hpd, context->id, level, file, line, fmt, vp);
    va_end(vp);
    return rc;
}

hpd_error_t hpd_vlogf(const hpd_module_t *context, hpd_log_level_t level, const char *file, int line, const char *fmt, va_list vp)
{
    if (!context) return HPD_E_NULL;
    if (!fmt) LOG_RETURN_E_NULL(context->hpd);
    return log_vlogf(context->hpd, context->id, level, file, line, fmt, vp);
}
