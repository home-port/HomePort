// ws_instance.h

#ifndef WS_INSTANCE_H
#define WS_INSTANCE_H

#include "webserver.h"
#include "ws_client.h"

struct ws_callbacks *ws_instance_get_callbacks(
      struct ws_instance *instance);

void ws_instance_set_first_client(
      struct ws_instance *instance,
      struct ws_client *client);

struct ws_client *ws_instance_get_first_client(
      struct ws_instance *instance);

#endif
