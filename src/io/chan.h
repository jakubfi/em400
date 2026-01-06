//  Copyright (c) 2012-2025 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef CHAN_H
#define CHAN_H

#include <inttypes.h>

#include "io/defs.h"
#include "io/dev/dev.h"

typedef struct chan chan_t;

// TODO: needs further cleaning (possibly move down, as interpretation is channel-specific)
enum chan_ou_commands {
	CHAN_CMD_EXISTS		= 0b000000,
	CHAN_CMD_MASK_PN	= 0b000010,
	CHAN_CMD_MASK_NPN	= 0b000100,
	CHAN_CMD_ASSIGN		= 0b000110,
};
enum chan_in_commands {
	CHAN_CMD_INTSPEC	= 0b000010,
	CHAN_CMD_ALLOC		= 0b000110,
	CHAN_CMD_STATUS		= 0b000100,
};

typedef void (*chan_f_noarg)(chan_t *chan);
typedef void (*chan_f_noarg)(chan_t *chan);
typedef int (*chan_f_cmd)(chan_t *chan, int dir, uint16_t n, uint16_t *r);
typedef int (*chan_f_connect_dev)(chan_t *chan, int devnum, em400_dev_t *dev);

struct chan {
	int num;
	int type;
	em400_dev_t *device[32];
	chan_f_connect_dev connect_dev;	// connects a device to the channel
	chan_f_cmd cmd;					// handles I/O command from the CPU
	chan_f_noarg reset;				// resets channel (asynchronously, from any thread)
	chan_f_noarg shutdown;			// shuts down channel and frees its resources
};

chan_t * chan_create(unsigned num, unsigned type);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
