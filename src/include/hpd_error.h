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

/**
 * @file hpd_error.h
 * @brief  Methods for managing Errors
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */


#ifndef HPD_ERROR_H
#define HPD_ERROR_H

#include "hpd_log.h"
#include <errno.h>

/** 		HPD error codes			*/
#define HPD_WAIT_FOR_TIMEOUT				3

#define HPD_NO						2

#define HPD_YES						1

#define HPD_E_SUCCESS		 			0	/**<No Error*/

#define HPD_E_NULL_POINTER				-1	/**< A NULL pointer have been given to HPD */
	
#define HPD_E_CFG_NOT_INIT				-2	/**< The configuration has not been initialized */

#define HPD_E_CFG_ALREADY_INIT 				-3	/**< Attempt to initialize the configuration that is already initialized */
	
#define HPD_E_CFG_FILE_ERROR				-4	/**< Error when trying to load the configuration from file */

#define HPD_E_MALLOC_ERROR 				-5	/**< A malloc failed */

#define HPD_E_BAD_PARAMETER				-6	/**< A bad paramter has been given to HPD */

#define HPD_E_MHD_ERROR					-7	/**< Error happened in the MHD library */

#define HPD_E_SERVER_NULL				-8	/**< Trying to access a server that has not been launched */

#define HPD_E_NO_HTTP					-9	/**< Trying to use HTTP without the HTTP feature enabled */

#define HPD_E_NO_HTTPS					-10	/**< Trying to use HTTPS withotu the HTTPS feature enabled */

#define HPD_E_SERVICE_ALREADY_REGISTER			-11	/**< Trying to register a service that is already registered */

#define HPD_E_SERVICE_NOT_REGISTER			-12	/**< Trying to unregister a service that is not registered */

#define HPD_E_SERVICE_IN_USE				-13	/**< Trying to delete a service that is still in use */

#define HPD_E_SERVICE_ALREADY_IN_LIST			-14	/**< Trying to add a service to a device's list that is already in it */

#define HPD_E_SERVICE_NOT_IN_LIST			-15	/**< Trying to remove a service form a device's list that is not in it */

#define HPD_E_PARAMETER_ALREADY_IN_LIST			-16	/**< Trying to add a parameter to a Service that already has it */

#define HPD_E_PARAMETER_NOT_IN_LIST			-17	/**< Trying to remove a parameter from a Service that does not have it */

#define HPD_E_XML_ERROR					-18	/**< XML error happened */

#define HPD_E_LIST_ERROR				-19	/**< Linked list error */

#define HPD_E_SSL_CERT_ERROR				-20	/**< Error when loading the certificates */

#define HPD_E_AVAHI_CLIENT_ERROR			-21	/**< Error with Avahi client */

#define HPD_E_AVAHI_CORE_ERROR				-22	/**< Error with Avahi core */

#define HPD_E_AVAHI_SERVICE_NOT_FOUND			-23	/**< A service was not found in the Avahi client or server */

#define HPD_E_CLIENT_CERT_ERROR				-24	/**< Error when trying to verify a client certificate */

#define HPD_E_LOG_ALREADY_EXIST				-25	/**< Error when trying to initialize the log */

#define HPD_E_LOG_FAILED_INIT				-26	/**< Error when trying to initialize the log */

#define HPD_E_LOG_NOT_INIT				-27	/**< Error when trying to use the log */

#define HPD_E_FAILED_TO_OPEN_FILE			-28	/**< Error when opening a file fails */

#define HPD_E_LOG_FAILED				-29	/**< Error when logging fails */

#define HPD_E_LOG_EVENTS_OUT_OF_BOUNDS			-30	/**<  */

#define HPD_E_EVENT_SUBSCRIBER_DOESNT_EXIST		-31	/**<  */

#define HPD_E_FAILED_XML_SUBSCRIPTION			-32	/**<  */

#define HPD_E_MISSING_OPTION				-33	/**< */

#define HPD_E_SUBSCRIBERS_HEAD_EMPTY			-34	/**< */

#define HPD_E_DEVICE_ALREADY_IN_XML			-35

#define HPD_E_SERVICE_ALREADY_IN_XML			-36

#define HPD_E_IMPOSSIBLE_TO_RETRIEVE_DEVICE_XML_NODE	-37

#define HPD_E_IMPOSSIBLE_TO_RETRIEVE_SERVICE_XML_NODE	-38

#define HPD_E_QUEUE_IS_EMPTY				-39

#define HPD_E_EVENT_IS_NULL				-40

#define HPD_E_EVENT_QUEUE_IS_NULL				-41

#define HPD_E_SERVICE_ID_IS_NULL				-42

#define HPD_E_EVENT_QUEUE_OR_EVENT_IS_NULL				-43

