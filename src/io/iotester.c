//  Copyright (c) 2018 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>

#include "log.h"
#include "io/io.h"
#include "io/chan.h"
#include "utils/elst.h"
#include "cfg.h"

#define INIT_DELAY_US 200000

enum it_event_types { EV_CMD, EV_RESET, EV_QUIT, };

struct it_event {
	int type;
	int cmd;
	uint16_t r;
};

enum it_commands {
	CMD_ISP	= 0b0001, // FETCH
	CMD_RND	= 0b0010, // FETCH
	CMD_ANS	= 0b0011, // FETCH
	CMD_WNB	= 0b0001, // SEND
	CMD_WAM	= 0b0010, // SEND
	CMD_WAB	= 0b0011, // SEND
	CMD_WM	= 0b0100, // SEND
	CMD_RM	= 0b0101, // SEND
	CMD_WMM	= 0b0110, // SEND
	CMD_RMM	= 0b0111, // SEND
	CMD_IRQ	= 0b1000, // SEND
	CMD_PA	= 0b1001, // SEND
	CMD_ERI = 0b1010, // SEND
};

struct iotester {
	pthread_t thread;
	ELST evq;

	int chnum;
	uint16_t intspec;
};

static void * it_cmdproc(void *ptr);

/*
FETCH requests are handled immediately, in CPU thread.
Command and argument is sent on address bus, response is sent back on data bus

ECO 1_DDDD_DDDDDD_DDDD_D - respond back with whatever was sent on address bus
ISP 0_0001_xxxxxx_xxxx_x - send interrupt specification
ANS 0_0011_xxxxRR_xxxx_x - respond with R to this request (no, en, ok, pe)
RND 0_0010_xxxxxx_xxxx_x - respond with a random number

SEND requests are handled in separate iotester execution thread.
Command is sent on address bus, argument is sent on data bus (with register arg.)

WNB 0_0001_xxxxxx_xxxx_x - NB := r & 0b1111, set interrupt, intspec = 0
WAM 0_0010_xxxxxx_xxxx_x - AM := r, set interrupt, intspec = 0
WAB 0_0011_xxxxxx_xxxx_x - AB := r, set interrupt, intspec = 0
WM  0_0100_xxxxxx_xxxx_x - write r words from the buffer: [nb:am] := buf[ab], set interrupt, intspec = num_writes
RM  0_0101_xxxxxx_xxxx_x - read r words into the buffer: buf[ab] := [nb:am], set int., intspec = num_reads
WMM 0_0110_xxxxxx_xxxx_x - as WM with multi writes
RMM 0_0111_xxxxxx_xxxx_x - as RM with multi reads
IRQ 0_1000_xxxxxx_xxxx_x - set interrupt, intspec = r
PA  0_1001_xxxxxx_xxxx_x - set Power Alarm interrupt
ERI 0_1010_xxxxxx_xxxx_x - enable sending interrupt after reset (disabled on startup)
*/

// -----------------------------------------------------------------------
void it_event_destructor(void *ptr)
{
	free(ptr);
}

// -----------------------------------------------------------------------
void * it_create(int num, em400_cfg *cfg)
{
	struct iotester *it = (struct iotester *) calloc(1, sizeof(struct iotester));
	if (!it) {
		LOGERR("Memory allocation error.");
		return NULL;
	}

	it->chnum = num;
	srand(time(NULL));

	for (int i=0 ; i<16 ; i++) {
		char section[16];
		sprintf(section, "dev%i.%i", num, i);
		if (cfg_contains(cfg, section)) {
			LOG(L_IO, "IOtester can't connect devices. Ignored device: %s", section);
		}
	}

	it->evq = elst_create(1024, it_event_destructor);
	if (!it->evq) {
		LOGERR("Failed to create event queue.");
		free(it);
		return NULL;
	}

	if (pthread_create(&it->thread, NULL, it_cmdproc, it)) {
		LOGERR("Failed to spawn main I/O tester thread.");
		elst_destroy(it->evq);
		free(it);
		return NULL;
	}

	char name[16];
	sprintf(name, "iotest%02i", it->chnum);
	pthread_setname_np(it->thread, name);

	LOG(L_IO, "I/O tester created");

	return it;
}

// -----------------------------------------------------------------------
struct it_event *it_event_new(int type, int cmd, uint16_t r)
{
	struct it_event *ev = (struct it_event *) calloc(1, sizeof(struct it_event));
	ev->type = type;
	ev->cmd = cmd;
	ev->r = r;

	return ev;
}

// -----------------------------------------------------------------------
void it_shutdown(void *ch)
{
	if (!ch) return;

	struct iotester *it = (struct iotester *) ch;

	LOG(L_IO, "I/O tester shutting down");

	elst_insert(it->evq, it_event_new(EV_QUIT, 0, 0), 0);
	pthread_join(it->thread, NULL);
	elst_destroy(it->evq);
	free(ch);

	LOG(L_IO, "Shutdown complete");
}

// -----------------------------------------------------------------------
void it_reset(void *ch)
{
	struct iotester *it = (struct iotester *) ch;
	LOG(L_IO, "Received reset request");
	elst_insert(it->evq, it_event_new(EV_RESET, 0, 0), 0);
}

