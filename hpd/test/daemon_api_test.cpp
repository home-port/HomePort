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

#include <gtest/gtest.h>
#include "hpd/hpd_api.h"
#include "daemon.h"

#define CASE hpd_daemon_api

typedef struct {
    int next;
    int create_called;
    int destroy_called;
    int start_called;
    int stop_called;
    int parse_opt_called;
    hpd_ev_loop_t *start_loop;
    hpd_ev_loop_t *stop_loop;
    hpd_error_t add_option1;
    hpd_error_t add_option2;
    int option_errors[21];
    int opt1_called;
    int opt2_called;
} module_data_t;

static module_data_t *last_module_data = NULL;

static hpd_error_t on_create(void **data, const hpd_module_t *)
{
    module_data_t *module_data = (module_data_t *) calloc(1, sizeof(module_data_t));
    if (!module_data) return HPD_E_ALLOC;
    module_data->create_called = ++module_data->next;
    *data = module_data;
    last_module_data = module_data;
    return HPD_E_SUCCESS;
}

static hpd_error_t on_create_add_options(void **data, const hpd_module_t *context)
{
    hpd_t *hpd = context->hpd;
    hpd_argp_option_t *option;
    hpd_error_t rc;
    if ((rc = on_create(data, context)) != HPD_E_SUCCESS) return rc;

    module_data_t *module_data = (module_data_t *) *data;
    if ((module_data->add_option1 = hpd_module_add_option(context, "opt1", NULL, 0, "Option 1")) != HPD_E_SUCCESS)
        return module_data->add_option1;
    if ((module_data->add_option2 = hpd_module_add_option(context, "opt2", NULL, 0, "Option 2")) != HPD_E_SUCCESS)
        return module_data->add_option2;

    if (hpd->module_options_count != 2) module_data->option_errors[0] = 1;
    option = &hpd->options[0];
    if (strcmp(option->name, "mod-opt1")) module_data->option_errors[1] = 1;
    if (option->key != 0xff+0) module_data->option_errors[2] = 1;
    if (option->arg != nullptr) module_data->option_errors[3] = 1;
    if (option->flags != 0) module_data->option_errors[4] = 1;
    if (strcmp(option->doc, "Option 1")) module_data->option_errors[5] = 1;
    if (option->group != 0) module_data->option_errors[6] = 1;
    option = &hpd->options[1];
    if (strcmp(option->name, "mod-opt2")) module_data->option_errors[7] = 1;
    if (option->key != 0xff+1) module_data->option_errors[8] = 1;
    if (option->arg != nullptr) module_data->option_errors[9] = 1;
    if (option->flags != 0) module_data->option_errors[10] = 1;
    if (strcmp(option->doc, "Option 2")) module_data->option_errors[11] = 1;
    if (option->group != 0) module_data->option_errors[12] = 1;
    option = &hpd->options[2];
    if (option->name != nullptr) module_data->option_errors[13] = 1;
    if (option->key != 0) module_data->option_errors[14] = 1;
    if (option->arg != nullptr) module_data->option_errors[15] = 1;
    if (option->flags != 0) module_data->option_errors[16] = 1;
    if (option->doc != nullptr) module_data->option_errors[17] = 1;
    if (option->group != 0) module_data->option_errors[18] = 1;
    if (hpd->option2module[0] != context) module_data->option_errors[19] = 1;
    if (hpd->option2module[1] != context) module_data->option_errors[20] = 1;

    return HPD_E_SUCCESS;
}

static hpd_error_t on_destroy(void *data)
{
    module_data_t *module_data = (module_data_t *) data;
    module_data->destroy_called = ++module_data->next;
    return HPD_E_SUCCESS;
}

static hpd_error_t on_start(void *data, hpd_t *hpd)
{
    module_data_t *module_data = (module_data_t *) data;
    module_data->start_called = ++module_data->next;
    module_data->start_loop = hpd->loop;
    return HPD_E_SUCCESS;
}

