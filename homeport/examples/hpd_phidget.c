#include <assert.h>
#include <ev.h>
#include <pthread.h>

#include "hpd_phidget.h"
#include "linked_list.h"

#define PHIDGET_DEVICE "phidget"

// Data struct for event queue
struct event {
   CPhidgetHandle ifk;
   int index;
   int state;
};

/** A CPhidgetInterfaceKitHandle to receive new phidget connecting */
static CPhidgetInterfaceKitHandle waitingIfKit = 0;

// Async watcher to handle async calls from phidget library
static struct ev_async event_w;

// Queues for watchers
static struct ll *attach_queue = NULL; 
static struct ll *detach_queue = NULL; 
static struct ll *event_queue  = NULL; 

// Common mutex for queues
static pthread_mutex_t queue_m = PTHREAD_MUTEX_INITIALIZER;

/** Function passed to HPD to get the value of an input */
static size_t 
get_input_value(Service *service, char *buffer, size_t max_buffer_size)
{
	int service_ID = atoi(service->ID), value; 
	CPhidgetInterfaceKit_getInputState ((CPhidgetInterfaceKitHandle)service->user_data_pointer,service_ID,&value);
	sprintf(buffer, "%d", value);
	return strlen(buffer);
}

/** Function passed to HPD to get the value of an output */
static size_t 
get_output_value(Service *service, char *buffer, size_t max_buffer_size)
{
	int service_ID = atoi(service->ID), value;
	CPhidgetInterfaceKit_getOutputState ((CPhidgetInterfaceKitHandle)service->user_data_pointer,service_ID,&value);
	sprintf(buffer, "%d", value);
	return strlen(buffer);
}

/** Function passed to HPD to set the value of an output */
static size_t 
put_output_value(Service *service, char *buffer, size_t max_buffer_size, char *value)
{
	int service_ID = atoi(service->ID), int_value; 
	int_value = atoi(value);
	CPhidgetInterfaceKit_setOutputState ((CPhidgetInterfaceKitHandle)service->user_data_pointer,service_ID, int_value);
	sprintf(buffer, "%d", int_value);
	return strlen(buffer);
}

static void
handle_events(struct ev_loop *loop, struct ev_async *w, int revents)
{
   pthread_mutex_lock(&queue_m);

   // Handle attachments
   while (ll_head(attach_queue) != NULL) {
      struct ll_iter *head = ll_head(attach_queue);
      CPhidgetHandle IFK = ll_data(head);
	   int serialNo, i, rc;
	   const char *name;
	   char *serial = (char*)malloc(10*sizeof(char));

      // Get data
	   CPhidget_getDeviceName(IFK, &name);
	   CPhidget_getSerialNumber(IFK, &serialNo);
	   sprintf(serial, "%d", serialNo);

      // Create device
	   Device *_device = create_device_struct(
            "phidget", serial, "0x01", "0x01", "V1", NULL, NULL,
            "ExampleRoom", PHIDGET_DEVICE, HPD_NON_SECURE_DEVICE);

      // Create services
      for( i = 0; i < 8 ; i++ )
	   {
	   	char *num = (char*)malloc(2*sizeof(char));
	   	sprintf(num, "%d", i);
	   	Service *_service_input = create_service_struct ("input", num, 1, "input", "0/1", _device, 
	                                                    get_input_value, NULL, 
	                                                    create_parameter_struct ("0", NULL, NULL,
	                                                                             NULL, NULL, NULL,
	                                                                             NULL, NULL), IFK);
	   	assert( !(rc = HPD_register_service (_service_input)) );
	   	Service *_service_output = create_service_struct ("output", num, 0, "output", "0/1", _device, 
	                                                    get_output_value, put_output_value, 
	                                                    create_parameter_struct ("0", NULL, NULL,
	                                                                             NULL, NULL, NULL,
	                                                                             NULL, NULL), IFK);
	   	assert( !(rc = HPD_register_service (_service_output)) );
	   	free(num);
	   }

      // Clean up
      free(serial);

      // Remove event from queue
      struct ll_iter *iter = ll_head(attach_queue);
      ll_remove(iter);
   }

   // Handle input
   while (ll_head(event_queue) != NULL) {
      struct ll_iter *head = ll_head(event_queue);
      struct event *data = ll_data(head);
      CPhidgetHandle IFK = data->ifk;
      int Index = data->index;
      int State = data->state;
	   int serialNo = 0;
	   char return_value[4], serial[10], index[3];
	   
      printf("Input number %d changed to state %d\n", Index, State);

      // Get data
	   CPhidget_getSerialNumber(IFK, &serialNo);
	   sprintf(serial, "%d", serialNo);
	   sprintf(index, "%d", Index);
	   Service *changed_service = HPD_get_service("phidget", serial, "input", index);
	   sprintf(return_value, "%d", State);

      // Send update
	   HPD_send_event_of_value_change (changed_service, return_value);	

      // Remove event from queue
      head = ll_head(event_queue);
      ll_remove(head);
   }

   // Handle detachments
   while (ll_head(detach_queue) != NULL) {
      struct ll_iter *head = ll_head(detach_queue);
      CPhidgetHandle IFK = ll_data(head);
      int serialNo;
	   char serial[10];
	   const char *name;

      // Get data
	   CPhidget_getDeviceName (IFK, &name);
	   CPhidget_getSerialNumber(IFK, &serialNo);
	   sprintf(serial, "%d", serialNo);
	   Device *_device = HPD_get_device(PHIDGET_DEVICE, serial);

      // Unregister device
	   HPD_unregister_device_services(_device);
	   destroy_device_struct(_device);

      // Close phidget
	   CPhidget_close(IFK);
	   CPhidget_delete(IFK);

      // Remove event from queue
      head = ll_head(detach_queue);
      ll_remove(head);
   }

   pthread_mutex_unlock(&queue_m);
}

