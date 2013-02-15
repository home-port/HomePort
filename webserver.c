// webserver.c

#include "webserver.h"
#include "accept.h"
#include "client.h"

#include <ev.h>

void ws_start(char *port)   
{
   struct ev_loop *loop = EV_DEFAULT;
   struct ev_io *watcher = ws_acc_init(loop, port, ws_cli_init);
   ev_run(loop, 0);
   ws_acc_deinit(watcher);
}

void ws_stop()
{
   ev_break(EV_DEFAULT, EVBREAK_ALL);
}