static hpd_error_t on_stop(void *data, hpd_t *hpd)
{
    module_data_t *module_data = (module_data_t *) data;
    module_data->stop_called = ++module_data->next;
    module_data->stop_loop = hpd->loop;
    return HPD_E_SUCCESS;
}

static hpd_error_t on_parse_opt(void *data, const char *name, const char *)
{
    module_data_t *module_data = (module_data_t *) data;
    if (strcmp(name, "opt1") == 0) module_data->opt1_called = ++module_data->next;
    else if (strcmp(name, "opt2") == 0) module_data->opt2_called = ++module_data->next;
    else module_data->parse_opt_called = ++module_data->next;
    return HPD_E_SUCCESS;
}

static void stop_hpd(hpd_ev_loop_t *loop, ev_timer *w, int revents)
{
    hpd_t *hpd = (hpd_t *) w->data;
    hpd_stop(hpd);
}

TEST(CASE, hpd_allocation) {
    hpd_t *hpd;

    ASSERT_EQ(hpd_alloc(&hpd), HPD_E_SUCCESS);
    ASSERT_EQ(hpd->loop, nullptr);
    ASSERT_EQ(hpd->configuration, nullptr);
    ASSERT_TRUE(TAILQ_EMPTY(&hpd->modules));
    ASSERT_EQ(hpd->module_options_count, 0);
    ASSERT_EQ(hpd->options, nullptr);
    ASSERT_EQ(hpd->option2module, nullptr);
    ASSERT_EQ(hpd_free(hpd), HPD_E_SUCCESS);
}

TEST(CASE, hpd_get_loop) {
    hpd_t *hpd;
    hpd_ev_loop_t *loop;

    ASSERT_EQ(hpd_alloc(&hpd), HPD_E_SUCCESS);
    ASSERT_EQ(hpd_get_loop(hpd, &loop), HPD_E_SUCCESS);
    ASSERT_EQ(loop, hpd->loop);
    ASSERT_EQ(hpd_free(hpd), HPD_E_SUCCESS);
}

TEST(CASE, hpd_run_1sec) {
    hpd_t *hpd;
    ev_timer timer;
    int argc = 0;
    char *argv[] = {
            (char *) "/usr/local/bin/hpd",
            nullptr
    };

    ASSERT_EQ(hpd_alloc(&hpd), HPD_E_SUCCESS);
    hpd_ev_loop_t *loop = hpd->loop;

    ev_init(&timer, stop_hpd);
    timer.repeat = 0.250;
    timer.data = hpd;
    // TODO Test broken: loop no longer available before start()
    ev_timer_again(loop, &timer);

    ASSERT_EQ(hpd_start(hpd, argc, argv), HPD_E_SUCCESS);

    ASSERT_EQ(hpd_free(hpd), HPD_E_SUCCESS);
}

TEST(CASE, module_allocation) {
    hpd_t *hpd;
    hpd_module_t *module;
    const char *id = "mod";
    hpd_module_def_t module_def { on_create, on_destroy, on_start, on_stop, on_parse_opt };

    ASSERT_EQ(hpd_alloc(&hpd), HPD_E_SUCCESS);

    ASSERT_EQ(hpd_module(hpd, id, &module_def), HPD_E_SUCCESS);
    ASSERT_FALSE(TAILQ_EMPTY(&hpd->modules));
    ASSERT_NE((module = TAILQ_FIRST(&hpd->modules)), nullptr);
    ASSERT_EQ(module->hpd, hpd);
    ASSERT_EQ(TAILQ_NEXT(module, HPD_TAILQ_FIELD), nullptr);
    ASSERT_EQ(TAILQ_PREV(module, hpd_modules, HPD_TAILQ_FIELD), nullptr);
    ASSERT_EQ(module->def.on_create, module_def.on_create);
    ASSERT_EQ(module->def.on_destroy, module_def.on_destroy);
    ASSERT_EQ(module->def.on_start, module_def.on_start);
    ASSERT_EQ(module->def.on_stop, module_def.on_stop);
    ASSERT_EQ(module->def.on_parse_opt, module_def.on_parse_opt);
    ASSERT_EQ(strcmp(module->id, id), 0);
    ASSERT_EQ(module->data, nullptr);
    ASSERT_EQ(last_module_data, nullptr);

    ASSERT_EQ(hpd_free(hpd), HPD_E_SUCCESS);
}

