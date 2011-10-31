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
 * @file hpd_log.c
 * @brief  Methods for managing the Log
 * @author Thibaut Le Guilly
 * @author Regis Louge
 */

#include "hpd_log.h"
#include "hpd_error.h"

#define HPD_SEND_LOG_EVENTS 	 1
#define HPD_DONT_SEND_LOG_EVENTS 0

HPD_Log *hpd_log = NULL;

/**
 * Creates the HPD Log
 *
 * @param max_log_size the maximum size of the file
 *
 * @param log_level The Level of logging desired (HPD_LOG_NO_LOG, HPD_LOG_ONLY_REQUESTS, HPD_LOG_ALL);
 *
 * @return Returns the structure HPD Log
 */
HPD_Log *
create_hpd_log( int max_log_size, enum HPD_Loglevel log_level)
{
	HPD_Log *return_log = (HPD_Log*)malloc(sizeof(HPD_Log));
	return_log->max_log_size = max_log_size;
	return_log->send_events = HPD_DONT_SEND_LOG_EVENTS;

	return_log->log_file = fopen(LOG_FILE_NAME, "rb+");
	if(return_log->log_file == NULL)
	{
		return_log->log_file = fopen(LOG_FILE_NAME, "w");
		fprintf(return_log->log_file,"#Version: 1.0\n");
		fprintf(return_log->log_file,"#Date: %s\n",get_date());
		fprintf(return_log->log_file,"#Fields: date time x-error-string c-ip cs-method cs-uri-stem cs-uri-query\n");
		fgetpos(return_log->log_file,&return_log->initial_pos);
		fgetpos(return_log->log_file,&return_log->pos);
		fclose(return_log->log_file);
	}
	else
	{
		char buffer[256];
		int error = 0;
		fgets(buffer, 256, return_log->log_file);
		fgets(buffer, 256, return_log->log_file);
		fgets(buffer, 256, return_log->log_file);
		fgetpos(return_log->log_file,&return_log->initial_pos);
		fgetpos(return_log->log_file,&return_log->pos);
		fclose(return_log->log_file);
	}

	if(log_level == 0) return_log->log_level = HPD_LOG_NO_LOG;
	else return_log->log_level = log_level;

	return_log-> mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(return_log-> mutex, NULL);

	return return_log;
}

/**
 * Destroys the HPD Log
 *
 * @return void
 */
void 
destroy_hpd_log()
{
	if(hpd_log)
	{
		if(hpd_log->mutex)
		{
			pthread_mutex_destroy(hpd_log->mutex);
			free(hpd_log->mutex);
		}
		free(hpd_log);
	}
}

/**
 * Get current date
 *
 * @return Returns the current date
 */
char *
get_date()
{
	time_t ltime;
	ltime = time(NULL);
	const struct tm *timeptr = localtime(&ltime);

	static char result[11];

	sprintf(result, "%d-%.2d-%.2d",
	        1900 + timeptr->tm_year,
	        timeptr->tm_mon,
	        timeptr->tm_mday);
	return result;
}

/**
 * Method that initiates the Log
 *
 * @param max_log_size the maximum size of the file
 *
 * @param log_level The Level of logging desired (HPD_LOG_NO_LOG, HPD_LOG_ONLY_REQUESTS, HPD_LOG_ALL);
 *
 * @return HPD_E_SUCCESS if successful, HPD_E_LOG_FAILED_INIT if failed
 */
int 
init_hpd_log( int max_log_size, enum HPD_Loglevel log_level)
{
	if(hpd_log != NULL)
	{
		return HPD_E_LOG_ALREADY_EXIST;
	}
	hpd_log = create_hpd_log( max_log_size, log_level);
	if(hpd_log) return HPD_E_SUCCESS;
	else return HPD_E_LOG_FAILED_INIT;
}

