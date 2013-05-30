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

// --- set configuration ---

struct cf_sc_pl {
	int dir;
	int used;
	int dev_type;
	int count;
};

struct ll_winch {
	int type;
	int format_protect;
};

struct ll_floppy {
	int type;
	int format_protect;
};

struct cf_sc_ll {
	int proto;
	int pl_id;
	int formatter;
	struct ll_winch *winch;
	struct ll_floppy *floppy;
};

struct cf_sc {
	int pl_desc_count;
	int ll_desc_count;
	uint16_t *retf;
	struct cf_sc_pl *pl;
	struct cf_sc_ll *ll;
};

// --- connect line ---

struct ps_cl_punch_reader {
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

struct ps_cl_puncher {
	int odd_parity;
	int eight_bits;
	int low_to_up;
	int txt_proc;
};

struct ps_cl_monitor {
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

int mx_decode_cf_sc(int addr, struct cf_sc *cf);
void mx_free_cf_sc(struct cf_sc *cf);


#endif

// vim: tabstop=4
