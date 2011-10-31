#Copyright 2011 Aalborg University. All rights reserved.
#
#Redistribution and use in source and binary forms, with or without modification, are
#permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright notice, this list of
#      conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright notice, this list
#      of conditions and the following disclaimer in the documentation and/or other materials
#      provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
#WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
#FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
#CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
#ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#The views and conclusions contained in the software and documentation are those of the
#authors and should not be interpreted as representing official policies, either expressed


AM_CFLAGS=-I$(top_srcdir)\
	-I$(top_srcdir)/src/include	

noinst_PROGRAMS = hpd-example
hpd_example_SOURCES = hpd_example.c
hpd_example_CFLAGS = $(AM_CFLAGS)
hpd_example_LDADD = $(AM_LDADD) ../hpdaemon/libhpd.la

EXTRA_DIST = hpd.cfg CA_root_cert.pem client-cert.pem HPD-key.key HPD-cert.pem client-browser-cert.p12

if ENABLE_PHIDGET

noinst_PROGRAMS += phidget-example
phidget_example_SOURCES = hpd_phidget_example.c hpd_phidget.h hpd_phidget.c
phidget_example_CFLAGS = $(AM_CFLAGS) -lphidget21 -g
phidget_example_LDADD = $(AM_LDADD) ../hpdaemon/libhpd.la

endif
