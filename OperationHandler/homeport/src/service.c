/*Copyright 2011 Aalborg University. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of
  conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list
  of conditions and the following disclaimer in the documentation and/or other materials
  provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed*/

#include "homeport.h"
#include "datamanager.h"
#include "hpd_error.h"
#include "hp_macros.h"
#include "lr_interface.h"
#include "libREST.h"
#include "json.h"
#include "xml.h"
#include <ev.h>

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
homePortNew( struct ev_loop *loop, int port )
{
  HomePort *homeport;
  alloc_struct(homeport);

  struct lr_settings settings = LR_SETTINGS_DEFAULT;
  settings.port = port;

  homeport->loop = loop;

  homeport->rest_interface = lr_create(&settings, loop);
  if ( homeport->rest_interface == NULL )
  {
    goto cleanup;
  }

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
    lr_destroy(homeport->rest_interface);
    configurationFree(homeport->configuration);
    free(homeport);
  }
}

int
homePortStart(HomePort *homeport)
{
  if(lr_start(homeport->rest_interface))
    return HPD_E_MHD_ERROR;

  return lr_register_service(homeport->rest_interface,
      "/devices",
      lri_getConfiguration, NULL, NULL, NULL,
      NULL, homeport);
}

void
homePortStop(HomePort *homeport)
{
  lr_stop(homeport->rest_interface);
}

static void
sig_cb ( struct ev_loop *loop, struct ev_signal *w, int revents )
{
  HomePort *homeport = (HomePort*)((void**)w->data)[0];
  void (*deinit)(HomePort *, void *) = ((void **)w->data)[1];

  // Call deinit
  // TODO Might be a problem that deinit is not called on ws_stop, but
  // only if the server is stopped by a signal. Note that this is only
  // used in HPD_easy way of starting the server.
  if (deinit)
     deinit(homeport, ((void **)w->data)[2]);


  // Stop server and loop
  homePortStop(homeport);
  homePortFree(homeport);
  ev_break(loop, EVBREAK_ALL);

  // TODO Isn't it bad that we down stop watcher here?
  free(w->data);
}

int
homePortEasy(init_f init, deinit_f deinit, void *data, int port )
{
  int rc;

  // Create loop
  struct ev_loop *loop = EV_DEFAULT;

  HomePort *homeport = homePortNew(loop, port);

  // Create signal watchers
  struct ev_signal sigint_watcher;
  struct ev_signal sigterm_watcher;
  ev_signal_init(&sigint_watcher, sig_cb, SIGINT);
  ev_signal_init(&sigterm_watcher, sig_cb, SIGTERM);
  void **w_data = malloc(3*sizeof(void *));
  w_data[0] = homeport;
  w_data[1] = deinit;
  w_data[2] = data;
  sigint_watcher.data = w_data;
  sigterm_watcher.data = w_data;
  ev_signal_start(loop, &sigint_watcher);
  ev_signal_start(loop, &sigterm_watcher);

  // Call init
  if (init)
     if ((rc = init(homeport, data))) return rc;

  if( ( rc = homePortStart(homeport) ) ) return rc;

  // Start loop
  ev_run(loop, 0);

  return 0;
}


