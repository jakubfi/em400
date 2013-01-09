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

#ifndef DRV_U9425_H
#define DRV_U9425_H

#include <inttypes.h>

#include "io.h"

enum _u9425_cmd {
	// OU
	U9425_CMD_ZER		= 0b10000000,
	U9425_CMD_OTR		= 0b11000000,
	U9425_CMD_NTR		= 0b11010000,
	U9425_CMD_SEEK		= 0b11100000,
	U9425_CMD_RTZ		= 0b01000000,
	U9425_CMD_SELOFF	= 0b10100000,
	U9425_CMD_RES		= 0b10010000,
	// IN
	U9425_CMD_TEST		= 0b10000000,
	U9425_CMD_TSR		= 0b01000000,
	U9425_CMD_TCH		= 0b10010000
};

int drv_u9425_init(struct unit_t *u, int cfgc, char **cfgv);
void drv_u9425_shutdown(struct unit_t *u);
void drv_u9425_reset(struct unit_t *u);
int drv_u9425_cmd(struct unit_t *u, int dir, int cmd, uint16_t *r);

#endif

// vim: tabstop=4
