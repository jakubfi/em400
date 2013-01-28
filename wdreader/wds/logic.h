#include <SaleaeDeviceApi.h>

extern Logic16Interface* dev;

extern U64 logic_id;

extern volatile int logic_connected;

struct data_t {
	U8* d;
	struct data_t *next;
};

extern struct data_t *data_last;
extern struct data_t *data_head;

void __stdcall OnDisconnect(U64 device_id, void* user_data);
void __stdcall OnReadData(U64 device_id, U8* data, U32 data_length, void* user_data);
void __stdcall OnError(U64 device_id, void* user_data);
void __stdcall OnConnect(U64 device_id, GenericInterface* device_interface, void* user_data);

// vim: tabstop=4
