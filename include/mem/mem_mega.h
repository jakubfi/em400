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

#ifndef MEM_MEGA_H
#define MEM_MEGA_H

#include <inttypes.h>
#include <pthread.h>

#include "mem/mem.h"

#define MEM_MAX_MEGA_SEGMENTS 16	// physical segments in mega module

enum mem_mega_flags {
	MEM_MEGA_ALLOC		= 0b0000001,
	MEM_MEGA_FREE		= 0b0000010,
	MEM_MEGA_PROM_SHOW	= 0b0010000,
	MEM_MEGA_PROM_HIDE	= 0b0100000,
	MEM_MEGA_ALLOC_DONE	= 0b1000000,
};

extern uint16_t *mem_mega[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];
extern uint16_t *mem_mega_prom;	// this needs to be visible, we check in mem_put() if we can write to segment

int mem_mega_init(int modc, char *prom_image);
void mem_mega_shutdown();
void mem_mega_reset();
void mem_mega_seg_set(int nb, int ab, struct mem_slot_t *slot);
int mem_mega_cmd(int nb, int ab, int mp, int seg, int flags);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
