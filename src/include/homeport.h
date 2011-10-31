/*Copyright 2011 Aalborg University. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed*/

/** @mainpage The HomePort Project
 *
 * @author Thibaut Le Guilly
 * @author Regis Louge
 *
 * @section intro Introduction 
 * The project is about establishing possibilities for intelligent control of the energy consumption in private homes through the development of a prototype for a so called ’home port’. This port establish access to coordinated control of the different subsystems in a private home for control and surveillance of energy consumption etc. Key concepts in the project are coexistence of different technologies controlled through a common service layer in a private house. The purpose of this coexistence is to create interoperability between the different technologies such that energy consumption can be lowered and consequently lowering the pollution of the environment. More info at http://www.energybox.dk/
 *
 * @section installation Installation 
 * In order to install libhpd, some dependencies are necessary:
	- libuuid
		- $ sudo apt-get install uuid-dev

	- minixml 
		- Download the latest version at http://www.minixml.org/
		- Extract
		- $ cd mxml-x.x
		  $ ./configure --enable-shared --prefix=/usr
		  $ make
		  $ sudo make install 

	- libmicrohttpd 
		- Download the latest version at http://www.gnu.org/s/libmicrohttpd/
		- Extract
		- For HTTPS feature enabled, dependencies needed : libgnutls and libgcrypt 
		- $ cd libmicrohttpd-x.x.xx
		  $ ./configure --prefix=/usr
		  $ make
		  $ sudo make install

	- (Optional) libgnutls
		- $ sudo apt-get install libgnutls-dev

	- (Optional) libgcrypt
		- $ sudo apt-get install libgcrypt11-dev

	- avahi-client or avahi-core 
		- Download the latest version at http://avahi.org/
		- Extract
		- $./configure --prefix=/usr \
 *            	  $	--sysconfdir=/etc \
 *            	  $	--localstatedir=/var \
 *            	  $	--with-distro=lfs \
 *         	  $	--disable-qt3 \
 *         	  $	--disable-qt4 \
 *         	  $	--disable-gtk \
 *		  $	--disable-gtk3 \
 *           	  $	--disable-dbus \ (necessary for avahi-client optional for avahi-core)
 *           	  $	--disable-libdaemon \ (necessary for avahi-client optional for avahi-core)
 *            	  $	--disable-python \
 *           	  $	--disable-mono \
 *           	  $	--disable-monodoc \
 *		  $	--disable-gdbm \
 *		  $	--disable-glib \
 *		  $	--disable-xmltoman 
 *		  $ make
 *		  $ sudo make install

In order to use avahi-core instead of avahi-client, make sur to use '--disable-hpd-avahi-client' flag

Then
	- $ cd HPDvx.x
	- $ autoreconf -i
	- $ ./configure with the corresponding flag(s)
	- $ make
	- $ sudo make install

To use the HomePort Daemon library with your own application, make sure that the <hpdaemon/homeport.h> in the include path when compiling.
 *
 * @section examples Examples 
 * In order to use the Phidget Example you will need the library phidget21 when configuring libhpd :
	- sudo apt-get install libusb-dev
	- Download the latest version at http://www.phidgets.com/
	- Extract 
	- $ cd libphidget-x.x.x.xxxxxxxx
	  $ ./configure --prefix=/usr
	  $ make
	  $ sudo make install 
 */

/**
 * @file homeport.h
 * @brief  Methods for managing HomePort Daemon
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#ifndef HOMEPORT_H
#define HOMEPORT_H



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdarg.h>

#include "hpd_services.h"
#include "hpd_configure.h"

enum HPD_FLAG
{
	HPD_NO_FLAG = 0,

	HPD_USE_OPTION = 1,

	HPD_USE_CFG_FILE = 2,

	HPD_USE_DEFAULT = 4
};

enum HPD_OPTION
{
	/** No more options / last option
	*/
	HPD_OPTION_END = 0,

	HPD_OPTION_HTTP = 1,

	HPD_OPTION_HTTPS = 2,

	HPD_OPTION_LOG = 3,

	HPD_OPTION_CFG_PATH = 4

};

int HPD_start( unsigned int option, char *hostname, ... );
int HPD_stop();
int HPD_register_service( Service *service_to_register );
int HPD_unregister_service( Service *service_to_unregister );
int HPD_register_device_services( Device *device_to_register );
int HPD_unregister_device_services( Device *device_to_unregister );
int HPD_send_event_of_value_change ( Service *service_changed, char *updated_value );

Service* HPD_get_service( char *device_type, char *device_ID, char *service_type, char *service_ID );
Device* HPD_get_device( char *device_type, char *device_ID );

#endif
