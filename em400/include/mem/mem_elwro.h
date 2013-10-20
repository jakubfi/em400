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

#ifndef MEM_ELWRO_H
#define MEM_ELWRO_H

#include <inttypes.h>

#include "mem/mem.h"

#define MEM_MAX_ELWRO_SEGMENTS 8

extern uint16_t *mem_elwro[MEM_MAX_MODULES][MEM_MAX_ELWRO_SEGMENTS];

int mem_elwro_init(int modc, int osc);
void mem_elwro_shutdown();
void mem_elwro_reset();
void mem_elwro_clear();
void mem_elwro_seg_set(int nb, int ab, struct mem_slot_t *slot);
int mem_elwro_cmd(int nb, int ab, int mp, int seg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
