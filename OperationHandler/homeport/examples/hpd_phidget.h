#ifndef HPD_PHIDGET_H
#define HPD_PHIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <phidget21.h>
#include "homeport.h"
#include "utlist.h"

int phidget_init(struct ev_loop *loop);
void phidget_deinit();

#endif
