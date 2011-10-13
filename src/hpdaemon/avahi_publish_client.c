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

#include "avahi_publish_client.h"
#include "hpd_error.h"
#include "hpd_configure.h"

static AvahiThreadedPoll *threaded_poll = NULL;

AvahiClient *client = NULL;

EntryGroupElement *entry_group_head = NULL;

static int static_create_services(Service *_service);

void clean_up_client ();

/**
 * Callback function that is called whenever an Entry Group state changes
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
static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) 
{

    /* Called whenever the entry group state changes */

    switch (state) 
	{
        case AVAHI_ENTRY_GROUP_ESTABLISHED :
            /* The entry group has been established successfully */
            //fprintf(stderr, "Service '%s' successfully established.\n", name);
            break;

        case AVAHI_ENTRY_GROUP_COLLISION : 
		{
            //char *n;

            /* A service name collision with a remote service
             * happened. Let's pick a new name */
            /*n = avahi_alternative_service_name(name);
            avahi_free(name);
            name = n;

            fprintf(stderr, "Service name collision, renaming service to '%s'\n", name);*/

            /* And recreate the services */
            //create_services(avahi_entry_group_get_client(g));
            break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            //fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));

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
int avahi_client_create_service (Service *_service)
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
    //char *n, r[128];
	char *r;
	int ret;
	AvahiEntryGroup *_group = NULL;
   	assert(client);
	int _port;

	if( _service->device->secure_device == HPD_NON_SECURE_DEVICE )
#if HPD_HTTP
		_port = hpd_daemon->http_port;
#else
		return HPD_E_NO_HTTP;
#endif
	else
#if HPD_HTTPS
		_port = hpd_daemon->https_port;
#else
		return HPD_E_NO_HTTPS;
#endif

	if (!(_group = avahi_entry_group_new(client, entry_group_callback, _service))) 
	{
		fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(client)));
		goto fail;
	}

	/* Setting TXT data */
    	r = (char*)malloc((strlen("URI=")+strlen(_service->value_url)+1)*sizeof(char));
	if( r == NULL )
		return HPD_E_MALLOC_ERROR;

	strcpy(r, "URI=");
    strcat(r, _service->value_url);


	/* Add the service to the entry group */
	if ((ret = avahi_entry_group_add_service(_group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, _service->zeroConfName, _service->DNS_SD_type, NULL, NULL, _port, r, NULL)) < 0) {

		fprintf(stderr, "Failed to add %s service: %s\n", _service->ID, avahi_strerror(ret));
		goto fail;
	}

	/* Tell the server to register the service */
	if ((ret = avahi_entry_group_commit(_group)) < 0) {
		fprintf(stderr, "Failed to commit entry group: %s\n", avahi_strerror(ret));
		goto fail;
	}

	free(r);

	EntryGroupElement *_new_entry_group_element = (EntryGroupElement*)malloc(sizeof(EntryGroupElement));
	_new_entry_group_element->avahiEntryGroup = _group;
	_new_entry_group_element->service = _service;
	_new_entry_group_element->next = NULL;

	LL_APPEND(entry_group_head, _new_entry_group_element);

    return HPD_E_SUCCESS;

fail:
    avahi_threaded_poll_quit(threaded_poll);

    return HPD_E_AVAHI_CLIENT_ERROR;
}

/**
 * Callback function that is called whenever the client or server state changes
 *
 * @param c An Avahi Client
 *
 * @param state The state of the Client
 *
 * @param userdata Unused
 *
 * @return void
 *
 */
static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) 
{
    assert(c);

    /* Called whenever the client or server state changes */

    switch (state) {
        case AVAHI_CLIENT_S_RUNNING:

            /* The server has startup successfully and registered its host
             * name on the network, so it's time to create our services */
            printf("AVAHI client running\n");
            break;

        case AVAHI_CLIENT_FAILURE:

            fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(c)));
            avahi_threaded_poll_quit(threaded_poll);

            break;

        case AVAHI_CLIENT_S_COLLISION:

            /* Let's drop our registered services. When the server is back
             * in AVAHI_SERVER_RUNNING state we will register them
             * again with the new host name. */

        case AVAHI_CLIENT_S_REGISTERING:

            /* The server records are now being established. This
             * might be caused by a host name change. We need to wait
             * for our own records to register until the host name is
             * properly esatblished. */

            /*if (group)
                avahi_entry_group_reset(group);*/

            break;

        case AVAHI_CLIENT_CONNECTING:
            ;
    }
}

/**
 * Removes a service to the Avahi Daemon
 *
 * @param _service The service to add
 *
 * @return 0 if successful -1 if failed
 */ 
int avahi_client_remove_service(Service *_service)
{

	EntryGroupElement *_tmp, *_iterator = NULL;
	AvahiEntryGroup *_group = NULL;
	int rc;

	LL_FOREACH_SAFE(entry_group_head, _iterator, _tmp)
	{
		if( strcmp( _iterator->service->zeroConfName, _service->zeroConfName ) == 0 )
		{
			_group = _iterator->avahiEntryGroup;
			LL_DELETE(entry_group_head, _iterator);
			free(_iterator);
			_iterator = NULL;
			break;
		}
	}

	if( _group == NULL )
	{
		printf("avahi_remove_service : No matching AvahiEntryGroup found\n");
		return HPD_E_AVAHI_SERVICE_NOT_FOUND;
	}

	avahi_threaded_poll_lock(threaded_poll);                                                                 

	avahi_entry_group_free(_group);

	avahi_threaded_poll_unlock(threaded_poll);

	return HPD_E_SUCCESS;
}


/**
 * Starts the Avahi client
 *
 * @return 0 if successful -1 if failed
 */ 
int avahi_client_start() {
    int error;

    /* Allocate main loop object */
    if (!(threaded_poll = avahi_threaded_poll_new())) {
        fprintf(stderr, "Failed to create simple poll object.\n");
        clean_up_client ();
		return HPD_E_AVAHI_CLIENT_ERROR;
    }

    /* Allocate a new client */
    client = avahi_client_new(avahi_threaded_poll_get(threaded_poll), 0, client_callback, NULL, &error);

    /* Check wether creating the client object succeeded */
    if (!client) {
        fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
        clean_up_client ();
		return HPD_E_AVAHI_CLIENT_ERROR;
    }

    /* Run the main loop */
    avahi_threaded_poll_start(threaded_poll);

    return HPD_E_SUCCESS;
}

/**
 * Frees the allocated memory for the Avahi client
 *
 * @return void
 */ 
void clean_up_client()
{

	EntryGroupElement *_tmp, *_iterator = NULL;

	LL_FOREACH_SAFE(entry_group_head, _iterator, _tmp)
	{
		LL_DELETE(entry_group_head, _iterator);
		free(_iterator);
		_iterator = NULL;
	}

	avahi_threaded_poll_lock(threaded_poll);  
	
	if (client)
		avahi_client_free(client);

	avahi_threaded_poll_unlock(threaded_poll); 

	if (threaded_poll)
		avahi_threaded_poll_free(threaded_poll);
}

/**
 * Stops the Avahi Client
 *
 * @return void
 */ 
void avahi_client_quit ()
{
	clean_up_client();
}
