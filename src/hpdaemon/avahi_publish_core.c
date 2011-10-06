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

#include "avahi_publish_core.h"
#include "hpd_error.h"

static AvahiServer *server=NULL;

static AvahiThreadedPoll *threaded_poll = NULL;
static char *name = NULL;

static int static_create_services(Service *_service);

int avahi_core_start_server (char* _host_name, char* _domain_name);

//Service *current_service = NULL;

void clean_up_server();

/**
 * Callback function that is called whenever an Entry Group state changes
 *
 * @param s An Avahi Server
 *
 * @param g An Avahi Entry Group
 *
 * @param state The state of the Entry Group
 *
 * @param userdata The Service that is involved in this state change
 *
 * @return void
 *
 */
static void entry_group_callback(AvahiServer *s, AvahiSEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) 
{    
    
    assert(s);
    
    //Service *_called_service = (Service*) avahi_s_entry_group_get_data(g);
	Service *_called_service = (Service*) userdata;

    /* Called whenever the entry group state changes */

    switch (state) {

        case AVAHI_ENTRY_GROUP_ESTABLISHED:

            /* The entry group has been established successfully */
            fprintf(stderr, "Service '%s' successfully established.\n", _called_service->zeroConfName);
            break;

        case AVAHI_ENTRY_GROUP_COLLISION: {
            char *n;

            /* A service name collision happened. Let's pick a new name */
            n = avahi_alternative_service_name(_called_service->ID);
            avahi_free(_called_service->ID);
            _called_service->ID = n;

            fprintf(stderr, "Service name collision, renaming service to '%s'\n", name);

            /* And recreate the services */
            
            avahi_core_create_service(_called_service);
            
            break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_server_errno(s)));

            /* Some kind of failure happened while we were registering our services */
            avahi_threaded_poll_quit(threaded_poll);
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

/**
 * Adds a new service to the Avahi Daemon
 *
 * @param _service The service to add
 *
 * @return void
 */ 
int avahi_core_create_service(Service *_service)
{
    int rc;

    avahi_threaded_poll_lock(threaded_poll);
    rc = static_create_services (_service);
    avahi_threaded_poll_unlock(threaded_poll);

    return rc;
}

/**
 * Adds a new service to the Avahi Daemon after having held the threaded poll
 *
 * @param _service The service to add
 *
 * @return void
 */ 
static int static_create_services(Service *_service) {
    char *r;
    int ret;
    AvahiSEntryGroup *_group = NULL;
    assert(server);
    
    /* Create an entry group */
	if (!(_group = avahi_s_entry_group_new(server, entry_group_callback, _service))) 
	{
            fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_server_errno(server)));
            goto fail;
    }
    
    //avahi_s_entry_group_set_data(_group, _service);

    /* Setting TXT data */
    r = (char*)malloc((strlen("URI=")+strlen(_service->value_url)+1)*sizeof(char));
    if( r == NULL )
	return HPD_E_MALLOC_ERROR;
    strcpy(r, "URI=");
    strcat(r, _service->value_url);
	

    /* Add the service for service_type */
    if ((ret = avahi_server_add_service(server, _group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, _service->zeroConfName, _service->DNS_SD_type, avahi_server_get_domain_name(server), NULL, 8888, r, NULL)) < 0) {
        fprintf(stderr, "Failed to add %s service with type %s: %s\n", _service->zeroConfName, _service->DNS_SD_type, avahi_strerror(ret));
        goto fail;
    }

    /* Tell the server to register the service */
    if ((ret = avahi_s_entry_group_commit(_group)) < 0) {
        fprintf(stderr, "Failed to commit entry_group: %s\n", avahi_strerror(ret));
        goto fail;
    }

    free(r);

    return HPD_E_SUCCESS;

fail:
    avahi_threaded_poll_quit(threaded_poll);
    return HPD_E_AVAHI_CORE_ERROR;
}

/**
 * Callback function that is called whenever the server state changes
 *
 * @param s An Avahi Server
 *
 * @param state The state of the Server
 *
 * @param userdata Unused
 *
 * @return void
 *
 */