/**
 * Method that is used to write in the log
 *
 * @param log_level the Log level
 *
 * @param error_string the string of the error if the log message is an error log message
 *
 * @param client_IP the IP of the client
 *
 * @param method the method received
 *
 * @param uri_stem the URL without the ?
 *
 * @param uri_query the URL after the ?
 *
 * @return HPD_E_SUCCESS if successful HPD_E_LOG_NOT_INIT if the log is not initialized and HPD_E_FAILED_TO_OPEN_FILE if the file can not be opened, or HPD_E_LOG_FAILED if failed
 */
int 
Log (enum HPD_Loglevel log_level, 
     char* error_string, 
     char* client_IP, 
     char *method, 
     char *uri_stem, 
     char *uri_query)
{
	int ret;
	if (hpd_log == NULL) 
	{
		printf("Log must be initialized before Logging\n");
		return HPD_E_LOG_NOT_INIT;
	}
	pthread_mutex_lock(hpd_log->mutex);

	if(hpd_log->log_level == HPD_LOG_NO_LOG)
	{
		pthread_mutex_unlock(hpd_log->mutex);
		return HPD_E_SUCCESS;
	}
	else
		hpd_log->log_file = fopen(LOG_FILE_NAME, "rb+");

	if (hpd_log->log_file == NULL) 
	{
		pthread_mutex_unlock(hpd_log->mutex);
		return HPD_E_FAILED_TO_OPEN_FILE;
	}
	else
	{
		if(log_level <= hpd_log->log_level)
		{
			char *log_message = get_allocated_log_message(	error_string, 
			                                              client_IP, 
			                                              method, 
			                                              uri_stem, 
			                                              uri_query);
			set_position_for_new_message(sizeof(char)*strlen(log_message));
			fprintf(hpd_log->log_file, "%s\n", log_message);
			fgetpos(hpd_log->log_file,&hpd_log->pos);
			fclose(hpd_log->log_file);
			pthread_mutex_unlock(hpd_log->mutex);
			ret = send_log_event(log_message);
			free(log_message);

			return HPD_E_SUCCESS;		    
		}
	}
	pthread_mutex_unlock(hpd_log->mutex);
	return HPD_E_LOG_FAILED;
}

/**
 * Sets the position for the new log message, if it reaches the end of the document go back to the beginning
 *
 * @param size_of_log_message the size of the desired log message
 *
 * @return void
 */
void 
set_position_for_new_message(size_t size_of_log_message)
{
	/*Set the position to the actual position*/
	fsetpos(hpd_log->log_file,&hpd_log->pos);
	/*If the positon + the size of the new message is higher than the maximum size of the log*/
	if(ftell(hpd_log->log_file) + size_of_log_message > hpd_log-> max_log_size)
	{
		/*Set the new position to the initial position*/
		fsetpos(hpd_log->log_file,&hpd_log->initial_pos);
		fgetpos(hpd_log->log_file,&hpd_log->pos);
	}
}

/**
 * Gets a message string based on the different log message parts
 *
 * @param error_string the string of the error if the log message is an error log message
 *
 * @param client_IP the IP of the client
 *
 * @param method the method received
 *
 * @param uri_stem the URL without the ?
 *
 * @param uri_query the URL after the ?
 *
 * @return The created message
 */
