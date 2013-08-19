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

#ifndef CMEM_H
#define CMEM_H

#include <inttypes.h>
#include <pthread.h>

#include "cfg.h"
#include "io.h"

#define CMEM_MAX_DEVICES 8

struct cmem_unit_proto_t;

typedef struct cmem_unit_proto_t * (*cmem_unit_f_create)(struct cfg_arg_t *args);
typedef void (*cmem_unit_f_shutdown)(struct cmem_unit_proto_t *unit);
typedef void (*cmem_unit_f_reset)(struct cmem_unit_proto_t *unit);
typedef int (*cmem_unit_f_cmd)(struct cmem_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg);

struct cmem_chan_t;

struct cmem_unit_proto_t {
	const char *name;

	cmem_unit_f_create create;
	cmem_unit_f_shutdown shutdown;
	cmem_unit_f_reset reset;
	cmem_unit_f_cmd cmd;

	struct cmem_chan_t *chan;
	int num;
};

struct cmem_chan_t {
    struct chan_proto_t proto;

	pthread_mutex_t int_mutex;
	int int_mask;
	int int_unit[CMEM_MAX_DEVICES];
	int int_reported;
	int was_en;
	int untransmitted;

	pthread_mutex_t transmit_mutex;
	struct cmem_unit_proto_t *unit[CMEM_MAX_DEVICES];
};

// interrupts
enum cmem_int_e {
	CMEM_INT_TOO_SLOW	= 001, // -transmission too slow (?)
	CMEM_INT_NOMEM		= 002, // no memory
	CMEM_INT_COMPARE	= 003, // transmission with comparison failed
	CMEM_INT_PARITY		= 004, // -memory parity error
	CMEM_INT_NONE		= 9999,// no interrupt (em400 marker)
};

struct chan_proto_t * cmem_create(struct cfg_unit_t *units);
void cmem_shutdown(struct chan_proto_t *chan);
void cmem_reset(struct chan_proto_t *chan);
void cmem_int(struct cmem_chan_t *chan, int unit_n, int interrupt);
int cmem_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
