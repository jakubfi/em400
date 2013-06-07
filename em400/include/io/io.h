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

#include "cfg.h"

#define IO_MAX_CHAN	16

enum io_dir {
	IO_IN = 0,
	IO_OU
};

enum io_result {
	IO_NO = 0,  // no channel, no control unit, or no memory block
	IO_EN = 1,  // not ready (engaged)
	IO_OK = 2,  // OK
	IO_PE = 3   // data error (parity error)
};

struct chan_t {
	int num;
	const char *name;

	void *i; // internal channel-specific state data

	void (*f_shutdown)(struct chan_t *chan);
	void (*f_reset)(struct chan_t *chan);
	int (*f_cmd)(struct chan_t *chan, int dir, uint16_t n, uint16_t *r);
};

struct cfg_unit_t; // why?
typedef int (*chan_initfun)(struct chan_t *chan, struct cfg_unit_t *units);

struct fundict_t {
	const char *name;
	chan_initfun f_init;
};

extern struct chan_t *io_chan[IO_MAX_CHAN];

int io_init();
void io_shutdown();
int io_dispatch(int dir, uint16_t n, uint16_t *r);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
