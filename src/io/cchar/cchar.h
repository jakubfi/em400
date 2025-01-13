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

#ifndef CCHAR_H
#define CCHAR_H

#include <inttypes.h>
#include <stdbool.h>
#include <pthread.h>
#include <uv.h>

#include "cfg.h"

#define CCHAR_MAX_DEVICES 8
#define CCHAR_INT_NONE 9999 // no interrupt (em400 marker)

typedef struct cchar_unit_proto_s cchar_unit_proto_t;

typedef cchar_unit_proto_t * (*cchar_unit_f_create)(em400_cfg *cfg, int ch_num, int dev_num);
typedef void (*cchar_unit_f_shutdown)(cchar_unit_proto_t *unit);
typedef void (*cchar_unit_f_free)(cchar_unit_proto_t *unit);
typedef void (*cchar_unit_f_reset)(cchar_unit_proto_t *unit);
typedef int (*cchar_unit_f_cmd)(cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg);
typedef int (*cchar_unit_f_intspec)(cchar_unit_proto_t *unit);
typedef bool (*cchar_unit_f_has_interrupt)(cchar_unit_proto_t *unit);

typedef struct cchar_chan_s cchar_chan_t;

struct cchar_unit_proto_s {
	const char *name;

	cchar_unit_f_create create;
	cchar_unit_f_shutdown shutdown;
	cchar_unit_f_free free;
	cchar_unit_f_reset reset;
	cchar_unit_f_cmd cmd;
	cchar_unit_f_intspec intspec;
	cchar_unit_f_has_interrupt has_interrupt;

	cchar_chan_t *chan;
	int num;
};

struct cchar_chan_s {
	int num;

	pthread_mutex_t int_mutex;
	int int_mask;
	int interrupting_device;
	int was_en;
	int untransmitted;

	cchar_unit_proto_t *unit[CCHAR_MAX_DEVICES];

	pthread_t ioloop_thread;
	uv_async_t async_quit;
};

void *cchar_create(int num, em400_cfg *cfg);
void cchar_shutdown(void *chan);
void cchar_reset(void *chan);
void cchar_int_trigger(cchar_chan_t *chan);
void cchar_int_cancel(cchar_chan_t *chan, int unit_n);
int cchar_cmd(void *chan, int dir, uint16_t n_arg, uint16_t *r_arg);

extern struct chan_drv cchar_chan_driver;

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
