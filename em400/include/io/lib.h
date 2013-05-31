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

#ifndef DRV_LIB_H
#define DRV_LIB_H

#include <inttypes.h>

#include "io/drivers.h"

enum _chan_cmd {
	// channel IN
	CHAN_CMD_EXISTS		= 0b00000000,
	CHAN_CMD_INTSPEC	= 0b00001000,
	CHAN_CMD_STATUS		= 0b00010000,
	CHAN_CMD_ALLOC		= 0b00011000,
	// channel OU
	CHAN_CMD_MASK_PN	= 0b00001000,
	CHAN_CMD_MASK_NPN	= 0b00010000,
	CHAN_CMD_ASSIGN		= 0b00011000
};

void chan_get_int_spec(void *self, uint16_t *r);
int args_to_cfg(struct cfg_arg_t *arg, const char *format, ...);

#endif

// vim: tabstop=4
