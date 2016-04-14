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
#include "datamanager.h"

#define CASE datamanager

TEST(CASE, alloc) {
    char *ptr;
    ALLOC(ptr, 2, char);
    free(ptr);

    ASSERT_TRUE(ptr);
    return;

    alloc_error:
    FAIL();
}

TEST(CASE, str_cpy) {
    const char *src = "Test";
    char *dst;
    STR_CPY(dst, src);

    ASSERT_EQ(strcmp(src, dst), 0);
    free(dst);
    return;

    alloc_error:
    FAIL();
}

TEST(CASE, attrs_insert_remove) {
    const char *key = "key";
    const char *value = "value";
    map_t attrs;
    pair_t *attr = NULL;

    TAILQ_INIT(&attrs);

    ATTRS_INSERT(&attrs, key, value);

    attr = TAILQ_FIRST(&attrs);
    ASSERT_EQ(strcmp(attr->k, key), 0);
    ASSERT_EQ(strcmp(attr->v, value), 0);

    ATTRS_REMOVE(&attrs, attr);

    ASSERT_TRUE(TAILQ_EMPTY(&attrs));

    return;

    alloc_error:
    FAIL();
}

TEST(CASE, attrs_insert_remove_all) {
    const char *keys[] = { "key_a", "key_b", "key_c" };
    const char *vals[] = { "val_a", "val_b", "val_c" };
    int i;
    map_t attrs;
    pair_t *attr = NULL;

    TAILQ_INIT(&attrs);
    for (i = 0; i < 3; i++)
    ATTRS_INSERT(&attrs, keys[i], vals[i]);

    i = 0;
    HPD_TAILQ_FOREACH(attr, &attrs) {
        ASSERT_EQ(strcmp(attr->k, keys[i]), 0);
        ASSERT_EQ(strcmp(attr->v, vals[i]), 0);
        i++;
    }

    ATTRS_REMOVE_ALL(&attrs);
    ASSERT_TRUE(TAILQ_EMPTY(&attrs));

    return;

    alloc_error:
    FAIL();
}

TEST(CASE, attrs_foreach) {
    const char *keys[] = { "key_a", "key_b", "key_c" };
    const char *vals[] = { "val_a", "val_b", "val_c" };
    int i;
    map_t attrs;
    pair_t *attr = NULL;

    TAILQ_INIT(&attrs);
    for (i = 0; i < 3; i++)
        ATTRS_INSERT(&attrs, keys[i], vals[i]);

    i = 0;
    HPD_TAILQ_FOREACH(attr, &attrs) {
        ASSERT_EQ(strcmp(attr->k, keys[i]), 0);
        ASSERT_EQ(strcmp(attr->v, vals[i]), 0);
        i++;
    }

    ATTRS_REMOVE_ALL(&attrs);
    ASSERT_TRUE(TAILQ_EMPTY(&attrs));

    return;

    alloc_error:
    FAIL();
}

TEST(CASE, attrs_foreach_safe) {
    const char *keys[] = { "key_a", "key_b", "key_c" };
    const char *vals[] = { "val_a", "val_b", "val_c" };
    int i;
    map_t attrs;
    pair_t *attr = NULL, *tmp;

    TAILQ_INIT(&attrs);
    for (i = 0; i < 3; i++)
    ATTRS_INSERT(&attrs, keys[i], vals[i]);

    i = 0;
    HPD_TAILQ_FOREACH_SAFE(attr, &attrs, tmp) {
        ASSERT_EQ(strcmp(attr->k, keys[i]), 0);
        ASSERT_EQ(strcmp(attr->v, vals[i]), 0);
        i++;
        ATTRS_REMOVE(&attrs, attr);
    }

    ASSERT_TRUE(TAILQ_EMPTY(&attrs));

    return;

    alloc_error:
    FAIL();
}

TEST(CASE, attrs_get) {
    const char *keys[] = { "key_a", "key_b", "key_c" };
    const char *vals[] = { "val_a", "val_b", "val_c" };
    int i;
    map_t attrs;
    pair_t *attr = NULL;

    TAILQ_INIT(&attrs);
    for (i = 0; i < 3; i++)
    ATTRS_INSERT(&attrs, keys[i], vals[i]);

    for (i = 0; i < 3; i++) {
        ATTRS_GET(attr, &attrs, keys[i]);
        ASSERT_EQ(strcmp(attr->k, keys[i]), 0);
        ASSERT_EQ(strcmp(attr->v, vals[i]), 0);
        i++;
    }

    ATTRS_REMOVE_ALL(&attrs);
    ASSERT_TRUE(TAILQ_EMPTY(&attrs));

    return;

    alloc_error:
    FAIL();
}

TEST(CASE, param_alloc_free) {
    parameter_t *param;
    PARAM_ALLOC(param);
    ASSERT_TRUE(param);
    PARAM_FREE(param);
    SUCCEED();
    return;

    alloc_error:
    FAIL();
}

TEST(CASE, service_alloc_free) {
    service_t *srv;
    SERVICE_ALLOC(srv);
    ASSERT_TRUE(srv);
    SERVICE_FREE(srv);
    SUCCEED();
    return;

    alloc_error:
    FAIL();
}
