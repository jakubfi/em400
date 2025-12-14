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
#include "cfg.h"

typedef enum chan_types {
	CHAN_CHAR,
	CHAN_MULTIX,
	CHAN_IOTESTER,
	CHAN_TYPE_COUNT
} chan_type_t;

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

typedef void (*chan_f_destroy)(chan_t *chan);
typedef void (*chan_f_reset)(chan_t *chan);
typedef int (*chan_f_cmd)(chan_t *chan, int dir, uint16_t n, uint16_t *r);

struct chan {
	int num;
	chan_type_t type;
	chan_f_cmd cmd;
	chan_f_reset reset;
	chan_f_destroy destroy;
};

chan_t * chan_create(unsigned num, unsigned type, em400_cfg *cfg);
void chan_destroy(chan_t *chan);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