char *
get_allocated_log_message(char *error_string, 
                          char *client_IP, 
                          char *method, 
                          char *uri_stem, 
                          char *uri_query)
{
	char* log_message;
	char new_time[9];
	char date[11];
	size_t message_size = 0;
	time_t ltime;
	ltime = time(NULL);
	const struct tm *timeptr = localtime(&ltime);

	/*	Fomat of message :
	 date time x-error-string c-ip cs-method cs-uri-stem cs-uri-query\0\n 
	 1    2              3    4         5           6             7 8   
	 */

	sprintf(date, "%d-%.2d-%.2d", 	1900 + timeptr->tm_year,
	        timeptr->tm_mon,
	        timeptr->tm_mday);

	sprintf(new_time, "%.2d:%.2d:%.2d",	timeptr->tm_hour,
	        timeptr->tm_min,
	        timeptr->tm_sec);

	message_size = 	sizeof(char)*11 +
		sizeof(char)*9 +
		(error_string != NULL ? sizeof(char)*strlen(error_string) : sizeof(char)) +
		(client_IP != NULL ? sizeof(char)*strlen(client_IP) : sizeof(char)) +
		(method != NULL ? sizeof(char)*strlen(method) : sizeof(char)) +
		(uri_stem != NULL ? sizeof(char)*strlen(uri_stem) : sizeof(char)) +
		(uri_query != NULL ? sizeof(char)*strlen(uri_query) : sizeof(char)) +
		sizeof(char)*7;

	log_message = malloc(message_size);

	sprintf(log_message, "%s %s %s %s %s %s %s",date,
	        new_time,
	        (error_string != NULL ? error_string : "-"),
	        (client_IP != NULL ? client_IP : "-"),
	        (method != NULL ? method : "-"),
	        (uri_stem != NULL ? uri_stem : "-"),
	        (uri_query != NULL ? uri_query : "-"));

	return log_message;
}
/**
 * Method that is used to write errors in log
 *
 * @param error_code The error_code
 *
 * @param message The message to write in the Log
 *
 * @param ... The different arguments of the message
 *
 * @return HPD_E_SUCCESS if successful HPD_E_LOG_NOT_INIT if the log is not initialized and HPD_E_FAILED_TO_OPEN_FILE if the file can not be opened, or HPD_E_LOG_FAILED if failed
 */
int 
LogError (int error_code, const char *message,...)
{
	char buffer[256];
	if(error_code!=HPD_NO)
	{
		sprintf(buffer,hpd_error_code_to_string(error_code));
	}
	else
	{
		va_list arglist;
		va_start(arglist, message);
		vsprintf(buffer,message,arglist);
		va_end(arglist);
	}


	sprintf(stderr, "error: %s\n", buffer);

	return Log(1,buffer, NULL, NULL, NULL, NULL);
}

/**
 * Method that is used to change the Log level
 *
 * @param log_level The Level of logging desired (HPD_LOG_NO_LOG, HPD_LOG_ONLY_REQUESTS, HPD_LOG_ALL);
 *
 * @return HPD_E_SUCCESS if successful HPD_E_LOG_NOT_INIT if the log is not initialized
 */
int 
change_hpd_log_level (enum HPD_Loglevel log_level)
{
	if(hpd_log == NULL)
		return HPD_E_LOG_NOT_INIT;
	else
	{
		pthread_mutex_lock(hpd_log->mutex);
		hpd_log->log_level = log_level;
		pthread_mutex_unlock(hpd_log->mutex);
		LogError(HPD_NO,"New Log Level : %d/%d",log_level,HPD_LOG_ALL);
	}
	return HPD_E_SUCCESS;
}

/**
 * Method that is used to set the sending of log events to true or false
 *
 * @param send_events HPD_SEND_LOG_EVENTS or HPD_DONT_SEND_LOG_EVENTS
 *
 * @return HPD_E_SUCCESS if successful HPD_E_LOG_EVENTS_OUT_OF_BOUNDS if the not HPD_SEND_LOG_EVENTS or HPD_DONT_SEND_LOG_EVENTS
 */
int 
set_event_sending(int send_events)
{
	if(send_events == HPD_SEND_LOG_EVENTS || send_events == HPD_DONT_SEND_LOG_EVENTS)
	{
		pthread_mutex_lock(hpd_log->mutex);
		hpd_log->send_events = send_events;
		pthread_mutex_unlock(hpd_log->mutex);
		return HPD_E_SUCCESS;
	}
	else
		return HPD_E_LOG_EVENTS_OUT_OF_BOUNDS;
}

char *
hpd_error_code_to_string(int error_code)
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
			return "Trying to use HTTPS without the HTTPS feature enabled";
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
}


