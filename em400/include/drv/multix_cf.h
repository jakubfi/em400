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

#ifndef _MULTIX_CF_H_
#define _MULTIX_CF_H_

#include <inttypes.h>

// Winchester operations

enum mx_t_winch_oper_e {
	MX_WINCH_FORMAT_SPARE = 0,
	MX_WINCH_FORMAT = 1,
	MX_WINCH_READ = 2,
	MX_WINCH_WRITE = 3,
	MX_WINCH_PARK = 5 // needs to be investigated, doesn't fit in 2 bits
};

// Winchester return field (state word) flags

enum mx_t_winch_status {
	MX_WS_EOF			= 0b1000000000000000,	// found end of transmission mark
	MX_WS_NOT_READY		= 0b0100000000000000,	// disk is not ready (no power, wrong speed, ...)
	MX_WS_ERR_WRITE		= 0b0010000000000000,	// cannot write (>1 head selected, no power, ...)
	MX_WS_HEADS_MOVING	= 0b0001000000000000,	// drive not ready, heads are still moving
	MX_WS_SPARE_OVLF	= 0b0000100000000000,	// spare area full (during MX_WINCH_FORMAT)
	MX_WS_SPARE_MAP_ERR	= 0b0000010000000000,	// error in spare sectors map
	MX_WS_ERR			= 0b0000000100000000,	// error during processing operation, see below:
	MX_WS_BAD_SECT		= 0b0000000010000000,	// sector is marked as bad
	MX_WS_BAD_CRC		= 0b0000000001000000,	// data CRC error
	MX_WS_NO_SECTOR		= 0b0000000000010000,	// sector not found (disk address field incorrect)
	MX_WS_REJECTED		= 0b0000000000001000,	// command rejected ('cause disk is not ready)
	MX_WS_ERR_T0		= 0b0000000000000010,	// cannot position heads on track 0
	MX_WS_ERR_A1		= 0b0000000000000001	// cannot find MFM A1 data marker
};

// --- set configuration -------------------------------------------------

// set configuration - physical line
struct mx_cf_sc_pl {
	int dir;
	int used;
	int dev_type;
	int count;
};

// set configuration - logical line - winchester
struct mx_ll_winch {
	int type;
	int format_protect;
};

// set configuration - logical line - floppy
struct mx_ll_floppy {
	int type;
	int format_protect;
};

// set configuration - logical line
struct mx_cf_sc_ll {
	int proto;
	int pl_id;
	int formatter;
	struct mx_ll_winch *winch;
	struct mx_ll_floppy *floppy;
};

// set configuration
struct mx_cf_sc {
	int pl_desc_count;
	int ll_desc_count;
	uint16_t *retf;
	struct mx_cf_sc_pl *pl;
	struct mx_cf_sc_ll *ll;
};

// --- connect line ------------------------------------------------------

// connect line - punch tape reader
struct mx_cf_cl_punch_reader {
	int watch_eot;
	int no_parity;
	int odd_parity;
	int eight_bits;
	int bs_can;
	int watch_oprq;
	int eot_code;
	int oprq_code;
	int txt_proc;
};

// connec line - tape puncher
struct mx_cf_cl_puncher {
	int odd_parity;
	int eight_bits;
	int low_to_up;
	int txt_proc;
};

// connect line - terminal (monitor)
struct mx_cf_cl_monitor {
	int watch_eot;
	int no_parity;
	int odd_parity;
	int eight_bits;
	int xon_xoff;
	int bs_can;
	int low_to_up;
	int watch_oprq;
	int eot_code;
	int oprq_code;
	int txt_proc;
	int txt_proc_params;
};

// --- get line status ---------------------------------------------------

// only return field

// --- transmit ----------------------------------------------------------

// transmit - winchester - format track and (optionally) move sectors to spare area
struct mx_cf_winch_format {
	uint16_t sector_map;
	int start_sector;
};

// transmit - winchester - read/write
struct mx_cf_winch_transmit {
	int ign_crc;
	int sector_fill;
	int watch_eof;
	int cpu;
	int nb;
	int addr;
	uint16_t len;
	int sector;
};

// transmit - winchester - move heads (park)
struct mx_cf_winch_park {
	int cylinder;
};

// transmit - winchester
struct mx_cf_winch_t {
	int oper;
	struct mx_cf_winch_format *format;
	struct mx_cf_winch_transmit *transmit;
	struct mx_cf_winch_park *park;	
	uint16_t *ret_len;
	uint16_t *ret_status;
};

// -----------------------------------------------------------------------

int mx_decode_cf_sc(int addr, struct mx_cf_sc *cf);
int mx_decode_cf_winch_t(int addr, struct mx_cf_winch_t *cf);

void mx_free_cf_sc(struct mx_cf_sc *cf);
void mx_free_cf_winch_t(struct mx_cf_winch_t *cf);


#endif

// vim: tabstop=4
