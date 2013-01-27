#include <memory.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <SaleaeDeviceApi.h>

Logic16Interface* dev = NULL;

U64 logic_id = 0;

volatile int logic_connected = 0;

struct data_t {
	U8* d;
	struct data_t *next;
};

struct data_t *data_last = NULL;
struct data_t *data_head = NULL;

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
	printf("Read %i words\n", data_length/2);

	struct data_t *d = (struct data_t*) malloc(sizeof(struct data_t));
	d->d = data;
	d->next = NULL;

	if (!data_last) {
		data_head = data_last = d;
	} else {
		data_last->next = d;
		data_last = d;
	}

	// you own this data.  You don't have to delete it immediately,
	// you could keep it and process it later, for example, or pass it to another thread for processing.
	//DevicesManagerInterface::DeleteU8ArrayPtr(data);
}

// -----------------------------------------------------------------------
void __stdcall OnError(U64 device_id, void* user_data)
{
	printf("Device error\n");
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

		U32 channels[3];
		for(U32 i=0; i<3; i++) {
			channels[i] = i;
		}

		dev->SetUse5Volts(true);
		dev->SetActiveChannels(channels, 3);
		dev->SetSampleRateHz(10*1000*1000);
	}

	logic_connected = 1;
}

// -----------------------------------------------------------------------
void serial_setup(int fd)
{
	struct termios options;

	fcntl(fd, F_SETFL, 0);

	// 9600 baud
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	// enable receiver, local mode
	options.c_cflag |= (CLOCAL | CREAD);

	// 8N1
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	// disable hardware flow control
	options.c_cflag &= ~CRTSCTS;

	// parity checking
	options.c_iflag &= ~(INPCK | ISTRIP);

	// disable software flow control
	options.c_iflag &= ~(IXON | IXOFF | IXANY);

	// raw mode
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// no postprocess
	options.c_oflag &= ~OPOST;

	tcsetattr(fd, TCSANOW, &options);
}

// -----------------------------------------------------------------------
int transmit(int fd, char c)
{
	char inbuf[1024 + 1] = {0};

	int res = write(fd, &c, 1);

	if (res != 1) {
		return -2;
	}

	res = 0;

	while (res != 3) {
		res += read(fd, inbuf+res, 1024);
	}

	if (!strcmp(inbuf, "OK!")) {
		return 0;
	}
	return -1;
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	int quit = 0;

	printf("Opening control connection...\n");
	int fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

	if (fd == -1) {
		printf("Cannot open serial port.\n");
		exit(1);
		return -1;
	}

	serial_setup(fd);

	printf("Waiting for Winchester to become ready...\n");

	while (transmit(fd, 's') || transmit(fd, 't')) {
		sleep(1);
	}

	printf("Winchester ready.\n");

	printf("Waiting for Logic interface...\n");

	DevicesManagerInterface::RegisterOnConnect(&OnConnect);
	DevicesManagerInterface::RegisterOnDisconnect(&OnDisconnect);
	DevicesManagerInterface::BeginConnect();

	while (!logic_connected) {
		sleep(1);
	}

	printf("Logic ready\n");

	char *buf;
	int res;
	rl_bind_key('\t',rl_abort);

	while (!quit) { 
		buf = readline("wdc> ");

		if (!strcmp(buf, "quit")) {
			quit = 1;
			res = 0;
		} else if (!strcmp(buf, "in")) {
			res = transmit(fd, 'i');
		} else if (!strcmp(buf, "out")) {
			res = transmit(fd, 'o');
		} else if (!strcmp(buf, "status")) {
			res = transmit(fd, 's');
		} else if (!strcmp(buf, "t0")) {
			res = transmit(fd, 't');
		} else if (!strcmp(buf, "start")) {
			dev->ReadStart();
			sleep(1);
			dev->Stop();
			res = 0;
		} else {
			res = -1;
		}

		if (res == 0) {
			printf(" OK\n");
		} else {
			printf(" ERR\n");
		}

		if (buf && *buf) {
			add_history(buf);
		}
		free(buf);
	}

	close(fd);
	
	return 0;
}

// vim: tabstop=4
