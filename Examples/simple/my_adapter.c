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

#include <stdlib.h>

// With HomePort installed, these should be changed to <hpd/...>
#include "hpd_adapter_api.h"

struct adp {
    adapter_t *adapter;
    device_t *device;
    service_t *service1;
    service_t *service2;
    parameter_t *param1;
    parameter_t *param2;
};

error_t adp_alloc(struct adp **my)
{
    if (!my) return HPD_E_NULL;
    *my = malloc(sizeof(struct adp));
    if (!my) return HPD_E_ALLOC;
    return HPD_E_SUCCESS;
}

error_t adp_free(struct adp *my)
{
    free(my);
    return HPD_E_SUCCESS;
}

error_t adp_init(struct adp *my, hpd_t *hpd)
{
    error_t rc;

    if ((rc = hpd_adapter_alloc(&my->adapter)) != HPD_E_SUCCESS)
        return rc;

    if ((rc = hpd_adapter_set_attrs(my->adapter,
                                    HPD_ATTR_TYPE, "example",
                                    HPD_ATTR_DESC, "Simple example adapter")) != HPD_E_SUCCESS)
        return rc;

    if ((rc = hpd_device_alloc(&my->device)) != HPD_E_SUCCESS)
        return rc;

    if ((rc = hpd_service_alloc(&my->service1)) != HPD_E_SUCCESS)
        return rc;

    return HPD_E_SUCCESS;
}

error_t adp_deinit(struct adp *my)
{
    return HPD_E_SUCCESS;
}
