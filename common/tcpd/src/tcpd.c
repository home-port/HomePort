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
 * THIS SOFTWARE IS PROVidED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
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

#include "tcpd_intern.h"
#include "hpd_shared_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ev.h>
#include <fcntl.h>

/**
 * Get the in_addr from a sockaddr (IPv4 or IPv6)
 *
 *  Get the in_addr for either IPv4 or IPv6. The type depends on the
 *  protocal, which is why this returns a void pointer. It works nicely
 *  to pass the result of this function as the second argument to
 *  inet_ntop().
 *
 *  \param sa The sockaddr
 *
 *  \return An in_addr or in6_addr depending on the protocol.
 */
static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        // IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr.s_addr);
    } else {
        // IPv6
        return &(((struct sockaddr_in6*)sa)->sin6_addr.s6_addr);
    }
}

/**
 * Recieve callback for io-watcher.
 *
 * Recieves up to max_data_size (from hpd_tcpd_settings_t) of data from a
 * connection and calls on_recieve with it. Also resets the timeout for
 * the connection, if one.
 *
 * \param  loop     The event loop
 * \param  watcher  The io watcher causing the call
 * \param  revents  Not used
 */
static void on_ev_recv(hpd_ev_loop_t *loop, struct ev_io *watcher, int revents)
{
    ssize_t received;
    hpd_tcpd_conn_t *conn = watcher->data;
    hpd_tcpd_settings_t *settings = &conn->tcpd->settings;
    size_t max_data_size = settings->max_data_size;
    char buffer[max_data_size];
    hpd_module_t *context = conn->tcpd->context;

    HPD_LOG_VERBOSE(context, "Receiving data from %s", conn->ip);
    if ((received = recv(watcher->fd, buffer, max_data_size-1, 0)) < 0) {
        int err = errno;
        if (received == -1 && errno == EWOULDBLOCK) {
            HPD_LOG_WARN(context, "libev callback called without data to receive (conn: %s)", conn->ip);
            return;
        }
        HPD_LOG_ERROR(context, "recv(): %s", strerror(err));
        if (hpd_tcpd_conn_kill(conn)) HPD_LOG_ERROR(context, "Failed to kill connection.");
        return;
    } else if (received == 0) {
        HPD_LOG_INFO(context, "Connection closed by %s\n", conn->ip);
        if (hpd_tcpd_conn_kill(conn)) HPD_LOG_ERROR(context, "Failed to kill connection.");
        return;
    }

    if (settings->on_receive) {
        if (settings->on_receive(conn->tcpd, conn,
                                 settings->tcpd_ctx, &conn->ctx,
                                 buffer, (size_t) received)) {
            HPD_LOG_ERROR(context, "Failed to handle new data, killing it.");
            if (hpd_tcpd_conn_kill(conn)) HPD_LOG_ERROR(context, "Failed to kill connection.");
            return;
        }
    }

    // Reset timeout
    if (conn->timeout) {
        conn->timeout_watcher.repeat = conn->tcpd->settings.timeout;
        ev_timer_again(loop, &conn->timeout_watcher);
    }
}

/**
 * Send callback for io-watcher
 *
 * Sends message stored in send_msg on the connection. If not all the
 * data could be sent at once, the remainer is store in send_msg again
 * and the watcher is not stopped. If a connection is flaggted with
 * close, the connection is closed when all the data has been sent.
 *
 * \param  loop     The event loop
 * \param  watcher  The io watcher causing the call
 * \param  revents  Not used
 */
static void on_ev_send(hpd_ev_loop_t *loop, struct ev_io *watcher, int revents)
{
    hpd_tcpd_conn_t *conn = watcher->data;
    ssize_t sent;
    hpd_module_t *context = conn->tcpd->context;

    sent = send(watcher->fd, conn->send_msg, conn->send_len, 0);
    if (sent < 0) {
        int err = errno;
        HPD_LOG_ERROR(context, "send(): %s", strerror(err));
    } else if (sent == conn->send_len) {
        free(conn->send_msg);
        conn->send_msg = NULL;
        conn->send_len = 0;
    } else {
        conn->send_len -= sent;
        char *s = malloc(conn->send_len*sizeof(char));
        if (!s) {
            HPD_LOG_ERROR(context, "Cannot allocate enough memory.");
            free(conn->send_msg);
            conn->send_msg = NULL;
            conn->send_len = 0;
        } else {
            strcpy(s, &conn->send_msg[sent]);
            free(conn->send_msg);
            conn->send_msg = s;
            return;
        }
    }

    ev_io_stop(conn->tcpd->loop, &conn->send_watcher);
    if (conn->send_close && hpd_tcpd_conn_kill(conn)) HPD_LOG_ERROR(context, "Failed to kill connection.");
}

