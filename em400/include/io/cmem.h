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

#ifndef CMEM_H
#define CMEM_H

#include <inttypes.h>

#include "cfg.h"
#include "io.h"

// interrupts
enum cmem_int_e {
	CMEM_INT_TOO_SLOW	= 0b001,
	CMEM_INT_COMPARE	= 0b011,
	CMEM_INT_NOMEM		= 0b010,
	CMEM_INT_PARITY		= 0b100,
};

struct chan_proto_t * cmem_create(struct cfg_unit_t *units);
void cmem_shutdown(struct chan_proto_t *chan);
void cmem_reset(struct chan_proto_t *chan);
int cmem_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
