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
#include <pthread.h>

#include "cfg.h"
#include "io.h"

#define CCHAR_MAX_DEVICES 8

struct cchar_unit_proto_t;

typedef struct cchar_unit_proto_t * (*cchar_unit_f_create)(struct cfg_arg_t *args);
typedef void (*cchar_unit_f_shutdown)(struct cchar_unit_proto_t *unit);
typedef void (*cchar_unit_f_reset)(struct cchar_unit_proto_t *unit);
typedef int (*cchar_unit_f_cmd)(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg);

struct cchar_chan_t;

struct cchar_unit_proto_t {
    const char *name;

    cchar_unit_f_create create;
    cchar_unit_f_shutdown shutdown;
    cchar_unit_f_reset reset;
    cchar_unit_f_cmd cmd;

    struct cchar_chan_t *chan;
    int num;
};

struct cchar_chan_t {
    struct chan_proto_t proto;

	pthread_mutex_t int_mutex;
	int int_mask;
	int int_unit[CCHAR_MAX_DEVICES];
	int int_reported;
	int was_en;
	int untransmitted;

    struct cchar_unit_proto_t *unit[CCHAR_MAX_DEVICES];
};

struct chan_proto_t *cchar_create(struct cfg_unit_t *units);
void cchar_shutdown(struct chan_proto_t *chan);
void cchar_reset(struct chan_proto_t *chan);
void cchar_int(struct cchar_chan_t *chan, int unit_n, int interrupt);
int cchar_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
