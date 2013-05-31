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

enum mx_t_winch_oper_e {
	MX_WINCH_FORMAT_SPARE = 0,
	MX_WINCH_FORMAT = 1,
	MX_WINCH_READ = 2,
	MX_WINCH_WRITE = 3,
	MX_WINCH_PARK = 666 // needs to be investigated, doesn't fit in 2 bits
};

// --- set configuration ---

struct mx_cf_sc_pl {
	int dir;
	int used;
	int dev_type;
	int count;
};

struct mx_ll_winch {
	int type;
	int format_protect;
};

struct mx_ll_floppy {
	int type;
	int format_protect;
};

struct mx_cf_sc_ll {
	int proto;
	int pl_id;
	int formatter;
	struct mx_ll_winch *winch;
	struct mx_ll_floppy *floppy;
};

struct mx_cf_sc {
	int pl_desc_count;
	int ll_desc_count;
	uint16_t *retf;
	struct mx_cf_sc_pl *pl;
	struct mx_cf_sc_ll *ll;
};

// --- connect line ---

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

struct mx_cf_cl_puncher {
	int odd_parity;
	int eight_bits;
	int low_to_up;
	int txt_proc;
};

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

// --- get line status ---

// tylko parametry zwracane

// --- transmit ---

struct mx_cf_winch_format {
	uint16_t sector_map;
	int start_sector;
};

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

struct mx_cf_winch_park {
	int cylinder;
};

struct mx_cf_winch_t {
	int oper;
	struct mx_cf_winch_format *format;
	struct mx_cf_winch_transmit *transmit;
	struct mx_cf_winch_park *park;	
	uint16_t *ret_len;
	uint16_t *ret_status;
};

// -----------------------------------------------------------------

int mx_decode_cf_sc(int addr, struct mx_cf_sc *cf);
int mx_decode_cf_winch_t(int addr, struct mx_cf_winch_t *cf);

void mx_free_cf_sc(struct mx_cf_sc *cf);
void mx_free_cf_winch_t(struct mx_cf_winch_t *cf);


#endif

// vim: tabstop=4