TEST(CASE, module_run_1sec) {
    hpd_t *hpd;
    ev_timer timer;
    int argc = 0;
    char *argv[] = {
            (char *) "/usr/local/bin/hpd",
            nullptr
    };
    const char *id = "mod";
    hpd_module_def_t module_def { on_create, on_destroy, on_start, on_stop, on_parse_opt };

    ASSERT_EQ(hpd_alloc(&hpd), HPD_E_SUCCESS);
    hpd_ev_loop_t *loop = hpd->loop;
    ASSERT_EQ(hpd_module(hpd, id, &module_def), HPD_E_SUCCESS);

    ev_init(&timer, stop_hpd);
    timer.repeat = 0.250;
    timer.data = hpd;
    ev_timer_again(loop, &timer);

    ASSERT_EQ(last_module_data, nullptr);
    ASSERT_EQ(hpd_start(hpd, argc, argv), HPD_E_SUCCESS);
    ASSERT_NE(hpd->loop, nullptr);
    ASSERT_NE(last_module_data, nullptr);
    ASSERT_EQ(last_module_data->create_called, 1);
    ASSERT_EQ(last_module_data->start_called, 2);
    ASSERT_EQ(last_module_data->stop_called, 3);
    ASSERT_EQ(last_module_data->destroy_called, 4);
    ASSERT_EQ(last_module_data->parse_opt_called, 0);
    ASSERT_EQ(last_module_data->start_loop, loop);
    ASSERT_EQ(last_module_data->stop_loop, loop);
    free(last_module_data);
    last_module_data = nullptr;

    ASSERT_EQ(hpd_free(hpd), HPD_E_SUCCESS);
}

TEST(CASE, option_no_call_1sec) {
    hpd_t *hpd;
    ev_timer timer;
    int argc = 1;
    char *argv[] = {
            (char *) "/usr/local/bin/hpd",
            nullptr
    };
    const char *id = "mod";
    hpd_module_def_t module_def { on_create_add_options, on_destroy, on_start, on_stop, on_parse_opt };

    ASSERT_EQ(hpd_alloc(&hpd), HPD_E_SUCCESS);
    hpd_ev_loop_t *loop = hpd->loop;
    ASSERT_EQ(hpd_module(hpd, id, &module_def), HPD_E_SUCCESS);

    ev_init(&timer, stop_hpd);
    timer.repeat = 0.250;
    timer.data = hpd;
    ev_timer_again(loop, &timer);

    ASSERT_EQ(last_module_data, nullptr);
    ASSERT_EQ(hpd_start(hpd, argc, argv), HPD_E_SUCCESS);
    ASSERT_NE(hpd->loop, nullptr);
    ASSERT_NE(last_module_data, nullptr);
    ASSERT_EQ(last_module_data->create_called, 1);
    ASSERT_EQ(last_module_data->start_called, 2);
    ASSERT_EQ(last_module_data->stop_called, 3);
    ASSERT_EQ(last_module_data->destroy_called, 4);
    ASSERT_EQ(last_module_data->parse_opt_called, 0);
    ASSERT_EQ(last_module_data->start_loop, loop);
    ASSERT_EQ(last_module_data->stop_loop, loop);
    ASSERT_EQ(last_module_data->add_option1, HPD_E_SUCCESS);
    ASSERT_EQ(last_module_data->add_option2, HPD_E_SUCCESS);
    for (int i = 0; i < 21; i++)
        ASSERT_EQ(last_module_data->option_errors[i], 0);
    free(last_module_data);
    last_module_data = nullptr;

    ASSERT_EQ(hpd_free(hpd), HPD_E_SUCCESS);
}

