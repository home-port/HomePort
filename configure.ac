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

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([libhpd], [0.1], [https://github.com/home-port/HomePort])
AM_INIT_AUTOMAKE([libhpd], [0.1])
AC_CONFIG_SRCDIR([src/hpdaemon/homeport.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

# DOXYGEN support

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)

DX_INIT_DOXYGEN([$PACKAGE_NAME],[doxygen.cfg])

# Checks for command line options AVAHI
AC_ARG_ENABLE([hpd-avahi-client],
    [AS_HELP_STRING([--disable-hpd-avahi-client],
        [disable hpd avahi client feature @<:@default: no@:>@])],
    [hpd_avahi_client=${enableval}], [hpd_avahi_client=yes])

# Checks for command line options HTTPS and HTTP
AC_ARG_ENABLE([hpd-https],
    [AS_HELP_STRING([--enable-hpd-https],
        [enable hpd https feature @<:@default: no@:>@])],
    [hpd_https=${enableval}], [hpd_https=no])

AC_ARG_ENABLE([hpd-http],
    [AS_HELP_STRING([--disable-hpd-http],
        [disable hpd http feature @<:@default: yes@:>@])],
    [hpd_http=${enableval}], [hpd_http=yes])

if test "x${hpd_avahi_client}" = xyes; then
    have_avahi_client=no
    AC_SEARCH_LIBS([avahi_client_new], [avahi-client], [have_avahi_client=yes])

    if test "x${have_avahi_client}" = xyes; then
        AC_CHECK_HEADERS([avahi-client/client.h avahi-client/publish.h], [], [have_avahi_client=no])
    fi

    if test "x${have_avahi_client}" = xno; then
        AC_MSG_ERROR([Compiling libhpd requires 'avahi-client'])
    fi
fi

if test "x${hpd_avahi_client}" = xno; then
    have_avahi_core=no
    AC_SEARCH_LIBS([avahi_server_new], [avahi-core], [have_avahi_core=yes])
    if test "x${have_avahi_core}" = xyes; then
        AC_CHECK_HEADERS([avahi-core/core.h avahi-core/publish.h], [], [have_avahi_core=no])
    fi

    if test "x${have_avahi_core}" = xno; then
        AC_MSG_ERROR([Compiling libhpd without avahi-client requires 'avahi-core'])
    fi
fi

if test "x${hpd_avahi_client}" = xyes; then
	AC_DEFINE([AVAHI_CLIENT], 1, [Avahi client enabled])
fi

AM_CONDITIONAL([ENABLE_AVAHI_CLIENT], [test "x$hpd_avahi_client" = xyes]) 

echo "
Enable HTTP : '${hpd_http}'
Enable HTTPS : '${hpd_https}'"

if test "x${hpd_https}" = xno -a "x${hpd_http}" = xno; then
	AC_MSG_ERROR([Cannot disable http and not enable https])
fi

if test "x${hpd_http}" = xyes; then
	AC_DEFINE([HPD_HTTP], 1, [http enabled])
fi

if test "x${hpd_https}" = xyes; then
	AC_CHECK_HEADERS([gnutls/gnutls.h],
            AC_CHECK_LIB([gnutls], [gnutls_priority_set],
	     gnutls=true))
	AC_DEFINE([HPD_HTTPS], 1, [https enabled])
fi

AM_CONDITIONAL([ENABLE_HTTP], [test "x$hpd_http" = xyes])
AM_CONDITIONAL([ENABLE_HTTPS], [test "x$hpd_https" = xyes])

# Checks for libraries.

# Check for headers that are ALWAYS required
AC_CHECK_HEADERS([fcntl.h math.h errno.h limits.h stdio.h locale.h sys/stat.h sys/types.h pthread.h malloc.h sys/socket.h],,AC_MSG_ERROR([Compiling libhpd requires standard UNIX headers files]))

#Check for libphidget for the phidget example
AC_CHECK_HEADERS([phidget21.h],
		[hpd_phidget=yes], [hpd_phidget=no])

AM_CONDITIONAL([ENABLE_PHIDGET], [test "x$hpd_phidget" = xyes] )

# Check for libconfig
PKG_CHECK_MODULES([LIBCONFIG], [libconfig >= 1.4])

# Check for libmicrohttpd
PKG_CHECK_MODULES([LIBMICROHTTPD], [libmicrohttpd])

# Check for libxml
PKG_CHECK_MODULES([MINIXML], [mxml])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([libhpd.pc
		 Makefile
                 src/Makefile
                 src/hpdaemon/Makefile
                 src/include/Makefile
		 src/examples/Makefile])
AC_OUTPUT

echo \
"-------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION}

 Prefix: '${prefix}'.
 Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
 Libraries: '${LIBS}'

 Package features:
   Avahi client ${hpd_avahi_client}

 Now type 'make @<:@<target>@:>@'
   where the optional <target> is:
     all                - build all binaries
     install            - install everything

--------------------------------------------------"
