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

#ifndef DRV_DRIVERS_H
#define DRV_DRIVERS_H

#include <inttypes.h>

#include "cfg.h"
#include "io.h"

enum drv_role_e {
	DRV_NONE = -1,
	DRV_CHAN = 0,
	DRV_UNIT
};

enum drv_chan_e {
	CHAN_IGNORE = -1,
	CHAN_NONE = 0,
	CHAN_CHAR,
	CHAN_MEM,
	CHAN_MULTIX,
	CHAN_PLIX
};

struct drv_t {
	const int role;
	const int chan_type;
	const char *name;
	int argc;
	int max_devs;
	int (*f_init)(void *self, struct cfg_arg_t *arg);
	void (*f_shutdown)(void *self);
	void (*f_reset)(void *self);
	int (*f_cmd)(void *self, int dir, uint16_t n, uint16_t *r);
};

extern struct drv_t drv_drivers[];

struct drv_t * drv_get(int role, int chan_type, char *name);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
