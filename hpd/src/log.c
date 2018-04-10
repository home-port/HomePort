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
#ifdef THREAD_SAFE
#include <pthread.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>

hpd_error_t log_logf(hpd_t *hpd, const char *module, hpd_log_level_t level, const char *file, int line, const char *fmt, ...)
{
    hpd_error_t rc;
    va_list vp;
    va_start(vp, fmt);
    rc = log_vlogf(hpd, module, level, file, line, fmt, vp);
    va_end(vp);
    return rc;
}

hpd_error_t log_vlogf(hpd_t *hpd, const char *module, hpd_log_level_t level, const char *file, int line, const char *fmt, va_list vp)
{
    if (strcmp(module, HPD_LOG_MODULE) == 0 && level > hpd->hpd_log_level) return HPD_E_SUCCESS;

    char *type = NULL;
    switch (level) {
        case HPD_L_NONE:
            break;
        case HPD_L_ERROR:
            type = "ERROR";
            break;
        case HPD_L_WARN:
            type = "WARNING";
            break;
        case HPD_L_INFO:
            type = "INFO";
            break;
        case HPD_L_DEBUG:
            type = "DEBUG";
            break;
        case HPD_L_VERBOSE:
            type = "VERBOSE";
            break;
    }
    if (!type) LOG_RETURN(hpd, HPD_E_ARGUMENT, "Unknown log level.");

    time_t timer;
    char time_buffer[26];
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(time_buffer, 26, "%Y/%m/%d %H:%M:%S", tm_info);

#ifdef THREAD_SAFE
    // TODO Better thing to do?
    if (pthread_mutex_lock(&hpd->log_mutex)) return HPD_E_UNKNOWN;
#endif

    fprintf(stderr, "%s [%s]%*s %8s: ", time_buffer, module, (int) (12 - strlen(module)), "", type);
    int len = vfprintf(stderr, fmt, vp);
    const char *fn = &strrchr(file, '/')[1];
    fprintf(stderr, "%*s  %s:%d\n", 128 - len, "", fn ? fn : file, line);

#ifdef THREAD_SAFE
    // TODO Better thing to do?
    if (pthread_mutex_unlock(&hpd->log_mutex)) return HPD_E_UNKNOWN;
#endif

    return HPD_E_SUCCESS;
}
