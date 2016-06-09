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

#include "log.h"
#include <stdio.h>
#include <string.h>

hpd_error_t log_logf(const char *module, hpd_log_level_t level, const char *file, int line, const char *fmt, ...)
{
    hpd_error_t rc;
    va_list vp;
    va_start(vp, fmt);
    rc = log_vlogf(module, level, file, line, fmt, vp);
    va_end(vp);
    return rc;
}

hpd_error_t log_vlogf(const char *module, hpd_log_level_t level, const char *file, int line, const char *fmt, va_list vp)
{
    FILE *stream;
    char *type;

    // TODO Something more fancy than just printing...
    switch (level) {
        case HPD_L_ERROR:
            stream = stderr;
            type = "ERROR";
            break;
        case HPD_L_WARN:
            stream = stderr;
            type = "WARNING";
            break;
        case HPD_L_INFO:
            stream = stdout;
            type = "INFO";
            break;
        case HPD_L_DEBUG:
            stream = stderr;
            type = "DEBUG";
            break;
        case HPD_L_VERBOSE:
            stream = stdout;
            type = "VERBOSE";
            break;
        default:
            LOG_RETURN(HPD_E_ARGUMENT, "Unknown log level.");
    }

    fprintf(stream, "[%s]%*s %8s: ", module, (int) (16 - strlen(module)), "", type);
    int len = vfprintf(stream, fmt, vp);
    fprintf(stream, "%*s  %s:%d\n", 128-len, "", file, line);

    return HPD_E_SUCCESS;
}
