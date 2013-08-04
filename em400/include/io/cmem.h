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

#ifndef DRV_CMEM_H
#define DRV_CMEM_H

#include <inttypes.h>

#include "cfg.h"
#include "io.h"

// cmem control field - modes of operation
enum cmem_trans_modes_e {
	READ_DATA   = 0b00,
	READ_ADDR   = 0b01,
	WRITE_DATA  = 0b10,
	WRITE_ADDR  = 0b11
};

// --- transmit ----------------------------------------------------------

struct cmem_cf_t {
	int cf_len;
	int cpu;
	int nb;
	int oper;
	int len;
	int ign_wrprotect;
	int ign_defects;
	int ign_key;
	int ign_eof;
	int cyl;
	int platter;
	int head;
	int sector;
	int key;
	uint16_t addr;
};


struct chan_proto_t * cmem_create(struct cfg_unit_t *units);
void cmem_shutdown(struct chan_proto_t *chan);
void cmem_reset(struct chan_proto_t *chan);
int cmem_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg);

int cmem_decode_cf_t(int addr, struct cmem_cf_t *cf);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
