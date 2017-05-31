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

#include "hpd/common/hpd_tty.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <hpd/common/hpd_common.h>
#include <hpd/hpd_shared_api.h>

struct hpd_tty {
    hpd_ev_loop_t *loop;
    int tty_fd;
    char *tty_dev;
    hpd_tty_msg_f *on_data;
    hpd_tty_f *on_close;
    void *data;
    struct ev_io read_watcher;
    unsigned char *read_msg;
    size_t read_len;
    struct ev_io write_watcher;
    unsigned char *write_buffer;
    int write_len;
    const hpd_module_t *context;
};

static hpd_error_t tty_conn(hpd_tty_t *tty, speed_t baud)
{
    int tty_fd;
    struct termios tio;

    memset(&tio, 0, sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=0;
    tio.c_cflag=CS8|CREAD|CLOCAL; // 8n1, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=1;
    tio.c_cc[VTIME]=5;
    cfsetospeed(&tio, baud);
    cfsetispeed(&tio, baud);

    tty_fd = open(tty->tty_dev, O_RDWR | O_NDELAY);
    if (tty_fd <= 0) {
        HPD_LOG_RETURN(tty->context, HPD_E_UNKNOWN, "Error while opening device on %s: %s", tty->tty_dev, strerror(errno));
    } else {
        tcsetattr(tty_fd, TCSANOW, &tio);
        tty->tty_fd = tty_fd;
        return HPD_E_SUCCESS;
    }
}

static void
read_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    struct hpd_tty *tty = w->data;
    ssize_t received, len;
    unsigned char chr;

    if ((received = read(w->fd, &chr, 1)) < 0) {
        HPD_LOG_DEBUG(tty->context, "Error while reading from %s: %s", tty->tty_dev, strerror(errno));
    } else if (received == 0) {
        HPD_LOG_INFO(tty->context, "Connection on %s closed", tty->tty_dev);
        tty->on_close(tty->data);
        hpd_tty_close(tty);
    } else {
        // Append data
        len = tty->read_len + received + 1;
        tty->read_msg = realloc(tty->read_msg, len*sizeof(char));
        memcpy(&tty->read_msg[tty->read_len], &chr, received);
        tty->read_msg[len-1] = '\0';
        tty->read_len = len - 1;

        // Process data
        len = tty->on_data(tty->data, tty->read_msg, tty->read_len);

        // Remove data
        tty->read_len -= len;
        memmove(tty->read_msg, &tty->read_msg[len], tty->read_len);
        tty->read_msg = realloc(tty->read_msg, (tty->read_len+1)*sizeof(char));
    }
}

static void
write_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    struct hpd_tty *tty = w->data;
    size_t sent;
    int err;

    sent = write(tty->tty_fd, tty->write_buffer, 1);

    if (sent == -1) {
        err = errno;
        if (err == EAGAIN || err == EWOULDBLOCK) return;
        HPD_LOG_DEBUG(tty->context, "Error while writing to %s: %s", tty->tty_dev, strerror(err));
    } else if ( sent == 1) {
        tty->write_len -= sent;
        if (tty->write_len > 0) {
            unsigned char *str = malloc(tty->write_len*sizeof(char));
            memcpy(str, &tty->write_buffer[sent], tty->write_len);
            free(tty->write_buffer);
            tty->write_buffer = str;
            return;
        }
    } else {
        HPD_LOG_DEBUG(tty->context, "Unknown error while writing to %s", tty->tty_dev);
    }

    free(tty->write_buffer);
    tty->write_buffer = NULL;
    tty->write_len = 0;
    ev_io_stop(loop, &tty->write_watcher);
}

hpd_error_t hpd_tty_open(hpd_tty_t **tty, const hpd_module_t *context, hpd_ev_loop_t *loop,
                         const char *dev, speed_t baud, hpd_tty_msg_f on_data, hpd_tty_f on_close, void *data)
{
    if (!tty) HPD_LOG_RETURN_E_NULL(context);
    if (!context) HPD_LOG_RETURN_E_NULL(context);
    if (!loop) HPD_LOG_RETURN_E_NULL(context);
    if (!dev) HPD_LOG_RETURN_E_NULL(context);

    hpd_error_t rc;

    HPD_CALLOC(*tty, 1, hpd_tty_t);
    (*tty)->context = context;
    HPD_STR_CPY((*tty)->tty_dev, dev);
    (*tty)->loop = loop;
    (*tty)->read_msg = NULL;
    (*tty)->read_len = 0;
    (*tty)->write_buffer = NULL;
    (*tty)->write_len = 0;
    (*tty)->read_watcher.data = *tty;
    (*tty)->write_watcher.data = *tty;
    (*tty)->on_data = on_data;
    (*tty)->on_close = on_close;
    (*tty)->data = data;

    if ((rc = tty_conn(*tty, baud)) != HPD_E_SUCCESS) {
        hpd_tty_close((*tty));
        *tty = NULL;
        return rc;
    }

    ev_io_init(&(*tty)->read_watcher, read_cb, (*tty)->tty_fd, EV_READ);
    ev_io_init(&(*tty)->write_watcher, write_cb, (*tty)->tty_fd, EV_WRITE);

    ev_io_start(loop, &(*tty)->read_watcher);

    return HPD_E_SUCCESS;

    alloc_error:
        if (*tty) free(tty);
        HPD_LOG_RETURN_E_ALLOC(context);
}

hpd_error_t hpd_tty_close(struct hpd_tty *tty)
{
    if (!tty) HPD_LOG_RETURN_E_NULL(tty->context);

    if (tty->tty_fd > 0) {
        ev_io_stop(tty->loop, &tty->read_watcher);
        ev_io_stop(tty->loop, &tty->write_watcher);
        close(tty->tty_fd);
    }
    free(tty->read_msg);
    free(tty->write_buffer);
    free(tty->tty_dev);
    free(tty);

    return HPD_E_SUCCESS;
}

hpd_error_t hpd_tty_write(struct hpd_tty *tty, const unsigned char *msg, int len)
{
    if (!tty) HPD_LOG_RETURN_E_NULL(tty->context);
    if (!msg) HPD_LOG_RETURN_E_NULL(tty->context);

    if (tty->write_buffer == NULL)
        ev_io_start(tty->loop, &tty->write_watcher);

    if (len == -1) len = strlen((char *) msg);

    tty->write_buffer = realloc(tty->write_buffer, (tty->write_len+len) * sizeof(unsigned char));
    memcpy(&tty->write_buffer[tty->write_len], msg, len);
    tty->write_len += len;

    return HPD_E_SUCCESS;
}



