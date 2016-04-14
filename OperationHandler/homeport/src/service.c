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

/**
 * Allocate new HomePort instance
 *
 * @param hpd   Pointer will be set to the newly allocated HomePort structure on success.
 * @param loop  The event loop that will be stored in hpd and used by adapters.
 *
 * @return HPD_E_SUCCESS  on success.
 */
error_t hpd_alloc(hpd_t **hpd, ev_loop_t *loop)
{
    // TODO Implement this
    return HPD_E_SUCCESS;
}

/**
 * Free a HomePort instance
 *
 * @param hpd   HomePort instance to free.
 *
 * @return HPD_E_SUCCESS  on success.
 */
error_t hpd_free(hpd_t *hpd)
{
    // TODO Implement this
    return HPD_E_SUCCESS;
}

/**
 * Helper function to easily start an event loop with HomePort and adapters. Function will automatically call hpd_alloc
 * and hpd_free for you. Use the two function-pointers to start and stop any required adapters. Note that this function
 * will only return on SIGINT/SIGTERM or calls to ev_break.
 *
 * @param init    Init function that will be called before starting the loop. Will have valid pointers to HomePort and
 *                the event loop.
 * @param deinit  Deinit function that will be called before returning.
 * @param data    Custom data that will be parsed on to the init and deinit function.
 *
 * @return HPD_E_SUCCESS  on success.
 */
error_t hpd_easy(init_f init, deinit_f deinit, void *data)
{
    // TODO Implement this
    return HPD_E_SUCCESS;
}




#include "datamanager.h"
#include "hpd_error.h"
#include "hp_macros.h"
#include <ev.h>
#include "utlist.h"

/**
 * Creates a HomePort structure
 * 
Not Acceptable
 * @param option HPD option as specified in homeport.h
 *
 * @param hostname Name of the desired host
 *
 * @param ... va list of option, last option has to be HPD_OPTION_END
 *
 * @return A HPD error code
 */
HomePort*
homePortNew( struct ev_loop *loop )
{
    HomePort *homeport;
    alloc_struct(homeport);

    homeport->loop = loop;

    homeport->configuration = configurationNew();
    if( homeport->configuration == NULL )
    {
        goto cleanup;
    }

    return homeport;

    cleanup:
    homePortFree(homeport);
    return NULL;
}

/**
 * 
 * Destroys a HomePort struct
 * @return A HPD error code
 */
void
homePortFree(HomePort *homeport)
{
    if(homeport != NULL)
    {
        // Free listeners
        Listener *l, *tmp;
        DL_FOREACH_SAFE(homeport->configuration->listener_head, l, tmp) {
            homePortFreeListener(l);
        }
        configurationFree(homeport->configuration);
        free(homeport);
    }
}


