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


#include "web_server_api.h"

#define AVAHI 1


static ServiceElement *service_head;/**< List containing all the services handled by the server */
static struct MHD_Daemon *d;/**< MDH daemon for the MHD web server listening for incoming connections */


int done_flag = 0;
char *put_data_temp = NULL;

static ssize_t
file_reader (void *cls, uint64_t pos, char *buf, size_t max)
{
    FILE *file = cls;

    (void)  fseek (file, pos, SEEK_SET);
    return fread (buf, 1, max, file);
}

static void
free_callback (void *cls)
{
    FILE *file = cls;
    fclose (file);
}

/**
 * Add an XML response to the queue of the server
 *
 * @param connection The client connection which will receive the response
 *
 * @return MHD return value, MHD_NO if the response failed to be created, 
 *		   return code of MHD_queue_response otherwise
 */
static int
send_xml (struct MHD_Connection *connection, const xmlChar *xmlbuff)
{

    int ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer (strlen( (char*)xmlbuff),(char *)xmlbuff, MHD_RESPMEM_PERSISTENT);

    if(!response)
    {
        if(xmlbuff)
	   xmlFree(xmlbuff);
        return MHD_NO;
    }

    MHD_add_response_header (response, "Content-Type", "text/xml");
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

/**
 * Add a Not found response to the queue of the server
 *
 * @param connection The client connection which will receive the response
 *
 * @return MHD return value, MHD_NO if the response failed to be created, 
 *		   return code of MHD_queue_response otherwise
 */
static int send_not_found(struct MHD_Connection *connection)
{
    int ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_data(strlen("Not Found"), (void *) "Not Found", MHD_NO, MHD_NO);

    if(!response)
        return MHD_NO;
    ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);

    return ret;
}

/**
 * Free the XML response sent to a client when a request is completed (MHD_RequestCompletedCallback)
 *
 * @param cls Custom value selected at callback registration time (NOT USED)
 *	
 * @param connection The client connection to which the response has been sent
 *
 * @param con_cls Pointer to the XML file to free set in answer_to_connection
 *
 * @param toe Reason for request termination see MHD_OPTION_NOTIFY_COMPLETED (NOT USED)
 *
 * @return 0 if an XML file has been freed, -1 otherwise
 */
int request_completed(void *cls, struct MHD_Connection *connection, 
                      void **con_cls, enum MHD_RequestTerminationCode toe)
{
    if(*con_cls)
    {
        xmlFree((xmlChar*)*con_cls);
        return 0;
    }

    return -1;

}

/**
 * Callback function used to answer clients's connection (MHD_AccessHandlerCallback)
 *
 * @param cls Custom value selected at callback registration time, used to initialize the done flag
 *	
 * @param connection The client connection 
 *
 * @param url The url on which the client made its request
 *
 * @param method The http method with which the client made its request (Only GET and PUT supported)
 *
 * @param version The HTTP version string (i.e. HTTP/1.1)
 *
 * @param upload_data Data beeing uploaded when receiving PUT
 *
 * @param upload_data_size Size of the data beeing uploaded when receiving PUT
 *
 * @param con_cls reference to a pointer, initially set to NULL, that this callback can set to some 
 *        address and that will be preserved by MHD for future calls for this request
 *
 * @return MHD_YES to pursue the request handling, MHD_NO in case of error with 
 *         the request, return value of the send_* functions otherwise
 */
