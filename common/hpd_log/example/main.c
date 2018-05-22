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

#include <hpd-0.6/hpd_daemon_api.h>
#include <hpd-0.6/modules/hpd_log.h>
#include <hpd-0.6/modules/hpd_rest.h>

int main(int argc, char *argv[])
{
    hpd_error_t rc;
    hpd_t *hpd;

    // Allocate hpd memory
    if ((rc = hpd_alloc(&hpd))) goto error_return;

    // Add modules
    if ((rc = hpd_module(hpd, "log", &hpd_log))) goto error_free;
    if ((rc = hpd_module(hpd, "rest", &hpd_rest))) goto error_free;

    // Start hpd
    if ((rc = hpd_start(hpd, argc, argv))) goto error_free;

    // Clean up
    if ((rc = hpd_free(hpd))) goto error_return;

    return rc;

    error_free:
        hpd_free(hpd);
    error_return:
        return rc;
}