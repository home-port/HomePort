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

#ifndef HOMEPORT_TCPD_TYPES_H
#define HOMEPORT_TCPD_TYPES_H

// Port numbers are assigned according to
// http://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xml
// (Site may have very long loading time)
#define HPD_TCPD_PORT_MAP(XX) \
   XX(    0, SYSTEM_PORTS_START) \
   XX(   80, HTTP    ) \
   XX(  443, HTTPS   ) \
   XX( 1023, SYSTEM_PORTS_END) \
   XX( 1023, USER_PORTS_START) \
   XX( 8080, HTTP_ALT) \
   XX(49151, USER_PORTS_END) \
   XX(49152, DYNAMIC_PORTS_START) \
   XX(65535, DYNAMIC_PORTS_END)

typedef enum hpd_tcpd_port
{
#define XX(num, str) HPD_TCPD_P_##str = num,
    HPD_TCPD_PORT_MAP(XX)
#undef XX
} hpd_tcpd_port_t;

#endif //HOMEPORT_TCPD_TYPES_H
