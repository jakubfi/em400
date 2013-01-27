#include <memory.h>
#include <stdio.h>
#include <pthread.h>

#include <SaleaeDeviceApi.h>


Logic16Interface* dev = NULL;

U64 logic_id = 0;
U32 sample_rate = 4000000;

// -----------------------------------------------------------------------
void OnDisconnect(U64 device_id, void* user_data)
{
	if (device_id == logic_id) {
		printf("A device was disconnected (id=0x%llx)\n", device_id);
		dev = NULL;
	}
}

// -----------------------------------------------------------------------
void OnReadData(U64 device_id, U8* data, U32 data_length, void* user_data)
{
	printf("Read %s words, starting with 0x%x\n", data_length/2, *(U16*)data);

	// you own this data.  You don't have to delete it immediately,
	// you could keep it and process it later, for example, or pass it to another thread for processing.
	DevicesManagerInterface::DeleteU8ArrayPtr(data);
}

// -----------------------------------------------------------------------
void OnError(U64 device_id, void* user_data)
{
	// A device reported an Error. This probably means that it
	// could not keep up at the given data rate, or was disconnected.
	// You can re-start the capture automatically, if your application
	// can tolerate gaps in the data.
	// note that you should not attempt to restart data collection
	// from this function -- you'll need to do it from your main thread
	// (or at least not the one that just called this function).
}

// -----------------------------------------------------------------------
void OnConnect(U64 device_id, GenericInterface* device_interface, void* user_data)
{
	if (device_interface != NULL) {
		printf("A Logic16 device was connected (id=0x%llx)\n", device_id);

		dev = (Logic16Interface*)device_interface;
		logic_id = device_id;

		dev->RegisterOnReadData(&OnReadData);
		dev->RegisterOnError(&OnError);

		U32 channels[16];
		for(U32 i=0; i<16; i++) {
			channels[i] = i;
		}

		dev->SetActiveChannels(channels, 16);
		dev->SetSampleRateHz(sample_rate);
	}
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char *argv[])
{
	DevicesManagerInterface::RegisterOnConnect(&OnConnect);
	DevicesManagerInterface::RegisterOnDisconnect(&OnDisconnect);
	DevicesManagerInterface::BeginConnect();

	while(1);

	return 0;
}

// vim: tabstop=4