/**
 * Timeout callback for timeout watcher.
 *
 * Kills the connection on timeout
 *
 * \param  loop     The event loop
 * \param  watcher  The io watcher causing the call
 * \param  revents  Not used
 */
static void on_ev_timeout(hpd_ev_loop_t *loop, struct ev_timer *watcher, int revents)
{
    hpd_tcpd_conn_t *conn = watcher->data;
    hpd_module_t *context = conn->tcpd->context;
    
    HPD_LOG_INFO(context, "Timeout on %s [%ld].", conn->ip, (long)conn);
    if (hpd_tcpd_conn_kill(conn)) HPD_LOG_ERROR(context, "Failed to kill connection.");
}

/**
 * Initialise and accept connection
 *
 *  This function is designed to be used as a callback function within
 *  LibEV. It will accept the conncetion as described inside the file
 *  descripter within the watcher. It will also add timeout and io
 *  watchers to the loop, which will handle the further communication
 *  with the connection.
 *
 *  \param loop The running event loop.
 *  \param watcher The watcher that was tiggered on the connection.
 *  \param revents Not used.
 */
static void on_ev_conn(hpd_ev_loop_t *loop, struct ev_io *watcher, int revents)
{
    char ip_string[INET6_ADDRSTRLEN];
    int in_fd;
    socklen_t in_size;
    struct sockaddr_storage in_addr_storage;
    struct sockaddr *in_addr = (struct sockaddr *)&in_addr_storage;
    hpd_tcpd_conn_t *conn;
    hpd_tcpd_t *tcpd = (hpd_tcpd_t *)watcher->data;
    hpd_tcpd_settings_t *settings = &tcpd->settings;
    hpd_module_t *context = tcpd->context;

    // Accept connection
    in_size = sizeof *in_addr;
    if ((in_fd = accept(watcher->fd, in_addr, &in_size)) < 0) {
        int err = errno;
        HPD_LOG_ERROR(context, "accept(): %s", strerror(err));
        return;
    }

    // Print a nice message
    inet_ntop(in_addr_storage.ss_family, get_in_addr(in_addr), ip_string, sizeof ip_string);
    HPD_LOG_INFO(context, "Got connection from %s.", ip_string);

    // Create conn and parser
    conn = malloc(sizeof(hpd_tcpd_conn_t));
    if (conn == NULL) {
        HPD_LOG_ERROR(context, "Cannot allocation memory for connection.");
        close(in_fd);
        return;
    }
    conn->tcpd = watcher->data;
    strcpy(conn->ip, ip_string);
    conn->timeout_watcher.data = conn;
    conn->recv_watcher.data = conn;
    conn->send_watcher.data = conn;
    conn->ctx = NULL;
    conn->send_msg = NULL;
    conn->send_len = 0;
    conn->send_close = 0;
    conn->timeout = 1;

    // Add connection to list
    TAILQ_INSERT_TAIL(&conn->tcpd->conns, conn, HPD_TAILQ_FIELD);

    // Call back
    if (settings->on_connect) {
        if (settings->on_connect(conn->tcpd, conn, settings->tcpd_ctx, &conn->ctx)) {
            HPD_LOG_ERROR(context, "Failed to handle new connection, killing it.");
            if (hpd_tcpd_conn_kill(conn)) HPD_LOG_ERROR(context, "Failed to kill connection.");
            return;
        }
    }

    // Start timeout and io watcher
    ev_io_init(&conn->recv_watcher, on_ev_recv, in_fd, EV_READ);
    ev_io_init(&conn->send_watcher, on_ev_send, in_fd, EV_WRITE);
    ev_io_start(loop, &conn->recv_watcher);
    ev_init(&conn->timeout_watcher, on_ev_timeout);
    conn->timeout_watcher.repeat = settings->timeout;
    if (conn->timeout)
        ev_timer_again(loop, &conn->timeout_watcher);
}

