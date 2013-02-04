#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <SaleaeDeviceApi.h>
#include "logic.h"

Logic16Interface* dev = NULL;
U64 logic_id;
volatile int logic_connected;
struct data_t *data_last;
struct data_t *data_head;

int data_counter;
bool device_error = false;

pthread_mutex_t dcount_mutex;
pthread_cond_t dcount_cv;


// -----------------------------------------------------------------------
void drop_logic_buffers()
{
	struct data_t *d = data_head;
	struct data_t *x = NULL;

	while (d) {
		DevicesManagerInterface::DeleteU8ArrayPtr(d->d);
		x = d;
		d = d->next;
		free(x);
	}
	data_head = data_last = NULL;
}

// -----------------------------------------------------------------------
void __stdcall OnDisconnect(U64 device_id, void* user_data)
{
	if (device_id == logic_id) {
		printf("Logic disconnected: 0x%llx)\n", device_id);
		dev = NULL;
	}
}

// -----------------------------------------------------------------------
void __stdcall OnReadData(U64 device_id, U8* data, U32 data_length, void* user_data)
{
	//printf(" Read %i bytes, three first bytes: %d, %d, %d\n", data_length, *data, *(data+1), *(data+2));

	// create new data packet
	struct data_t *d = (struct data_t*) malloc(sizeof(struct data_t));
	d->d = data;
	d->next = NULL;
	d->len = data_length;

	// append the packet to the list
	if (!data_last) {
		data_head = data_last = d;
	} else {
		data_last->next = d;
		data_last = d;
	}

	pthread_mutex_lock(&dcount_mutex);
	data_counter += data_length;
	pthread_cond_signal(&dcount_cv);
	pthread_mutex_unlock(&dcount_mutex);

	//DevicesManagerInterface::DeleteU8ArrayPtr(data);
}

// -----------------------------------------------------------------------
void __stdcall OnError(U64 device_id, void* user_data)
{
	printf("Device error\n");
	device_error = true;
	// A device reported an Error. This probably means that it
	// could not keep up at the given data rate, or was disconnected.
	// You can re-start the capture automatically, if your application
	// can tolerate gaps in the data.
	// note that you should not attempt to restart data collection
	// from this function -- you'll need to do it from your main thread
	// (or at least not the one that just called this function).
}

// -----------------------------------------------------------------------
void __stdcall OnConnect(U64 device_id, GenericInterface* device_interface, void* user_data)
{
	if(dynamic_cast<Logic16Interface*>(device_interface) != NULL) {
		printf("Logic connected: 0x%llx\n", device_id);

		dev = (Logic16Interface*)device_interface;
		logic_id = device_id;

		dev->RegisterOnReadData(&OnReadData);
		dev->RegisterOnError(&OnError);

		U32 channels[2];
		for(U32 i=0; i<2; i++) {
			channels[i] = i;
		}

		dev->SetUse5Volts(true);
		dev->SetActiveChannels(channels, 2);
		dev->SetSampleRateHz(100*1000*1000);
	}

	logic_connected = 1;
}

// vim: tabstop=4