static void server_callback(AvahiServer *s, AvahiServerState state, AVAHI_GCC_UNUSED void * userdata) {
    assert(s);

   

    switch (state) {

        case AVAHI_SERVER_RUNNING:
            /* The serve has startup successfully and registered its host
             * name on the network */

            printf("AvahiSever %s sarted succesfully.\n", avahi_server_get_domain_name(server));

            break;

        case AVAHI_SERVER_COLLISION: {
            char *n;
            int r;
            
            /* A host name collision happened. Let's pick a new name for the server */
            n = avahi_alternative_host_name(avahi_server_get_host_name(s));
            fprintf(stderr, "Host name collision, retrying with '%s'\n", n);
            r = avahi_server_set_host_name(s, n);
            avahi_free(n);

            if (r < 0) {
                fprintf(stderr, "Failed to set new host name: %s\n", avahi_strerror(r));

                avahi_threaded_poll_quit(threaded_poll);
                return;
            }

        }

            /* Fall through */

        case AVAHI_SERVER_REGISTERING:
            
	        /* Let's drop our registered services. When the server is back
             * in AVAHI_SERVER_RUNNING state we will register them
             * again with the new host name. */
            reset_all_services ();

            break;

        case AVAHI_SERVER_FAILURE:
            /* Terminate on failure */

            fprintf(stderr, "Server failure: %s\n", avahi_strerror(avahi_server_errno(s)));
            avahi_threaded_poll_quit(threaded_poll);
            break;

        case AVAHI_SERVER_INVALID:
            ;
    }
}


/**
 * Starts the Avahi server
 *
 * @param _host_name The Host Name
 *
 * @param _domain_name The Domain Name
 *
 * @return void
 */ 
int avahi_core_start (char *_host_name, char *_domain_name)
{
    return avahi_core_start_server (_host_name, _domain_name);
}

/**
 * Starts the Avahi server
 *
 * @param _host_name The Host Name
 *
 * @param _domain_name The Domain Name
 *
 * @return ????
 */
int avahi_core_start_server(char* _host_name, char* _domain_name){
    AvahiServerConfig config;
    server = NULL;
    int error;
    int ret = 1;

    /* Initialize the pseudo-RNG */
    srand(time(NULL));

    /* Allocate main loop object */
    if (!(threaded_poll = avahi_threaded_poll_new())) {
        fprintf(stderr, "Failed to create simple poll object.\n");
        goto fail;
    }

    /* Let's set the host name for this server. */
    avahi_server_config_init(&config);
    config.host_name = avahi_strdup(_host_name);
    config.domain_name = avahi_strdup(_domain_name);
    config.publish_workstation = 0;

    /* Allocate a new server */
    server = avahi_server_new(avahi_threaded_poll_get(threaded_poll), &config, server_callback, NULL, &error);

    /* Free the configuration data */
    avahi_server_config_free(&config);

    /* Check wether creating the server object succeeded */
    if (!server) {
        fprintf(stderr, "Failed to create server: %s\n", avahi_strerror(error));
        goto fail;
    }

    /* Run the main loop */
    avahi_threaded_poll_start(threaded_poll);
    
    return HPD_E_SUCCESS;

fail:

    clean_up_server();

    return HPD_E_AVAHI_CORE_ERROR;
}

/**
 * Cleans up the Avahi server
 *
 * @return void
 */

void clean_up_server()
{
    if (server)
        avahi_server_free(server);

    if (threaded_poll)
        avahi_threaded_poll_free(threaded_poll);
}

/**
 * Removes a Service
 *
 * @param _service Service to remove
 *
 * @return 0 if successful -1 if failed
 */

int avahi_core_remove_service(Service *_service)
{
  
   AvahiSEntryGroup *_group;
   int rc;
   
    avahi_threaded_poll_lock(threaded_poll);
    
    rc = avahi_server_get_group_of_service(server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
                                                                    _service->zeroConfName, _service->DNS_SD_type, 
                                                                    avahi_server_get_domain_name(server), &_group);
    if( rc != AVAHI_OK )
    {
            printf("avahi_server_get_group_of_service failed : %d\n", rc);
            avahi_threaded_poll_unlock(threaded_poll);
            return HPD_E_AVAHI_SERVICE_NOT_FOUND;
    }                                                                    
    
    avahi_s_entry_group_free(_group);

    avahi_threaded_poll_unlock(threaded_poll);
    
    return HPD_E_SUCCESS;
}


/*
 * NOT IMPLEMENTED YET
 */

void reset_all_services()
{
/*    ServiceEntry *iterator = service_entry_list;
    
    while(iterator!=NULL)
    {
        printf("RESET\n");
        avahi_s_entry_group_reset(iterator->entryGroup);
        iterator = iterator->next_service_entry;
    }
*/
    //free_service_entry_list (service_entry_list);
}

/**
 * Closes the Avahi server
 *
 * @return void
 */
void avahi_core_quit()
{
    avahi_threaded_poll_stop(threaded_poll);
    
    clean_up_server();
}