int answer_to_connection (void *cls, struct MHD_Connection *connection, 
                          const char *url, 
                          const char *method, const char *version, 
                          const char *upload_data, 
                          size_t *upload_data_size, void **con_cls)
{
    ServiceElement *_requested_service_element;
    int *done = cls;
    static int aptr;

    if( 0 == strcmp (method, MHD_HTTP_METHOD_GET) )
    {

        if (&aptr != *con_cls)
        {
	   /* do never respond on first call */
	   *con_cls = &aptr;
	   return MHD_YES;
        }
        *con_cls = NULL;
        xmlChar *xmlbuff;

        if(strcmp(url,"/services_css.css") == 0){

	   FILE *file;
	   struct stat buf;
	   struct MHD_Response *response;
	   int ret;
	   if (0 == stat ("services_css.css", &buf))
	       file = fopen ("services_css.css", "rb");
	   else
	       file = NULL;

	   response = MHD_create_response_from_callback (buf.st_size, 32 * 1024,     /* 32k page size */
	                                                 &file_reader,
	                                                 file,
	                                                 &free_callback);
	   if (response == NULL)
	   {
	       fclose (file);
	       return MHD_NO;
	   }
	   ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	   MHD_destroy_response (response);
	   return ret;
        }

        else if(strcmp(url,"/devices") == 0)
        {
	   const char *_get_arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "c");
	   if(_get_arg != NULL)
	   {
	       if(strcmp(_get_arg, "1") == 0)
	       {
		  char* css_add_string = "<?xml-stylesheet type=\"text/css\" href=\"services_css.css\"?>";
		  char string[1000] = "";
		  FILE* originalFile = fopen("services.xml","r");
		  FILE* newFile = fopen("services_with_css.xml","w+t");
		  int i = 0;
		  while(fgets(string,1000,originalFile)!=NULL){
		      i++;
		      if(i==2){
			 fputs(css_add_string,newFile);
		      }
		      fputs(string,newFile);
		  }
		  fclose(originalFile);
		  fclose(newFile);

		  xmlDocPtr doc = xmlParseFile ("services_with_css.xml");
		  int buffersize;

		  xmlDocDumpFormatMemory (doc, &xmlbuff, &buffersize, 1);

		  remove("services_with_css.xml");

		  return send_xml (connection, xmlbuff);

	       }
	   }
	   else{
	       xmlDocPtr doc = xmlParseFile (XML_FILE_NAME);
	       int buffersize;

	       xmlDocDumpFormatMemory (doc, &xmlbuff, &buffersize, 1);

	       return send_xml (connection, xmlbuff);
	   }
        }
        else if( ( _requested_service_element = matching_service (service_head, url) ) !=NULL )
        {
	   pthread_mutex_lock(&_requested_service_element->mutex);
	   const char *_get_arg = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "x");
	   if(_get_arg != NULL)
	   {
	       if(strcmp(_get_arg, "1") == 0)
	       {
		  xmlbuff = extract_service_xml(_requested_service_element->service);

		  *con_cls = xmlbuff;

		  pthread_mutex_unlock(&_requested_service_element->mutex);
		  return send_xml (connection, xmlbuff);
	       }
	   }
	   char *_value = _requested_service_element->service->get_function(_requested_service_element->service);
	   xmlbuff = get_xml_value (_value);
	   *con_cls = xmlbuff;
	   pthread_mutex_unlock(&_requested_service_element->mutex);
	   return send_xml(connection, xmlbuff);
        }
        else
        {
	   return send_not_found(connection);
        }
    }

    else if( 0 == strcmp(method, MHD_HTTP_METHOD_PUT) )
    {  
        if( ( *done ) == 0)
        {
	   if(*upload_data_size == 0)
	   {
	       return MHD_YES; /* not ready yet */
	   }	    
	   *done = 1;
	   /* Add a space for a '/0' in order to clear the end of the XML */
	   put_data_temp = (char*)malloc((*upload_data_size)*sizeof(char)+1);
	   memcpy(put_data_temp, upload_data, *upload_data_size);
	   put_data_temp[*upload_data_size]='\0';
	   *upload_data_size = 0;
	   return MHD_YES;
        }
        else
        {
	   *done = 0;
	   if( ( _requested_service_element = matching_service (service_head, url) ) !=NULL )
	   {
	       pthread_mutex_lock(&_requested_service_element->mutex);
	       if( _requested_service_element->service->put_function != NULL && put_data_temp != NULL)
	       {
		  char* _value = get_value_from_xml_value (put_data_temp);
		  if(_value == NULL)
		      return send_not_found (connection);
		  xmlChar* xmlbuff = get_xml_value ( _requested_service_element->service->put_function(_requested_service_element->service, _value));
		  *con_cls = xmlbuff;
		  put_data_temp = NULL;
		  pthread_mutex_unlock(&_requested_service_element->mutex);
		  return send_xml (connection, xmlbuff);
	       }
	       else
	       {
		  pthread_mutex_unlock(&_requested_service_element->mutex);
		  return send_not_found (connection);
	       }
	   }
	   else
	       return send_not_found (connection);
        }
    }
    return MHD_NO;
}

/**
 * Add a service to the XML file, the server's service list and the AVAHI server
 *
 * @param _service The service to add
 *
 * @return 0 if the service has been successfully added,
 *		  -1 if a similar service already exists in the server,
 *		  -2 if the adding to the XML file failed,
 *        -3 if the service was not added to the list successfully, 
 *		  -4 if the server couldn't add it to its list
 */
int register_service_in_server(Service *_service)
{
    int rc;

    if( is_service_registered( _service ) )
    {
        printf("A similar service is already registered in the server\n");
        return -1;
    }

    ServiceElement *_service_element_to_add = create_service_element (_service);
    /* Add to XML */
    rc = add_service_to_xml (_service);
    if (rc == -1){
        printf("Impossible to add the Service to the XML file.\n");
        return -2;
    }
    else if(rc == -2){
        printf("The Service already exists\n");
    }
    LL_APPEND(service_head, _service_element_to_add);
    if(service_head == NULL)
    {
        printf("add_service_to_list failed\n");
        return -3;
    }
#if AVAHI == 1
    avahi_create_service (_service);
#endif
    return 0;
}

