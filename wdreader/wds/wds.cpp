#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "logic.h"
#include "wdc.h"

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	int quit = 0;

	printf("Opening control connection...\n");
	wdc_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

	if (wdc_fd == -1) {
		printf("Cannot open serial port.\n");
		exit(1);
		return -1;
	}

	serial_setup(wdc_fd);

	printf("Waiting for Winchester to become ready...\n");

	while (!(wdc_ready() && wdc_track0())) {
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
			res = false;
		} else if (!strcmp(buf, "in")) {
			res = wdc_step_in();
		} else if (!strcmp(buf, "out")) {
			res = wdc_step_out();
		} else if (!strcmp(buf, "status")) {
			res = wdc_ready();
		} else if (!strcmp(buf, "t0")) {
			res = wdc_track0();
		} else if (!strncmp(buf, "seek", 4)) {
			res = wdc_seek(atoi(buf+5));
		} else if (!strcmp(buf, "start")) {
			dev->ReadStart();
			sleep(1);
			dev->Stop();
			res = true;
		} else {
			res = false;
		}

		if (res) {
			printf(" OK\n");
		} else {
			printf(" ERR\n");
		}

		if (buf && *buf) {
			add_history(buf);
		}
		free(buf);
	}

	close(wdc_fd);
	
	return 0;
}

// vim: tabstop=4
