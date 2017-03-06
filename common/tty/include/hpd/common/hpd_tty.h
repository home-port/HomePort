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

#ifndef HOMEPORT_HPD_TTY_H
#define HOMEPORT_HPD_TTY_H

#include <ev.h>
#include <termios.h>
#include <hpd/hpd_types.h>

typedef struct hpd_tty hpd_tty_t;

typedef hpd_error_t hpd_tty_f(void *data);
typedef hpd_error_t hpd_tty_msg_f(void *data, const unsigned char *msg, size_t len);

hpd_error_t hpd_tty_open(hpd_tty_t **tty, const hpd_module_t *context, hpd_ev_loop_t *loop,
                         const char *dev, speed_t baud, hpd_tty_msg_f on_data, hpd_tty_f on_close, void *data);
hpd_error_t hpd_tty_close(struct hpd_tty *tty);
hpd_error_t hpd_tty_write(struct hpd_tty *tty, const unsigned char *msg, int len);


#endif //HOMEPORT_HPD_TTY_H