/**
 * Remove a service from the XML file, the server's service list, and the AVAHI server
 *
 * @param _service The service to remove
 *
 * @return 0 if the service has been successfully removed, 
 *        -1 if the removing from the XML file failed,
 *        -2 if the service couldn't be removed from the server's service list,
 *		  -3 if the service couldn't be removed from the AVAHI server
 */
int unregister_service_in_server( Service *_service )
{
    int rc;
    ServiceElement *_tmp, *_iterator;

    rc = remove_service_from_XML(_service);
    if( rc < 0 )
    {
        printf("remove_service_from_xml failed : %d\n", rc);
        return -1;
    }

    LL_FOREACH_SAFE(service_head, _iterator, _tmp)
    {
        if( strcmp( _iterator->service->value_url, _service->value_url ) == 0 )
        {
	   LL_DELETE(service_head, _iterator);
	   break;
        }
    }

#if AVAHI == 1
    rc = avahi_remove_service ( _service );
    if(  rc < 0 )
    {
        printf("avahi_remove_service failed : %d\n", rc);
        return -3;
    }
#endif

    return 0;
}

/**
 * Start the MHD web server and the AVAHI server
 *
 * @param _hostname Hostname for the local address of the server
 *
 * @param _domain_name Domain name for the local address of the server (if NULL = .local)
 *
 * @return 0 if the server has been successfully started, 
 *        -1 otherwise
 */
int start_server( char* _hostname, char* _domain_name)
{  
    init_xml_file (XML_FILE_NAME,DEVICE_LIST_ID);

    d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION, PORT, NULL, NULL, 
                          &answer_to_connection, &done_flag, MHD_OPTION_NOTIFY_COMPLETED, 
                          &request_completed, NULL, MHD_OPTION_END);
    if (NULL == d) return 1;
#if AVAHI == 1
    avahi_start (_hostname, _domain_name);
#endif

    return 0;
}

/**
 * Stop the MHD web server and the AVAHI server and delete the XML file
 *
 * @return void
 */
void stop_server()
{
    MHD_stop_daemon (d);
    delete_xml(XML_FILE_NAME);
#if AVAHI == 1
    avahi_quit ();
#endif
}

/**
 * Determines if a given service is registered in the server
 *
 * @param _service The service to check
 *
 * @return 0 If the given service is not registered
 *		   1 If the given service is registered
 */

int is_service_registered( Service *_service )
{
    ServiceElement *_iterator = NULL;

    LL_FOREACH( service_head, _iterator )
    {
        if(   ( strcmp( _iterator->service->device->type, _service->device->type ) == 0 ) 
           && ( strcmp( _iterator->service->device->ID, _service->device->ID ) == 0 )
           && ( strcmp( _iterator->service->type, _service->type ) == 0 )
           && ( strcmp( _iterator->service->ID, _service->ID ) == 0 )            )
        {
	   return 1;
        }
    }

    return 0;
}

/**
 * Looks in the server's service list for a matching service, and returns it.
 *
 * @param _device_type The type of device that owns the service
 *
 * @param _device_ID   The device's ID that own the service_head
 *
 * @param _service_type The type of the service to look for
 *
 * @param _service_ID The service's ID to look for
 *
 * @return Service* if a corresponding service was found
 *		   NULL    otherwise
 */

Service* get_service_from_server( char *_device_type, char *_device_ID, char *_service_type, char *_service_ID )
{
    ServiceElement *_iterator = NULL;

    LL_FOREACH(service_head, _iterator)
    {
        if(   ( strcmp( _iterator->service->device->type, _device_type ) == 0 ) 
           && ( strcmp( _iterator->service->device->ID, _device_ID ) == 0 )
           && ( strcmp( _iterator->service->type, _service_type ) == 0 )
           && ( strcmp( _iterator->service->ID, _service_ID ) == 0 )            )
        {
	   return _iterator->service;
        }
    }

    printf("get_service_from_server : No matching service found\n");

    return NULL;
}

/**
 * Looks in the server's service list for a matching device, and returns it.
 *
 * @param _device_type The type of device to look for
 *
 * @param _device_ID   The device's ID to look for
 *
 * @return Device* if a corresponding device was found
 *		   NULL    otherwise
 */

Device* get_device_from_server( char *_device_type, char *_device_ID)
{
    ServiceElement *_iterator = NULL;

    LL_FOREACH(service_head, _iterator)
    {
        if(   ( strcmp( _iterator->service->device->type, _device_type ) == 0 ) 
           && ( strcmp( _iterator->service->device->ID, _device_ID ) == 0 ))
        {
	   return _iterator->service->device;
        }
    }

    printf("get_device_from_server : No matching service found\n");

    return NULL;
}
