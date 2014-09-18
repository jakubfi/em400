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
#include <stdio.h>
#include "cpu/cpu.h"

#define MEM_SEGMENT_SIZE 4 * 1024	// segment size (16-bit words)
#define MEM_MAX_MODULES 16			// physical memory modules
#define MEM_MAX_SEGMENTS 16			// max physical segments in a module
#define MEM_MAX_NB 16				// logical blocks
#define MEM_MAX_AB 16				// logical segments in a logical block

struct mem_slot_t {
	uint16_t *seg;
};

extern struct mem_slot_t mem_map[MEM_MAX_NB][MEM_MAX_AB];

int mem_init();
void mem_shutdown();
int mem_cmd(uint16_t n, uint16_t r);
void mem_reset();

int mem_get(int nb, uint16_t addr, uint16_t *data);
int mem_put(int nb, uint16_t addr, uint16_t data);
int mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count);
int mem_mput(int nb, uint16_t saddr, uint16_t *src, int count);
int mem_cpu_get(int nb, uint16_t addr, uint16_t *data);
int mem_cpu_put(int nb, uint16_t addr, uint16_t data);
int mem_cpu_mget(int nb, uint16_t saddr, uint16_t *dest, int count);
int mem_cpu_mput(int nb, uint16_t saddr, uint16_t *src, int count);
int mem_get_byte(int nb, uint32_t addr, uint8_t *data);
int mem_put_byte(int nb, uint32_t addr, uint8_t data);

void mem_clear();
int mem_seg_load(FILE *f, uint16_t *ptr);
int mem_load(const char* fname, int nb, int start_ab, int len);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
