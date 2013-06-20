//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef _MULTIX_WINCH_H_
#define _MULTIX_WINCH_H_

#include <inttypes.h>

#include "io/io.h"
#include "io/devices/winch.h"

struct mx_unit_winch_t {
	struct unit_proto_t proto;
	struct winchester_t *winchester;
};

// Winchester operations

enum mx_winch_t_oper_e {
	MX_WINCH_FORMAT_SPARE = 0,
	MX_WINCH_FORMAT = 1,
	MX_WINCH_READ = 2,
	MX_WINCH_WRITE = 3,
	MX_WINCH_PARK = 5 // needs to be investigated, doesn't fit in 2 bits
};

// Winchester return field (state word) flags

enum mx_winch_t_status {
	MX_WS_EOF			= 0b1000000000000000,   // found end of transmission mark
	MX_WS_NOT_READY		= 0b0100000000000000,   // disk is not ready (no power, wrong speed, ...)
	MX_WS_ERR_WRITE		= 0b0010000000000000,   // cannot write (>1 head selected, no power, ...)
	MX_WS_HEADS_MOVING	= 0b0001000000000000,   // drive not ready, heads are still moving
	MX_WS_SPARE_OVLF	= 0b0000100000000000,   // spare area full (during MX_WINCH_FORMAT)
	MX_WS_SPARE_MAP_ERR	= 0b0000010000000000,   // error in spare sectors map
	MX_WS_ERR			= 0b0000000100000000,   // error during processing operation, see below:
	MX_WS_BAD_SECT		= 0b0000000010000000,   // sector is marked as bad
	MX_WS_BAD_CRC		= 0b0000000001000000,   // data CRC error
	MX_WS_NO_SECTOR		= 0b0000000000010000,   // sector not found (disk address field incorrect)
	MX_WS_REJECTED		= 0b0000000000001000,   // command rejected ('cause disk is not ready)
	MX_WS_ERR_T0		= 0b0000000000000010,   // cannot position heads on track 0
	MX_WS_ERR_A1		= 0b0000000000000001	// cannot find MFM A1 data marker
};

// transmit: format track and (optionally) move sectors to spare area
struct mx_winch_cf_format {
	uint16_t sector_map;
	int start_sector;
};

// transmit: read/write
struct mx_winch_cf_transmit {
	int ign_crc;
	int sector_fill;
	int watch_eof;
	int cpu;
	int nb;
	int addr;
	uint16_t len;
	int sector;
};

// transmit: move heads (park)
struct mx_winch_cf_park {
	int cylinder;
};

// transmit
struct mx_winch_cf_t {
	int oper;
	struct mx_winch_cf_format *format;
	struct mx_winch_cf_transmit *transmit;
	struct mx_winch_cf_park *park;
	uint16_t *ret_len;
	uint16_t *ret_status;
};

struct unit_proto_t * mx_winch_create(struct cfg_arg_t *args);
struct mx_unit_winch_t * mx_winch_create_internal(struct winchester_t *winchester);
void mx_winch_connect(struct mx_unit_winch_t *unit, struct winchester_t *winchester);
void mx_winch_disconnect(struct mx_unit_winch_t *unit);
void mx_winch_shutdown(struct unit_proto_t *unit);
void mx_winch_reset(struct unit_proto_t *unit);
int mx_winch_cmd(struct unit_proto_t *unit, int dir, uint16_t n, uint16_t *r);
struct mx_winch_cf_t * mx_winch_cf_t_decode(int addr);
void mx_winch_cf_t_free(struct mx_winch_cf_t *cf);



#endif

// vim: tabstop=4 shiftwidth=4 autoindent
