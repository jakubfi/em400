//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef IO_H
#define IO_H

#include <inttypes.h>
#include <pthread.h>

#define IO_MAX_CHAN	16
#define IO_MAX_UNIT	8

enum io_dir {
	IO_IN = 0,
	IO_OU
};

// TODO: those are in fact interfece signals
enum io_result {
	IO_NO = 0,  // no channel, no control unit, or no memory block
	IO_EN = 1,  // not ready (engaged)
	IO_OK = 2,  // OK
	IO_PE = 3   // data error (parity error)
};

struct unit_t;

// -----------------------------------------------------------------------
struct chan_t {
	int num;
	int type;
	const char *name;
	struct unit_t *unit[IO_MAX_UNIT];
	int finish;
	pthread_t thread;

	void (*f_shutdown)(void *self);
	void (*f_reset)(void *self);
	int (*f_cmd)(void *self, int u_num, int dir, int cmd, uint16_t *r);

	uint16_t int_spec; // 0 past-EN, 3-7 int spec, 8-10 dev number
	uint16_t untransmitted;
	int int_mask;
};

// -----------------------------------------------------------------------
struct unit_t {
	int num;
	const char *name;
	struct chan_t *chan;
	void *cfg;

	void (*f_shutdown)(void *self);
	void (*f_reset)(void *self);
	int (*f_cmd)(void *self, int u_num, int dir, int cmd, uint16_t *r);
};

extern struct chan_t io_chan[IO_MAX_CHAN];

int io_init();
void io_shutdown();
int io_dispatch(int dir, uint16_t n, uint16_t *r);

#endif

// vim: tabstop=4
