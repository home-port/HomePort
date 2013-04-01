// callbacks.h

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "webserver.h"

struct ws_callbacks {
   request_cb header_cb;
   request_cb body_cb;
};

#endif