#define HPD_E_EVENT_QUEUE_OR_URL_IS_NULL				-44

#define HPD_E_EVENT_QUEUE_OR_SERVICE_ID_IS_NULL				-45

#define HPD_E_SERVICE_ID_NOT_IN_EVENT_QUEUE				-46

#define HPD_E_NOT_HPD_BOOLEAN				-47

#define HPD_E_SERVICE_IS_NULL				-48

#define HPD_E_ERROR_QUEUING_EVENT				-49

#define HPD_E_LOG_DATA_IS_NULL				-50

#define HPD_E_CONNECTION_ID_IS_NULL				-51





/*
char *hpd_error_code_to_string(int error_code)
{
	switch(error_code)
	{
		case	HPD_NO:
			return "No";
		case	HPD_YES:
			return "Yes";
		case	HPD_E_SUCCESS:
			return "Success";
		case	HPD_E_NULL_POINTER:
			return "A NULL pointer have been given to HPD";
		case	HPD_E_CFG_NOT_INIT:
			return "The configuration has not been initialized";
		case	HPD_E_CFG_ALREADY_INIT:
			return "Attempt to initialize the configuration that is already initialized";
		case	HPD_E_CFG_FILE_ERROR:
			return "Error when trying to load the configuration from file";
		case	HPD_E_MALLOC_ERROR:
			return "A malloc failed";
		case	HPD_E_BAD_PARAMETER:
			return "A bad paramter has been given to HPD";
		case	HPD_E_MHD_ERROR:
			return "Error happened in the MHD library";
		case	HPD_E_SERVER_NULL:
			return "Trying to access a server that has not been launched";
		case	HPD_E_NO_HTTP:
			return "Trying to use HTTP without the HTTP feature enabled";
		case	HPD_E_NO_HTTPS:
			return "Trying to use HTTPS withotu the HTTPS feature enabled";
		case	HPD_E_SERVICE_ALREADY_REGISTER:
			return "Trying to register a service that is already registered";
		case	HPD_E_SERVICE_NOT_REGISTER:
			return "Trying to unregister a service that is not registered";
		case	HPD_E_SERVICE_IN_USE:
			return "Trying to delete a service that is still in use";
		case	HPD_E_SERVICE_ALREADY_IN_LIST:
			return "Trying to add a service to a device's list that is already in it";
		case	HPD_E_SERVICE_NOT_IN_LIST:
			return "Trying to remove a service form a device's list that is not in it";
		case	HPD_E_PARAMETER_ALREADY_IN_LIST:
			return "Trying to add a parameter to a Service that already has it";
		case	HPD_E_PARAMETER_NOT_IN_LIST:
			return "Trying to remove a parameter from a Service that does not have it";
		case	HPD_E_XML_ERROR:
			return "XML error happened";
		case	HPD_E_LIST_ERROR:
			return "Linked list error";
		case	HPD_E_SSL_CERT_ERROR:
			return "Error when loading the certificates";
		case	HPD_E_AVAHI_CLIENT_ERROR:
			return "Error with Avahi client";
		case	HPD_E_AVAHI_CORE_ERROR:
			return "Error with Avahi core";
		case	HPD_E_AVAHI_SERVICE_NOT_FOUND:
			return "A service was not found in the Avahi client or server";
		case	HPD_E_CLIENT_CERT_ERROR:
			return "Error when trying to verify a client certificate";
		case	HPD_E_LOG_ALREADY_EXIST:
			return "Error when trying to initialize the log it already exists";
		case	HPD_E_LOG_FAILED_INIT:
			return "Error when trying to initialize the log";
		case	HPD_E_LOG_NOT_INIT:
			return "Error when trying to use the log it is not initialized";
		case	HPD_E_FAILED_TO_OPEN_FILE:
			return "Error while loading a file";
		case	HPD_E_LOG_FAILED:
			return "Logging failed";
		case	HPD_E_LOG_EVENTS_OUT_OF_BOUNDS:
			return "";
		case	HPD_E_EVENT_SUBSCRIBER_DOESNT_EXIST:
			return "";
		case	HPD_E_FAILED_XML_SUBSCRIPTION:
			return "";
		case	HPD_E_MISSING_OPTION:
			return "";
		case	HPD_E_SUBSCRIBERS_HEAD_EMPTY:
			return "";
		case	HPD_E_DEVICE_ALREADY_IN_XML:
			return "";
		case	HPD_E_SERVICE_ALREADY_IN_XML:
			return "";
		case	HPD_E_IMPOSSIBLE_TO_RETRIEVE_DEVICE_XML_NODE:
			return "";
		case	HPD_E_IMPOSSIBLE_TO_RETRIEVE_SERVICE_XML_NODE:	
			return "";	
	}
}*/

#endif
