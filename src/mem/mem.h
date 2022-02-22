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

#ifndef MEMORY_H
#define MEMORY_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "cfg.h"

#define MEM_SEGMENT_SIZE 4 * 1024	// segment size (16-bit words)
#define MEM_MAX_MODULES 16			// physical memory modules
#define MEM_MAX_SEGMENTS 16			// max physical segments in a module
#define MEM_MAX_NB 16				// logical blocks
#define MEM_MAX_AB 16				// logical segments in a logical block

extern uint16_t * mem_map[MEM_MAX_NB][MEM_MAX_AB];

int mem_init(em400_cfg *cfg);
void mem_shutdown();
int mem_cmd(uint16_t n, uint16_t r);
void mem_reset();
bool mem_mega_boot();

bool mem_read_1(int nb, uint16_t addr, uint16_t *data);
bool mem_write_1(int nb, uint16_t addr, uint16_t data);
bool mem_read_n(int nb, uint16_t saddr, uint16_t *dest, int count);
bool mem_write_n(int nb, uint16_t saddr, uint16_t *src, int count);

uint16_t mem_get_map(int seg);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
