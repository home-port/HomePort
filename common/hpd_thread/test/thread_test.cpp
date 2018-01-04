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

#include <gtest/gtest.h>
#include "hpd/hpd_api.h"
#include "hpd/common/hpd_thread.h"
#include "hpd/common/hpd_thread_module.h"
#include <ev.h>

#define HPD_LOG_RETURN_PTHREAD(CONTEXT, RC) HPD_LOG_RETURN((CONTEXT), HPD_E_UNKNOWN, "pthread failed [code: %i].", (RC))
#define CASE hpd_thread

typedef struct {
    const hpd_module_t *context;
    hpd_ev_loop_t *loop;
    ev_async stop;
} module_data_t;

static hpd_t *hpd;

static void stop_hpd(hpd_ev_loop_t *, ev_async *, int)
{
    hpd_stop(hpd);
}

static void *on_thread(void *data)
{
    auto *module_data = (module_data_t *) data;

    sleep(2);
    hpd_thread_lock(module_data->context);
    // TODO How to check if the other thread is actually paused here?
    HPD_LOG_INFO(module_data->context, "Doing work...");
    hpd_thread_unlock(module_data->context);

    ev_async_send(module_data->loop, &module_data->stop);

    return nullptr;
}

static hpd_error_t on_create(void **data, const hpd_module_t *context)
{
    auto *module_data = (module_data_t *) calloc(1, sizeof(module_data_t));
    if (!module_data) return HPD_E_ALLOC;
    module_data->context = context;
    ev_async_init(&module_data->stop, stop_hpd);
    module_data->stop.data = hpd;

    *data = module_data;
    return HPD_E_SUCCESS;
}

static hpd_error_t on_destroy(void *)
{
//    module_data_t *module_data = (module_data_t *) data;
    return HPD_E_SUCCESS;
}

static hpd_error_t on_start(void *data)
{
    auto *module_data = (module_data_t *) data;

    hpd_get_loop(module_data->context, &module_data->loop);
    ev_async_start(module_data->loop, &module_data->stop);

    int stat;
    pthread_t thread;
    if ((stat = pthread_create(&thread, nullptr, on_thread, module_data)))
        HPD_LOG_RETURN_PTHREAD(module_data->context, stat);

    return HPD_E_SUCCESS;
}

static hpd_error_t on_stop(void *)
{
//    module_data_t *module_data = (module_data_t *) data;
    return HPD_E_SUCCESS;
}

static hpd_error_t on_parse_opt(void *, const char *, const char *)
{
//    module_data_t *module_data = (module_data_t *) data;
    return HPD_E_ARGUMENT;
}

TEST(CASE, lock_unlock) {
    int argc = 2;
    char *argv[] = {
            (char *) "/usr/local/bin/hpd",
            (char *) "-v",
            nullptr
    };
    const char *id = "mod";
    hpd_module_def_t module_def { on_create, on_destroy, on_start, on_stop, on_parse_opt };

    ASSERT_EQ(hpd_alloc(&hpd), HPD_E_SUCCESS);
    ASSERT_EQ(hpd_module(hpd, "thread", &hpd_thread), HPD_E_SUCCESS);
    ASSERT_EQ(hpd_module(hpd, id, &module_def), HPD_E_SUCCESS);
    ASSERT_EQ(hpd_start(hpd, argc, argv), HPD_E_SUCCESS);
    ASSERT_EQ(hpd_free(hpd), HPD_E_SUCCESS);
}
