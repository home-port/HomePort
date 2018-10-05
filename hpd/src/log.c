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
#include "event.h"

#ifdef THREAD_SAFE
#include <pthread.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>

#define COLOR_BLACK    "\x1b[30m"
#define COLOR_RED      "\x1b[31m"
#define COLOR_GREEN    "\x1b[32m"
#define COLOR_YELLOW   "\x1b[33m"
#define COLOR_BLUE     "\x1b[34m"
#define COLOR_MAGENTA  "\x1b[35m"
#define COLOR_CYAN     "\x1b[36m"
#define COLOR_WHITE    "\x1b[37m"
#define COLOR_BBLACK   "\x1b[90m"
#define COLOR_BRED     "\x1b[91m"
#define COLOR_BGREEN   "\x1b[92m"
#define COLOR_BYELLOW  "\x1b[93m"
#define COLOR_BBLUE    "\x1b[94m"
#define COLOR_BMAGENTA "\x1b[95m"
#define COLOR_BCYAN    "\x1b[96m"
#define COLOR_BWHITE   "\x1b[97m"
#define COLOR_RESET    "\x1b[0m"

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
    char *color = "";
    if (hpd->log_colored) color = COLOR_RESET;
    switch (level) {
        case HPD_L_NONE:
            break;
        case HPD_L_ERROR:
            type = "ERROR";
            if (hpd->log_colored) color = COLOR_RED;
            break;
        case HPD_L_WARN:
            type = "WARNING";
            if (hpd->log_colored) color = COLOR_YELLOW;
            break;
        case HPD_L_INFO:
            type = "INFO";
            if (hpd->log_colored) color = COLOR_BLACK;
            break;
        case HPD_L_DEBUG:
            type = "DEBUG";
            if (hpd->log_colored) color = COLOR_WHITE;
            break;
        case HPD_L_VERBOSE:
            type = "VERBOSE";
            if (hpd->log_colored) color = COLOR_BBLACK;
            break;
    }

    if (!type) LOG_RETURN(hpd, HPD_E_ARGUMENT, "Unknown log level.");

    time_t timer;
    char time_buffer[26];
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(time_buffer, 26, "%Y/%m/%d %H:%M:%S", tm_info);

    const char *fn = &strrchr(file, '/')[1];

    va_list  vp_copy;
    va_copy(vp_copy, vp);
    int mlen = vsnprintf(NULL, 0, fmt, vp_copy);
    if (mlen < 0) return HPD_E_UNKNOWN;
    int slen =
            snprintf(NULL, 0, "%s [%s]%*s %8s: ", time_buffer, module, (int) (12 - strlen(module)), "", type) +
            mlen +
            snprintf(NULL, 0, "%*s  %s:%d\n", 128 - mlen, "", fn ? fn : file, line);
    if (slen < 0) return HPD_E_UNKNOWN;
    size_t uslen = (unsigned int) slen+1;

    char *msg = calloc(uslen, sizeof(char));
    if (!msg) return HPD_E_ALLOC;

    int i = snprintf(msg, uslen, "%s [%s]%*s %8s: ", time_buffer, module, (int) (12 - strlen(module)), "", type);
    i += vsnprintf(&msg[i], uslen - i, fmt, vp);
    snprintf(&msg[i], uslen - i, "%*s  %s:%d\n", 128 - mlen, "", fn ? fn : file, line);

#ifdef THREAD_SAFE
    // TODO Better thing to do?
    if (pthread_mutex_lock(&hpd->log_mutex)) return HPD_E_UNKNOWN;
#endif

    fprintf(stderr, "%s%s%s", color, msg, COLOR_RESET);
    // TODO Using log methods inside on_log callbacks causes everything to freeze (above mutex lock)
    event_log(hpd, msg);

#ifdef THREAD_SAFE
    // TODO Better thing to do?
    if (pthread_mutex_unlock(&hpd->log_mutex)) return HPD_E_UNKNOWN;
#endif

    free(msg);
    return HPD_E_SUCCESS;
}