TEST(CASE, option_call1_1sec) {
    hpd_t *hpd;
    ev_timer timer;
    int argc = 2;
    char *argv[] = {
            (char *) "/usr/local/bin/hpd",
            (char *) "--mod-opt1",
            nullptr
    };
    const char *id = "mod";
    hpd_module_def_t module_def { on_create_add_options, on_destroy, on_start, on_stop, on_parse_opt };

    ASSERT_EQ(hpd_alloc(&hpd), HPD_E_SUCCESS);
    hpd_ev_loop_t *loop = hpd->loop;
    ASSERT_EQ(hpd_module(hpd, id, &module_def), HPD_E_SUCCESS);

    ev_init(&timer, stop_hpd);
    timer.repeat = 0.250;
    timer.data = hpd;
    ev_timer_again(loop, &timer);

    ASSERT_EQ(last_module_data, nullptr);
    ASSERT_EQ(hpd_start(hpd, argc, argv), HPD_E_SUCCESS);
    ASSERT_NE(hpd->loop, nullptr);
    ASSERT_NE(last_module_data, nullptr);
    ASSERT_EQ(last_module_data->create_called, 1);
    ASSERT_EQ(last_module_data->opt1_called, 2);
    ASSERT_EQ(last_module_data->opt2_called, 0);
    ASSERT_EQ(last_module_data->parse_opt_called, 0);
    ASSERT_EQ(last_module_data->start_called, 3);
    ASSERT_EQ(last_module_data->stop_called, 4);
    ASSERT_EQ(last_module_data->destroy_called, 5);
    ASSERT_EQ(last_module_data->start_loop, loop);
    ASSERT_EQ(last_module_data->stop_loop, loop);
    ASSERT_EQ(last_module_data->add_option1, HPD_E_SUCCESS);
    ASSERT_EQ(last_module_data->add_option2, HPD_E_SUCCESS);
    for (int i = 0; i < 21; i++)
        ASSERT_EQ(last_module_data->option_errors[i], 0);
    free(last_module_data);
    last_module_data = nullptr;

    ASSERT_EQ(hpd_free(hpd), HPD_E_SUCCESS);
}

TEST(CASE, option_call2_1sec) {
    hpd_t *hpd;
    ev_timer timer;
    int argc = 3;
    char *argv[] = {
            (char *) "/usr/local/bin/hpd",
            (char *) "--mod-opt1",
            (char *) "--mod-opt2",
            nullptr
    };
    const char *id = "mod";
    hpd_module_def_t module_def { on_create_add_options, on_destroy, on_start, on_stop, on_parse_opt };

    ASSERT_EQ(hpd_alloc(&hpd), HPD_E_SUCCESS);
    hpd_ev_loop_t *loop = hpd->loop;
    ASSERT_EQ(hpd_module(hpd, id, &module_def), HPD_E_SUCCESS);

    ev_init(&timer, stop_hpd);
    timer.repeat = 0.250;
    timer.data = hpd;
    ev_timer_again(loop, &timer);

    ASSERT_EQ(last_module_data, nullptr);
    ASSERT_EQ(hpd_start(hpd, argc, argv), HPD_E_SUCCESS);
    ASSERT_NE(hpd->loop, nullptr);
    ASSERT_NE(last_module_data, nullptr);
    ASSERT_EQ(last_module_data->create_called, 1);
    ASSERT_EQ(last_module_data->opt1_called, 2);
    ASSERT_EQ(last_module_data->opt2_called, 3);
    ASSERT_EQ(last_module_data->parse_opt_called, 0);
    ASSERT_EQ(last_module_data->start_called, 4);
    ASSERT_EQ(last_module_data->stop_called, 5);
    ASSERT_EQ(last_module_data->destroy_called, 6);
    ASSERT_EQ(last_module_data->start_loop, loop);
    ASSERT_EQ(last_module_data->stop_loop, loop);
    ASSERT_EQ(last_module_data->add_option1, HPD_E_SUCCESS);
    ASSERT_EQ(last_module_data->add_option2, HPD_E_SUCCESS);
    for (int i = 0; i < 21; i++)
        ASSERT_EQ(last_module_data->option_errors[i], 0);
    free(last_module_data);
    last_module_data = nullptr;

    ASSERT_EQ(hpd_free(hpd), HPD_E_SUCCESS);
}