/**
 *  Create new tcpd instance.
 *
 *  This creates a new tcpd instances, that can be started with
 *  hpd_tcpd_start() and stopped with hpd_tcpd_stop(). The instance should be
 *  destroyed with hpd_tcpd_destroy when not longer needed.
 *
 *  \param  settings  The settings for the tcpd.
 *  \param  loop      The event loop to run tcpd on.
 *
 *  \return  The new tcpd instance.
 */
hpd_error_t hpd_tcpd_create(hpd_tcpd_t **tcpd, hpd_tcpd_settings_t *settings, hpd_module_t *context,
                            hpd_ev_loop_t *loop)
{
    if (settings == NULL || context == NULL || loop == NULL) return HPD_E_NULL;
    if (settings->port <= HPD_P_SYSTEM_PORTS_START || settings->port > HPD_P_DYNAMIC_PORTS_END) return HPD_E_ARGUMENT;

    (*tcpd) = malloc(sizeof(hpd_tcpd_t));
    if (!(*tcpd)) return HPD_E_ALLOC;

    memcpy(&(*tcpd)->settings, settings, sizeof(hpd_tcpd_settings_t));
    sprintf((*tcpd)->port_str, "%i", settings->port);

    (*tcpd)->context = context;
    (*tcpd)->loop = loop;
    TAILQ_INIT(&(*tcpd)->conns);

    return HPD_E_SUCCESS;
}

/**
 * Destroy tcpd and free used memory.
 *
 *  This function destroys and frees all connections are instances. The
 *  tcpd should be stopped before destroy to properly close all
 *  connections and sockets first.
 *
 *  \param  instance  The tcpd instance to destroy
 */
hpd_error_t hpd_tcpd_destroy(hpd_tcpd_t *tcpd)
{
    if (!tcpd) return HPD_E_NULL;
    free(tcpd);
    return HPD_E_SUCCESS;
}

/**
 * Start the tcpd.
 *
 *  The libev-based tcpd is added to an event loop by a call to
 *  this function. It is the caller's resposibility to start the
 *  event loop.
 *
 *  To stop the tcpd again, one may call hpd_tcpd_stop().
 *
 *  \param instance The tcpd instance to start. Created with
 *  hpd_tcpd_create();
 *
 *  \return  0 on success, 1 on error.
 */
