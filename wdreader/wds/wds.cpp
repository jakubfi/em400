#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "logic.h"
#include "wdc.h"

int debug;
int shake_pattern[] = { 1, 614, 50, 550, 100, 500, 150, 450, 200, 400, 210, 390, 220, 380, 240, 360, 260, 340, 280, 320, 0 };

// -----------------------------------------------------------------------
int process_data(struct data_t *input, U8 *output)
{
	// make sure we have something
	if (!input || !input->d || (input->len == 0)) {
		return -1;
	}

	int icnt = 0;	// input byte counter per data block
	int ticnt = 0;	// input word counter (total)
	int ocnt = 0;	// output sample counter
	int bit = 0;	// output bit position
	int index = 0;	// counter of INDEX rising edges
	int oipos = 0;	// previous INDEX position
	int ov = 0;		// previous sample value
	int v;			// current sample value

	// trim and pack the data
	struct data_t *d = input;
	while (d) {
		v = *((d->d)+icnt);

		// search for INDEX rising edge (200000 samples to make sure we drop noise at the falling edge)
		if (((ov & 1) == 1) && ((v & 1) == 0) && (ticnt-oipos>200000)) {
			index++;
			oipos = ticnt;
		}

		ticnt++;
		icnt += 2;
		ov = v;

		// we're collecting data
		if (index == 2) {
			// clear the byte if this is the 0th bit
			if (bit == 0) {
				output[ocnt] = 0;
			}

			output[ocnt] |= ((v&2)>>1) << (7-bit);

			bit++;

			// if this is the last bit, skip to next byte
			if (bit >= 8) {
				bit = 0;
				ocnt++;
			}
		}

		// data block done, get the next one
		if (icnt >= d->len) {
			d = d->next;
			icnt = 0;
		}
	}
	return ocnt;
}

// -----------------------------------------------------------------------
bool read_track(char *session, int drive, int cylinder, int head)
{
	static U8 *output_data;
	if (!output_data) output_data = (U8*) malloc(10000000);
	data_counter = 0;

	if (device_error) {
		return false;
	}

	// start reading data
	dev->ReadStart();

	// wait for at least two full disk revolutions
	pthread_mutex_lock(&dcount_mutex);
	while ((data_counter < 7000000) && !device_error) {
		pthread_cond_wait(&dcount_cv, &dcount_mutex);
	}
	pthread_mutex_unlock(&dcount_mutex);

	// stop reading and wait for Logic to stop streaming
	dev->Stop();
	while (dev->IsStreaming()) usleep(100);

	// process the data into output_data
	int obytes = process_data(data_head, output_data);
	if ((obytes <= 0) || device_error) {
		drop_logic_buffers();
		return false;
	}

	// dump data to a file
	char sname[1024];
	sprintf(sname, "%s--%i--%03i--%i.wds", session, drive, cylinder, head);
	int f = open(sname, O_WRONLY | O_EXCL | O_CREAT | O_NOATIME | O_SYNC | O_TRUNC, S_IWUSR | S_IRUSR | S_IRGRP);
	if (f < 0) {
		drop_logic_buffers();
		return false;
	}
	int res = write(f, output_data, obytes);
	if (res != obytes) {
		close(f);
		drop_logic_buffers();
		return false;
	}
	close(f);

	drop_logic_buffers();
	if (device_error) {
		return false;
	}
	return true;
}

// -----------------------------------------------------------------------
bool dump(int direction)
{
	bool res;

	time_t rawtime;
	struct tm * timeinfo;
	char session_name[100];

	// preapre session name
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	if (direction > 0) {
		strftime(session_name, 99, "fwd-%Y-%m-%d-%H-%M-%S", timeinfo);
	} else {
		strftime(session_name, 99, "bwd-%Y-%m-%d-%H-%M-%S", timeinfo);
	}
	printf(" Starting session: %s\n", session_name);

	// check if session is started
	if (!session_name || !*session_name) {
		printf(" Session name empty\n");
		return false;
	}

	// select the drive
	res = wdc_set_drive(1);
	if (!res) {
		printf(" Cannot select drive\n");
		return false;
	}
	// check Logic16 status
	if (device_error) {
		printf(" Logic device error\n");
		return false;
	}

	// iterate over all cylinders
	int cyl;
	for (int c=0 ; c<615 ; c++) {
		// mind the direction
		if (direction != 1) {
			cyl = 614 - c;
		} else {
			cyl = c;
		}

		printf(" Dumping cylinder: %i\n", cyl);

		res = wdc_seek(cyl);
		if (!res) {
			printf(" Seek failed");
			return false;
		}
		// iterate over all tracks
		for (int h=0 ; h<4 ; h++) {
			res = wdc_set_head(h);
			if (!res) {
				printf(" Cannot set head\n");
				return false;
			}
			// read the track
			res = read_track(session_name, drive, cylinder, head);
			if (!res) {
				printf(" Cannot read track\n");
				return false;
			}
		}
	}
	return true;
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	printf("Waiting for Logic interface...\n");

	DevicesManagerInterface::RegisterOnConnect(&OnConnect);
	DevicesManagerInterface::RegisterOnDisconnect(&OnDisconnect);
	DevicesManagerInterface::BeginConnect();

	while (!logic_connected) {
		sleep(1);
	}

	printf("Logic ready.\n");

	printf("Opening WDC control connection...\n");
	wdc_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

	if (wdc_fd == -1) {
		printf("Cannot open serial port.\n");
		exit(1);
	}

	fcntl(wdc_fd, F_SETFL, 0);
	serial_setup(wdc_fd);

	printf("Control connection ready.\n");

	char *buf;
	int res;
	rl_bind_key('\t', rl_abort);

	int quit = 0;
	while (!quit) { 
		buf = readline("wdc> ");
		res = false;

		if (!strcmp(buf, "quit")) {
			quit = 1;
			res = true;
		} else if (!strcmp(buf, "debug")) {
			if (debug) debug = 0;
			else debug = 1;
			res = true;

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
		} else if (!strncmp(buf, "drive", 5)) {
			res = wdc_set_drive(atoi(buf+6));
		} else if (!strncmp(buf, "head", 4)) {
			res = wdc_set_head(atoi(buf+5));

		} else if (!strcmp(buf, "test")) {
			res = read_track("test_track", drive, cylinder, head);
		} else if (!strcmp(buf, "shake")) {
			int *shake = shake_pattern;
			while (*shake != 0) {
				res = wdc_seek(*shake);
				if (!res) break;
				shake++;
			}
		} else if (!strcmp(buf, "warmup")) {
			int c;
			for (c=0 ; c<=614 ; c++) {
				res = wdc_seek(c);
				if (!res) break;
			}
			for (c=614 ; c>=0 ; c--) {
				wdc_seek(c);
				if (!res) break;
			}

		} else if (!strcmp(buf, "dump fw")) {
			res = dump(1);
		} else if (!strcmp(buf, "dump bw")) {
			res = dump(-1);
		} else {
			printf(" Unknown command.\n");
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
