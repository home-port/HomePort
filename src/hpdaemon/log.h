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


#ifndef LOG_H
#define LOG_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <arpa/inet.h>


#define LOG_FILE_NAME "hpd_log.log"

enum HPD_Loglevel{
    HPD_LOG_NO_LOG = 0,
    HPD_LOG_ONLY_REQUESTS = 1,
    HPD_LOG_ALL = 2
};

typedef struct HPD_Log HPD_Log;
struct HPD_Log
{
    int max_log_size;/**<The maximum size of the Log File*/
    int send_events;/**<Integer that is used to know if we need to send Log event notifications*/
    FILE *log_file;/**<The actual Log File*/
    fpos_t pos;/**<The current position in the Log File*/
    fpos_t initial_pos;/**<The initial file position*/
    pthread_mutex_t *mutex; /**<A mutex used to access theLog */
    enum HPD_Loglevel log_level;/**<The desired log level*/
};

HPD_Log *create_hpd_log( int _max_log_size, enum HPD_Loglevel _log_level);
char *get_date();
char *get_time ();
char *hpd_error_code_to_string(int error_code);
void destroy_hpd_log();
int init_hpd_log( int _max_log_size, enum HPD_Loglevel _log_level);
int Log (enum HPD_Loglevel _log_level, char* error_string, char* client_IP, char *method, char *uri_stem, char *uri_query);
int LogError (int error_code, const char *message,...);
int change_hpd_log_level (enum HPD_Loglevel _log_level);
int set_event_sending(int _send_events);
int LogHTTPRequest (struct sockaddr *addr, char *method, char *url, char *version);
char *timestamp();

#endif