/** Function that manage the attachment of a new phidget */
int 
AttachHandler( CPhidgetHandle IFK, void *userptr )
{
   struct ev_loop *loop = userptr;
   struct ll_iter *tail;

   // Add handle to queue
   pthread_mutex_lock(&queue_m);
   tail = ll_tail(attach_queue);
   ll_insert(attach_queue, tail, IFK);
   pthread_mutex_unlock(&queue_m);

   // Signal watcher
   ev_async_send(loop, &event_w);

   // Remove handler
   CPhidget_set_OnAttach_Handler((CPhidgetHandle)IFK, NULL, NULL);

   // TODO Why this ?!
	//phidget_init (NULL);	

	return 0;
}

/** Function that manages the detachment of a phidget */
int 
DetachHandler( CPhidgetHandle IFK, void *userptr )
{
   struct ev_loop *loop = userptr;
   struct ll_iter *tail;

   // Add handle to queue
   pthread_mutex_lock(&queue_m);
   tail = ll_tail(detach_queue);
   ll_insert(detach_queue, tail, IFK);
   pthread_mutex_unlock(&queue_m);

   // Signal watcher
   ev_async_send(loop, &event_w);

	return 0;
}

/** Handle the errors : Not implemented */
int 
ErrorHandler( CPhidgetHandle IFK, void *userptr, int ErrorCode, const char *unknown )
{
   //struct ev_loop *loop = userptr;

	//printf("Error handled. %d - %s", ErrorCode, unknown);

	return 0;

}

/** Manage the changement of an input */
int 
InputChangeHandler( CPhidgetInterfaceKitHandle IFK, void *usrptr,
                    int Index, int State )
{
   struct ev_loop *loop = usrptr;
   struct ll_iter *iter;

   // Create data
   struct event *data = malloc(sizeof(struct event));
   data->ifk = (CPhidgetHandle)IFK;
   data->index = Index;
   data->state = State;

   // Add handle to queue
   pthread_mutex_lock(&queue_m);
   iter = ll_tail(event_queue);
   ll_insert(event_queue, iter, data);
   pthread_mutex_unlock(&queue_m);

   // Signal watcher
   ev_async_send(loop, &event_w);

	return 0;
}

/** Init ressources needed */
int 
phidget_init(struct ev_loop *loop)
{
   // Setup watchers
   ev_async_init(&event_w, handle_events);
   ev_async_start(loop, &event_w);
   pthread_mutex_lock(&queue_m);
   ll_create(attach_queue); 
   ll_create(detach_queue); 
   ll_create(event_queue); 
   pthread_mutex_unlock(&queue_m);

	CPhidgetInterfaceKit_create(&waitingIfKit);

	CPhidget_set_OnAttach_Handler((CPhidgetHandle)waitingIfKit,
                                 AttachHandler, loop);

	CPhidget_set_OnDetach_Handler((CPhidgetHandle)waitingIfKit,
                                 DetachHandler, loop);

	CPhidget_set_OnError_Handler((CPhidgetHandle)waitingIfKit,
                                ErrorHandler, loop);


	CPhidgetInterfaceKit_set_OnInputChange_Handler(waitingIfKit,
         InputChangeHandler, loop);

	CPhidget_open((CPhidgetHandle)waitingIfKit, -1);

   return 0;
}

/** Deinit the ressources */
void 
phidget_deinit(struct ev_loop *loop)
{	
	CPhidgetManagerHandle phidm = 0;
  	CPhidgetHandle *device_array;
	int count, i, serialNo;
	char serial[10];

	memset(serial, '\0', 10);
	
	CPhidgetManager_create(&phidm);
	
	CPhidget_close((CPhidgetHandle)waitingIfKit);
	CPhidget_delete((CPhidgetHandle)waitingIfKit);

	CPhidgetManager_open(phidm);
	
	CPhidgetManager_getAttachedDevices(phidm, &device_array, &count);

	if( count != 0 )
	{
		for( i = 0; i < count ; i++ )
		{
			CPhidget_getSerialNumber(device_array[i], &serialNo);
			sprintf(serial, "%d",serialNo);
			CPhidgetHandle tmp = (CPhidgetHandle)HPD_get_device(PHIDGET_DEVICE, serial)->service_head->service->user_data_pointer;
			CPhidget_close(tmp);
			CPhidget_delete(tmp);
		}
	}
	
	CPhidgetManager_freeAttachedDevicesArray(device_array);

	CPhidgetManager_close(phidm);
	CPhidgetManager_delete(phidm);	

   // Clean up event loop
   ev_async_stop(loop, &event_w);
   pthread_mutex_lock(&queue_m);
   ll_destroy(attach_queue); 
   ll_destroy(detach_queue); 
   while (ll_head(event_queue) != NULL) {
      struct ll_iter *head = ll_head(event_queue);
      free(ll_data(head));
      ll_data(head) = NULL;
      ll_remove(head);
   }
   ll_destroy(event_queue); 
   pthread_mutex_unlock(&queue_m);
}
