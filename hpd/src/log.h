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

#ifndef HOMEPORT_LOG_H
#define HOMEPORT_LOG_H

#include "hpd/hpd_types.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HPD_LOG_MODULE "hpd"

hpd_error_t log_logf(hpd_t *hpd, const char *module, hpd_log_level_t level, const char *file, int line, const char *fmt, ...);
hpd_error_t log_vlogf(hpd_t *hpd, const char *module, hpd_log_level_t level, const char *file, int line, const char *fmt, va_list vp);

#define LOG_ERROR(HPD, FMT, ...) log_logf((HPD), HPD_LOG_MODULE, HPD_L_ERROR, __FILE__, __LINE__, (FMT), ##__VA_ARGS__)
#define LOG_WARN(HPD, FMT, ...) log_logf((HPD), HPD_LOG_MODULE, HPD_L_WARN , __FILE__, __LINE__, (FMT), ##__VA_ARGS__)
#define LOG_INFO(HPD, FMT, ...) log_logf((HPD), HPD_LOG_MODULE, HPD_L_INFO , __FILE__, __LINE__, (FMT), ##__VA_ARGS__)
#define LOG_DEBUG(HPD, FMT, ...) log_logf((HPD), HPD_LOG_MODULE, HPD_L_DEBUG, __FILE__, __LINE__, (FMT), ##__VA_ARGS__)

#define LOG_RETURN(HPD, E, FMT, ...) do { LOG_DEBUG((HPD), (FMT), ##__VA_ARGS__); return (E); } while(0)

#define LOG_RETURN_E_NULL(HPD)  LOG_RETURN((HPD), HPD_E_NULL,  "Unexpected null pointer.")
#define LOG_RETURN_E_ALLOC(HPD) LOG_RETURN((HPD), HPD_E_ALLOC, "Unable to allocate memory.")

#define LOG_RETURN_HPD_STOPPED(HPD) LOG_RETURN((HPD), HPD_E_STATE, "Cannot perform %s() while hpd is stopped.", __func__)
#define LOG_RETURN_ATTACHED(HPD) LOG_RETURN((HPD), HPD_E_ARGUMENT, "Cannot perform %s(), object already attached.", __func__)
#define LOG_RETURN_DETACHED(HPD) LOG_RETURN((HPD), HPD_E_ARGUMENT, "Cannot perform %s(), object already detached.", __func__)

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_LOG_H
