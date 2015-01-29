//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_PROTO_WINCH_H
#define MX_PROTO_WINCH_H

#include "io/mx_proto.h"

struct mx_proto_winch {
	struct mx_proto base;

	int heads;
	int fprotect;
	int newmx;
};

// old multix Winchester types
enum mx_winch_types {
	MX_WINCH_BASF = 0,
	MX_WINCH_NEC = 1,
	MX_WINCH_MAX
};

// Winchester operations
enum mx_winch_ops {
	MX_WINCH_OP_FORMAT_SPARE	= 0,
	MX_WINCH_OP_FORMAT			= 1,
	MX_WINCH_OP_READ			= 2,
	MX_WINCH_OP_WRITE			= 3,
	MX_WINCH_OP_PARK			= 5, // needs to be investigated, doesn't fit in 2 bits
	MX_WINCH_OP_MAX
};

// Winchester return field (state word) flags
enum mx_winch_status {
	MX_WS_EOF			= 0b1000000000000000, // found end of transmission mark
	MX_WS_NOT_READY		= 0b0100000000000000, // disk is not ready (no power, wrong speed, ...)
	MX_WS_ERR_WRITE		= 0b0010000000000000, // cannot write (>1 head selected, no power, ...)
	MX_WS_HEADS_MOVING	= 0b0001000000000000, // drive not ready, heads are still moving
	MX_WS_SPARE_OVLF	= 0b0000100000000000, // spare area full (during MX_WINCH_FORMAT)
	MX_WS_SPARE_MAP_ERR	= 0b0000010000000000, // error in spare sectors map
	MX_WS_ERR			= 0b0000000100000000, // error during processing operation, see below:
	MX_WS_BAD_SECT		= 0b0000000010000000, // sector is marked as bad
	MX_WS_BAD_CRC		= 0b0000000001000000, // data CRC error
	MX_WS_NO_SECTOR		= 0b0000000000010000, // sector not found (disk address field incorrect)
	MX_WS_REJECTED		= 0b0000000000001000, // command rejected ('cause disk is not ready)
	MX_WS_ERR_T0		= 0b0000000000000010, // cannot position heads on track 0
	MX_WS_ERR_A1		= 0b0000000000000001  // cannot find MFM A1 data marker
};

extern int mx_proto_winch_init(uint16_t *args, struct mx_line *lline);
extern void mx_proto_winch_close(struct mx_proto *proto);
extern int mx_proto_winch_attach(struct mx_line *line, uint16_t *arg);
extern void mx_proto_winch_detach(struct mx_line *line);
extern void mx_proto_winch_transmit(struct mx_proto *proto, uint16_t *arg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
