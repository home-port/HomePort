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

#ifndef HOMEPORT_TCPD_INTERN_H
#define HOMEPORT_TCPD_INTERN_H

#include "hpd/common/hpd_tcpd.h"
#include "hpd/hpd_types.h"
#include "hpd/common/hpd_queue.h"
#include <ev.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

TAILQ_HEAD(hpd_tcpd_conns, hpd_tcpd_conn);
typedef struct hpd_tcpd_conns hpd_tcpd_conns_t;

/// Instance of a tcpd
struct hpd_tcpd {
    hpd_tcpd_settings_t settings; ///< Settings
    char port_str[6];           ///< Port number - as a string
    hpd_ev_loop_t *loop;        ///< Event loop
    hpd_tcpd_conns_t conns;       ///< Linked List of connections
    int sockfd;                 ///< Socket file descriptor
    ev_io watcher;              ///< New connection watcher
    const hpd_module_t *context;
};

/// All data to represent a connection
struct hpd_tcpd_conn {
    TAILQ_ENTRY(hpd_tcpd_conn) HPD_TAILQ_FIELD;
    hpd_tcpd_t *tcpd;        ///< Webserver instance
    char ip[INET6_ADDRSTRLEN]; ///< IP address of client
    ev_timer timeout_watcher;  ///< Timeout watcher
    int timeout;               ///< Restart timeout watcher ?
    ev_io recv_watcher;        ///< Recieve watcher
    ev_io send_watcher;        ///< Send watcher
    char *send_msg;            ///< Data to send
    size_t send_len;           ///< Length of data to send
    int send_close;            ///< Close socket after send ?
    void *ctx;                 ///< Connection context
};

#ifdef __cplusplus
}
#endif

#endif //HOMEPORT_TCPD_INTERN_H
