// main.c

#include <stdio.h>
#include "webserver.h"

int main()
{
   struct ws_instance ws_http;
   struct ev_loop *loop = EV_DEFAULT;

   // Init webserver and start it
   ws_init(&ws_http, loop);
   ws_start(&ws_http);

   // Start the loop
   ev_run(loop, 0);

   // Clean up the webserver
   ws_stop(&ws_http);

   return 0;
}
