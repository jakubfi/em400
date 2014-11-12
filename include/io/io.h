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
	IO_OU = 0,
	IO_IN = 1,
};

enum io_result {
	// value matters (IN/OU opcodes use it)
	IO_NO = 0,  // no channel, no control unit, or no memory block
	IO_EN = 1,  // not ready (engaged)
	IO_OK = 2,  // OK
	IO_PE = 3,  // data error (parity error)
};

typedef struct chan_proto_t * (*chan_f_create)(struct cfg_unit *units);
typedef void (*chan_f_shutdown)(struct chan_proto_t *chan);
typedef void (*chan_f_reset)(struct chan_proto_t *chan);
typedef int (*chan_f_cmd)(struct chan_proto_t *chan, int dir, uint16_t n, uint16_t *r);

struct chan_proto_t {
	int num;
	const char *name;
	chan_f_create create;
	chan_f_shutdown shutdown;
	chan_f_reset reset;
	chan_f_cmd cmd;
};

extern struct chan_proto_t *io_chan[IO_MAX_CHAN];

int io_init(struct cfg_em400 *cfg);
void io_shutdown();
void io_reset();
int io_dispatch(int dir, uint16_t n, uint16_t *r);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