hpd_error_t hpd_tcpd_start(hpd_tcpd_t *tcpd)
{
    if (!tcpd) return HPD_E_NULL;
    
    int rc;
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;

    // Clear struct and set requirements for socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // IPv4/IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP Stream
    hints.ai_flags = AI_PASSIVE;     // Wildcard address

    // Get address infos we later use this to open socket
    if ((rc = getaddrinfo(NULL, tcpd->port_str, &hints, &servinfo)) != 0) {
        switch (rc) {
            case EAI_SYSTEM: {
                int err = errno;
                HPD_LOG_ERROR(tcpd->context, "getaddrinfo() failed with: %s", gai_strerror(rc));
                HPD_LOG_ERROR(tcpd->context, "System error message: %s", strerror(err));
                freeaddrinfo(servinfo);
                return HPD_E_UNKNOWN;
            }
            default: {
                HPD_LOG_ERROR(tcpd->context, "getaddrinfo() failed with: %s", gai_strerror(rc));
                freeaddrinfo(servinfo);
                return HPD_E_UNKNOWN;
            }
        }
    }

    // Loop through results and bind to first
    for (p = servinfo; p != NULL; p=p->ai_next) {

        // Create socket
        if ((tcpd->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            int err = errno;
            HPD_LOG_DEBUG(tcpd->context, "socket() failed with: %s", strerror(err));
            continue;
        }

        // Change to non-blocking sockets
        if (fcntl(tcpd->sockfd, F_SETFL, O_NONBLOCK)) {
            int err = errno;
            HPD_LOG_DEBUG(tcpd->context, "fcntl() failed with: %s", strerror(err));
            close(tcpd->sockfd);
            continue;
        }

        // Reuse addr for testing purposes
#ifdef DEBUG
        int yes = 1;
        if (setsockopt(tcpd->sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            int err = errno;
            HPD_LOG_DEBUG(tcpd->context, "setsockopt() failed with: %s", strerror(err));
            close(tcpd->sockfd);
            continue;
        }
#endif

        // Bind to socket
        if (bind(tcpd->sockfd, p->ai_addr, p->ai_addrlen) != 0) {
            int err = errno;
            HPD_LOG_DEBUG(tcpd->context, "bind() failed with: %s", strerror(err));
            close(tcpd->sockfd);
            continue;
        }

        break;
    }

    // Check if we binded to anything
    if (p == NULL) {
        close(tcpd->sockfd);
        freeaddrinfo(servinfo);
        return HPD_E_UNKNOWN;
    }

    // Clean up
    freeaddrinfo(servinfo);

    // Listen on socket
    if (listen(tcpd->sockfd, SOMAXCONN) < 0) {
        int err = errno;
        HPD_LOG_DEBUG(tcpd->context, "listen() failed with: %s", strerror(err));
        close(tcpd->sockfd);
        return HPD_E_UNKNOWN;
    }

    // Set listener on libev
    tcpd->watcher.data = tcpd;
    ev_io_init(&tcpd->watcher, on_ev_conn, tcpd->sockfd, EV_READ);
    ev_io_start(tcpd->loop, &tcpd->watcher);

    return HPD_E_SUCCESS;
}

/**
 * Stop an already running tcpd.
 *
 *  The tcpd, started with hpd_tcpd_start(), may be stopped by calling
 *  this function. It will take the tcpd off the event loop and
 *  clean up after it.
 *
 *  This includes killing all connections without waiting for
 *  remaining data to be sent.
 *
 *  \param instance The tcpd instance to stop.
 */
hpd_error_t hpd_tcpd_stop(hpd_tcpd_t *tcpd)
{
    if (!tcpd) return HPD_E_NULL;
    
    hpd_tcpd_conn_t *conn, *tmp;
    hpd_module_t *context = tcpd->context;

    // Stop accept watcher
    ev_io_stop(tcpd->loop, &tcpd->watcher);

    // Kill all connections
    HPD_TAILQ_FOREACH_SAFE(conn, &tcpd->conns, tmp)
        if (hpd_tcpd_conn_kill(conn)) HPD_LOG_ERROR(context, "Failed to kill connection.");

    // Close socket
    if (close(tcpd->sockfd) != 0) {
        int err = errno;
        HPD_LOG_ERROR(context, "close(): %s", strerror(err));
        return HPD_E_UNKNOWN;
    }

    return HPD_E_SUCCESS;
}

/**
 * Get the IP address of the client
 *
 *  \param  conn  The connection on which the client is connected.
 *
 *  \return  The IP address in a string.
 */
hpd_error_t hpd_tcpd_conn_get_ip(hpd_tcpd_conn_t *conn, const char **ip)
{
    if (!conn || !ip) return HPD_E_NULL;
    (*ip) = conn->ip;
    return HPD_E_SUCCESS;
}


/**
 * Disable timeout on connection
 *
 *  Every connection have per default a timeout value, which is set in
 *  hpd_tcpd_settings_t. If there is no activity on the connection
 *  before the timeout run out the connection is killed. This function
 *  disables the timeout, so connections will stay open. A connection
 *  will still be killed when the client closes the connection, or
 *  kill/close is called.
 *
 *  \param  conn  The connection to keep open
 */
hpd_error_t hpd_tcpd_conn_keep_open(hpd_tcpd_conn_t *conn)
{
    if (!conn) return HPD_E_NULL;
    conn->timeout = 0;
    ev_timer_stop(conn->tcpd->loop, &conn->timeout_watcher);
    return HPD_E_SUCCESS;
}


/**
 * Send message on connection
 *
 * This function is used similary to the standard printf function, with
 * a format string and variable arguments. It calls hpd_tcpd_conn_vsendf() to
 * handle the actually sending, see this for more information.
 *
 * Connection is kept open for further communication, use hpd_tcpd_conn_close
 * to close it.
 *
 * \param  conn  Connection to send on
 * \param  fmt   Format string
 *
 * \return The same as hpd_tcpd_conn_vsendf()
 */
hpd_error_t hpd_tcpd_conn_sendf(hpd_tcpd_conn_t *conn, const char *fmt, ...)
{
    if (!conn) return HPD_E_NULL;
    va_list arg;
    va_start(arg, fmt);
    hpd_error_t rc = hpd_tcpd_conn_vsendf(conn, fmt, arg);
    va_end(arg);
    return rc;
}


/**
 * Send message on connection
 *
 * This function is simiar to the standard vprintf function, with a
 * format string and a list of variable arguments.
 *
 * Note that this function only schedules the message to be send. A send
 * watcher on the event loop will trigger the actual sending, when the
 * connection is ready for it.
 *
 * Connection is kept open for further communication, use hpd_tcpd_conn_close
 * to close it.
 *
 * \param  conn  Connection to send on
 * \param  fmt   Format string
 * \param  arg   List of arguments
 *
 * \return  zero on success, -1 or the return value of vsprintf on
 *          failure
 */
hpd_error_t hpd_tcpd_conn_vsendf(hpd_tcpd_conn_t *conn, const char *fmt, va_list vp)
{
    if (!conn) return HPD_E_NULL;

    char *new_msg;
    int new_len;
    va_list vp2;

    // Copy arg to avoid errors on 64bit
    va_copy(vp2, vp);

    // Get the length to expand with
    new_len = vsnprintf("", 0, fmt, vp);
    if (new_len < 0) {
        int err = errno;
        HPD_LOG_DEBUG(conn->tcpd->context, "vsnprintf(): %s", strerror(err));
        return HPD_E_UNKNOWN;
    }

    // Expand message to send
    new_msg = realloc(conn->send_msg,
                      (conn->send_len + new_len + 1)*sizeof(char));
    if (new_msg == NULL) HPD_LOG_RETURN_E_ALLOC(conn->tcpd->context);
    conn->send_msg = new_msg;

    // Concatenate strings
    vsprintf(&(conn->send_msg[conn->send_len]), fmt, vp2);

    // Start send watcher
    if (conn->send_len == 0 && conn->tcpd != NULL)
        ev_io_start(conn->tcpd->loop, &conn->send_watcher);

    // Update length
    conn->send_len += new_len;

    return HPD_E_SUCCESS;
}

/**
 * Close a connection, after the remaining data has been sent
 *
 * This sets the close flag on a connection. The connection will be
 * closed after the remaining messages has been sent. If there is no
 * waiting messages the connection will be closed instantly.
 *
 * \param  conn  The connection to close
 */
hpd_error_t hpd_tcpd_conn_close(hpd_tcpd_conn_t *conn)
{
    if (!conn) return HPD_E_NULL;

    hpd_module_t *context = conn->tcpd->context;

    conn->send_close = 1;

    if (conn->send_msg == NULL) {
        ev_io_stop(conn->tcpd->loop, &conn->send_watcher);
        if (conn->send_close && hpd_tcpd_conn_kill(conn)) HPD_LOG_ERROR(context, "Failed to kill connection.");
    }

    return HPD_E_SUCCESS;
}

/**
 * Kill and clean up after a connection
 *
 *  This function stops the LibEV watchers, closes the socket, and frees
 *  the data structures used by a connection.
 *
 *  Note that you should use hpd_tcpd_conn_close for a graceful closure of the
 *  connection, where the remaining data is sent.
 *
 *  \param conn The connection to kill.
 */
hpd_error_t hpd_tcpd_conn_kill(hpd_tcpd_conn_t *conn)
{
    if (!conn) return HPD_E_NULL;

    hpd_tcpd_settings_t *settings = &conn->tcpd->settings;

    // Print messange
    HPD_LOG_INFO(conn->tcpd->context, "Killing connection from %s.", conn->ip);

    // Stop circular calls and only kill this connection once
    if (conn->recv_watcher.fd < 0) return HPD_E_SUCCESS;

    // Stop watchers
    ev_io_stop(conn->tcpd->loop, &conn->recv_watcher);
    ev_io_stop(conn->tcpd->loop, &conn->send_watcher);
    ev_timer_stop(conn->tcpd->loop, &conn->timeout_watcher);

    // Close socket
    if (close(conn->recv_watcher.fd) != 0) {
        int err = errno;
        HPD_LOG_ERROR(conn->tcpd->context, "close(): %s", strerror(err));
    }
    conn->recv_watcher.fd = -1;

    // Remove from list
    TAILQ_REMOVE(&conn->tcpd->conns, conn, HPD_TAILQ_FIELD);

    // Call back
    if (settings->on_disconnect && settings->on_disconnect(conn->tcpd, conn, settings->tcpd_ctx, &conn->ctx))
        HPD_LOG_ERROR(conn->tcpd->context, "Failed to disconnect.");

    // Cleanup
    free(conn->send_msg);
    free(conn);

    return HPD_E_SUCCESS;
}
