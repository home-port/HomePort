// ws_callbacks.h

#ifndef WS_CALLBACKS_H
#define WS_CALLBACKS_H

#include "webserver.h"

struct ws_callbacks {
   request_cb header_cb;
   request_cb body_cb;
};

#endif
