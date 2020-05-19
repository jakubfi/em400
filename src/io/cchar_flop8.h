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

#ifndef CCHAR_FLOP8_H
#define CCHAR_FLOP8_H

#include <stdio.h>
#include "io/cchar.h"
#include "cfg.h"

struct cchar_unit_flop8_t {
	struct cchar_unit_proto_t proto;
	char *image[4];
	FILE *f[4];
	int track, sector, byte, disk;
};

// commands
enum cchar_flop8_cmd_e {
	// OU
	CCHAR_FLOP8_CMD_RESET		= 0b100000, // reset
	CCHAR_FLOP8_CMD_DISCONNECT	= 0b101000, // disconnect device (soft reset)
	CCHAR_FLOP8_CMD_WRITE		= 0b110000, // write
	CCHAR_FLOP8_CMD_SEEK		= 0b111010, // seek
	// IN
	CCHAR_FLOP8_CMD_SPU			= 0b100000, // check device
	CCHAR_FLOP8_CMD_READ		= 0b101000, // read
};

// interrupts
enum char_flop_int_e {
	CCHAR_FLOP8_INT_OUTDATED	= 0, // interrupt out of date
	CCHAR_FLOP8_INT_READY	= 1, // ready again
	CCHAR_FLOP8_INT_OPR_CALL	= 3, // operator call
	CCHAR_FLOP8_INT_TOO_SLOW	= 5, // transmission too slow
};

struct cchar_unit_proto_t * cchar_flop8_create(em400_cfg *cfg, int ch_num, int dev_num);
void cchar_flop8_shutdown(struct cchar_unit_proto_t *unit);
void cchar_flop8_reset(struct cchar_unit_proto_t *unit);
int cchar_flop8_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