// -----------------------------------------------------------------------
static void * it_cmdproc(void *ptr)
{
	int quit = 0;
	struct iotester *it = (struct iotester *) ptr;

	LOG(L_IO, "Entering main I/O tester loop");

	uint16_t r;
	int res = 0;
	int nb = 0;
	uint16_t am = 0;
	uint16_t ab = 0;
	uint16_t buf[0x10000];
	int reset_int = 0;

	while (!quit) {
		struct it_event *ev = (struct it_event *) elst_wait_pop(it->evq, 0);
		switch (ev->type) {
			case EV_QUIT:
				LOG(L_IO, "Quit");
				quit = 1;
				break;
			case EV_RESET:
				if (!reset_int) {
					LOG(L_IO, "Reset (no interrupt sent)");
				} else {
					reset_int = 0;
					LOG(L_IO, "Reset delay %ius", INIT_DELAY_US);
					usleep(INIT_DELAY_US);
					LOG(L_IO, "Reset done, sending interrupt with intspec 0xffff");
					atomic_store_explicit(&it->intspec, 0xffff, memory_order_release);
					io_int_set(it->chnum);
				}
				break;
			case EV_CMD:
				r = ev->r;
				switch (ev->cmd) {
					case CMD_WNB:
						LOG(L_IO, "NB = %i", r & 0b1111);
						nb = r & 0b1111;
						atomic_store_explicit(&it->intspec, 0, memory_order_release);
						io_int_set(it->chnum);
						break;
					case CMD_WAM:
						LOG(L_IO, "AM = 0x%04x", r);
						am = r;
						atomic_store_explicit(&it->intspec, 0, memory_order_release);
						io_int_set(it->chnum);
						break;
					case CMD_WAB:
						LOG(L_IO, "AB = 0x%04x", r);
						ab = r;
						atomic_store_explicit(&it->intspec, 0, memory_order_release);
						io_int_set(it->chnum);
						break;
					case CMD_WM:
						LOG(L_IO, "single: buf[0x%04x] -> [%i:0x%04x], %i words", ab, nb, am, r);
						for (int i=0 ; i<r ; i++) {
							res = io_mem_write_1(nb, am+i, buf[ab+i]);
							if (!res) break;
						}
						atomic_store_explicit(&it->intspec, res, memory_order_release);
						io_int_set(it->chnum);
						break;
					case CMD_RM:
						LOG(L_IO, "single: [%i:0x%04x] -> buf[0x%04x], %i words", nb, am, ab, r);
						for (int i=0 ; i<r ; i++) {
							res = io_mem_read_1(nb, am+i, buf+ab+i);
							if (!res) break;
						}
						atomic_store_explicit(&it->intspec, res, memory_order_release);
						io_int_set(it->chnum);
						break;
					case CMD_WMM:
						LOG(L_IO, "multi: buf[0x%04x] -> [%i:0x%04x], %i words", ab, nb, am, r);
						res = io_mem_write_n(nb, am, buf+ab, r);
						atomic_store_explicit(&it->intspec, res, memory_order_release);
						io_int_set(it->chnum);
						break;
					case CMD_RMM:
						LOG(L_IO, "multi: [%i:0x%04x] -> buf[0x%04x], %i words", nb, am, ab, r);
						res = io_mem_read_n(nb, am, buf+ab, r);
						atomic_store_explicit(&it->intspec, res, memory_order_release);
						io_int_set(it->chnum);
						break;
					case CMD_IRQ:
						LOG(L_IO, "Sending interrupt (intspec set to 0x%04x)", r);
						atomic_store_explicit(&it->intspec, r, memory_order_release);
						io_int_set(it->chnum);
						break;
					case CMD_PA:
						LOG(L_IO, "Sending Power Alarm interrupt");
						io_int_set_pa();
						break;
					case CMD_ERI:
						reset_int = 1;
						LOG(L_IO, "Interrupt after reset enabled");
						break;
					default:
						LOG(L_IO, "Unknown 'SEND' command: %i", ev->cmd);
						atomic_store_explicit(&it->intspec, 0, memory_order_release);
						io_int_set(it->chnum);
						break;
				}
				break;
			default:
				LOG(L_IO, "Unknown event: %i", ev->type);
				break;
		}
		free(ev);
	}

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int it_cmd(void *ch, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	struct iotester *it = (struct iotester *) ch;

	int echo = (n_arg & 0b1000000000000000) >> 15;
	int cmd =  (n_arg & 0b0111100000000000) >> 11;
	int rmd =  (n_arg & 0b0000011111100000) >> 5;

	// 'FETCH' requests are served on the spot
	if (dir == IO_IN) {
		if (echo) {
			LOG(L_IO, "Echo: 0x%04x", n_arg);
			*r_arg = n_arg;
		} else {
			switch (cmd) {
				case CMD_ANS:
					LOG(L_IO, "CPU wants response: %i", rmd & 0b11);
					return (rmd & 0b11);
				case CMD_ISP:
					*r_arg = atomic_load_explicit(&it->intspec, memory_order_acquire);
					LOG(L_IO, "Sending intspec to CPU: 0x%04x", *r_arg);
					break;
				case CMD_RND:
					*r_arg = random();
					LOG(L_IO, "Generated random: 0x%04x", *r_arg);
					break;
				default:
					LOG(L_IO, "Unknown FETCH request: %i", cmd);
					return IO_NO;
			}
		}
	// 'SEND' requests are handled in the event thread except CMD_ANS
	} else {
		LOG(L_IO, "Enqueue command: %i, r_arg: 0x%04x", cmd, *r_arg);
		elst_append(it->evq, it_event_new(EV_CMD, cmd, *r_arg));
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
const struct chan_drv it_chan_driver = {
	.name = "iotester",
	.create = it_create,
	.shutdown = it_shutdown,
	.reset = it_reset,
	.cmd = it_cmd
};

// vim: tabstop=4 shiftwidth=4 autoindent
