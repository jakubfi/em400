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
	IO_IN,
	IO_OU
};

// TODO: those are in fact interfece signals
enum io_result {
	IO_NO = 0,  // no channel, no control unit, or no memory block
	IO_EN = 1,  // not ready (engaged)
	IO_OK = 2,  // OK
	IO_PE = 3   // data error (parity error)
};

struct chan_t {
	int type;
	volatile int finish;
	void (*f_shutdown)(struct chan_t *ch);
	void (*f_reset)(struct chan_t *ch);
	int (*f_cmd)(struct chan_t *ch, int dir, int unit, int cmd, int r);
	pthread_t thread;
	uint16_t int_spec;
	uint8_t int_mask;
	uint8_t dev_alloc;
};

struct unit_t {
	int type;
	struct chan_t *chan;
	void (*f_shutdown)(struct unit_t *u);
	void (*f_reset)(struct unit_t *u);
	int (*f_cmd)(struct unit_t *u, int dir, int cmd, int r);
};

extern struct chan_t io_chan[IO_MAX_CHAN];
extern struct unit_t io_unit[IO_MAX_CHAN][IO_MAX_UNIT];

int io_init();
void io_shutdown();
uint16_t io_get_int_spec(int interrupt);
int io_dispatch(int dir, uint16_t n, int r);

#endif

// vim: tabstop=4
