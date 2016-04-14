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

#include "my_adapter.h"
#include "my_application.h"

// With HomePort installed, these should be changed to <hpd/...>
#include "hpd_daemon_api.h"

struct simple {
    struct adp *my_adapter;
    struct app *my_app;
};

static error_t init(hpd_t *hpd, void *data)
{
    struct simple *simple = data;
    error_t rc;

    if ((rc = adp_init(simple->my_adapter, hpd)) != HPD_E_SUCCESS)
        return rc;

    if ((rc = app_init(simple->my_app, hpd)) != HPD_E_SUCCESS)
        return rc;

    return HPD_E_SUCCESS;
}

static error_t deinit(hpd_t *hpd, void *data)
{
    struct simple *simple = data;
    error_t rc;

    if ((rc = adp_deinit(simple->my_adapter)) != HPD_E_SUCCESS)
        return rc;

    if ((rc = app_deinit(simple->my_app)) != HPD_E_SUCCESS)
        return rc;

    return HPD_E_SUCCESS;
}

int main(int argc, char **argv)
{
    struct simple simple;
    error_t rc;

    if ((rc = adp_alloc(&simple.my_adapter)) != HPD_E_SUCCESS)
        return rc;

    if ((rc = app_alloc(&simple.my_app)) != HPD_E_SUCCESS)
        return rc;

    rc = hpd_easy(init, deinit, &simple);

    if (rc == HPD_E_SUCCESS) {
        if ((rc = adp_free(simple.my_adapter)) != HPD_E_SUCCESS)
            return rc;

        if ((rc = app_free(simple.my_app)) != HPD_E_SUCCESS)
            return rc;
    } else {
        adp_free(simple.my_adapter);
        app_free(simple.my_app);
    }

    return rc;
}
