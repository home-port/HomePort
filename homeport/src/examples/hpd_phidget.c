#include <assert.h>

#include "hpd_phidget.h"


#define PHIDGET_DEVICE "phidget"

/** A CPhidgetInterfaceKitHandle to receive new phidget connecting */
CPhidgetInterfaceKitHandle waitingIfKit = 0;

/** Function passed to HPD to get the value of an input */
size_t 
get_input_value( Service *service, char *buffer, size_t max_buffer_size )
{
	int service_ID = atoi(service->ID), value; 
	CPhidgetInterfaceKit_getInputState ((CPhidgetInterfaceKitHandle)service->user_data_pointer,service_ID,&value);
	sprintf(buffer, "%d", value);
	return strlen(buffer);
}

/** Function passed to HPD to get the value of an output */
size_t 
get_output_value( Service *service, char *buffer, size_t max_buffer_size )
{
	int service_ID = atoi(service->ID), value;
	CPhidgetInterfaceKit_getOutputState ((CPhidgetInterfaceKitHandle)service->user_data_pointer,service_ID,&value);
	sprintf(buffer, "%d", value);
	return strlen(buffer);
}

/** Function passed to HPD to set the value of an output */
size_t 
put_output_value( Service *service, char *buffer, size_t max_buffer_size, char *value )
{
	int service_ID = atoi(service->ID), int_value; 
	int_value = atoi(value);
	CPhidgetInterfaceKit_setOutputState ((CPhidgetInterfaceKitHandle)service->user_data_pointer,service_ID, int_value);
	sprintf(buffer, "%d", int_value);
	return strlen(buffer);
}


/** Function that manage the attachment of a new phidget */
int 
AttachHandler( CPhidgetHandle IFK, void *userptr )
{

	int serialNo, i, rc;

	const char *name;

	char *serial = (char*)malloc(10*sizeof(char));

	CPhidget_getDeviceName(IFK, &name);

	CPhidget_getSerialNumber(IFK, &serialNo);

	CPhidget_set_OnAttach_Handler((CPhidgetHandle)IFK, NULL, NULL);

	sprintf(serial, "%d", serialNo);

	Device *_device = create_device_struct("phidget", 
						serial, 
						"0x01", 
						"0x01", 
						"V1", 
						NULL, 
						NULL, 
						"ExampleRoom", 
						PHIDGET_DEVICE,																				 
						HPD_NON_SECURE_DEVICE);

	free(serial);

	for( i = 0; i < 8 ; i++ )
	{
		char *num = (char*)malloc(2*sizeof(char));
		sprintf(num, "%d", i);
		Service *_service_input = create_service_struct ("input", num, "input", "0/1", _device, 
	                                                 get_input_value, NULL, 
	                                                 create_parameter_struct ("0", NULL, NULL,
	                                                                          NULL, NULL, NULL,
	                                                                          NULL, NULL), IFK);
		assert( !(rc = HPD_register_service (_service_input)) );
		
		
		Service *_service_output = create_service_struct ("output", num, "output", "0/1", _device, 
	                                                 get_output_value, put_output_value, 
	                                                 create_parameter_struct ("0", NULL, NULL,
	                                                                          NULL, NULL, NULL,
	                                                                          NULL, NULL), IFK);
		assert( !(rc = HPD_register_service (_service_output)) );

		free(num);
		num = NULL;
		
	}

	HPD_phidget_init ();	

	return 0;

}

/** Function that manages the detachment of a phidget */
int 
DetachHandler( CPhidgetHandle IFK, void *userptr )
{

	int serialNo;
	char serial[10];
	const char *name;

	CPhidget_getDeviceName (IFK, &name);

	CPhidget_getSerialNumber(IFK, &serialNo);

	sprintf(serial, "%d", serialNo);

	Device *_device = HPD_get_device(PHIDGET_DEVICE, serial);

	HPD_unregister_device_services(_device);

	destroy_device_struct(_device);

	CPhidget_close(IFK);
	CPhidget_delete(IFK);

	return 0;

}

/** Handle the errors : Not implemented */
int 
ErrorHandler( CPhidgetHandle IFK, void *userptr, int ErrorCode, const char *unknown )
{

	//printf("Error handled. %d - %s", ErrorCode, unknown);

	return 0;

}

/** Manage the changement of an input */
int 
InputChangeHandler( CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int State )
{
	printf("Input number %d changed to state %d\n", Index, State);

	int serialNo = 0;
	
	char return_value[4], serial[10], index[3];

	CPhidget_getSerialNumber((CPhidgetHandle)IFK, &serialNo);

	sprintf(serial, "%d", serialNo);

	sprintf(index, "%d", Index);
	
	Service *changed_service = HPD_get_service("phidget", serial, "input", index);

	sprintf(return_value, "%d", State);
	
	HPD_send_event_of_value_change (changed_service, return_value);	

	return 0;
}

/** Init ressources needed */
void 
HPD_phidget_init()
{
	CPhidgetInterfaceKit_create(&waitingIfKit);

	CPhidget_set_OnAttach_Handler((CPhidgetHandle)waitingIfKit, AttachHandler, NULL);

	CPhidget_set_OnDetach_Handler((CPhidgetHandle)waitingIfKit, DetachHandler, NULL);

	CPhidget_set_OnError_Handler((CPhidgetHandle)waitingIfKit, ErrorHandler, NULL);


	CPhidgetInterfaceKit_set_OnInputChange_Handler (waitingIfKit, InputChangeHandler, NULL);

	CPhidget_open((CPhidgetHandle)waitingIfKit, -1);
}

/** Deinit the ressources */
void 
HPD_phidget_deinit()
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
}
