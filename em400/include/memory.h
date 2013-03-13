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

#define MEM_MAX_MODULES 16
#define MEM_MAX_SEGMENTS 16
#define MEM_SEGMENT_SIZE 4 * 1024
#define MEM_MAX_NB 16
#define MEM_MAX_AB 16

extern int mem_conf[MEM_MAX_MODULES];
extern uint16_t *mem_segment[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];
extern uint16_t *mem_map[MEM_MAX_NB][MEM_MAX_AB];

int mem_init();
void mem_shutdown();
int mem_add_map(int nb, int ab, int mp, int segment);
void mem_remove_maps();

#define mem_ptr(nb, addr) (mem_map[nb][((addr) & 0b1111000000000000) >> 12] ? mem_map[nb][((addr) & 0b1111000000000000) >> 12] + ((addr) & 0b0000111111111111) : NULL)
uint16_t mem_read(int nb, uint16_t addr, int trace);
uint8_t mem_read_byte(int nb, uint16_t addr, int trace);
void mem_write(int nb, uint16_t addr, uint16_t val, int trace);
void mem_write_byte(int nb, uint16_t addr, uint8_t val, int trace);

void mem_clear();
int mem_load_image(const char* fname, int nb);

// memory access macros
#define MEM(a)			mem_read(SR_Q*SR_NB, a, 1)
#define nMEM(a)			mem_read(SR_Q*SR_NB, a, 0)
#define MEMw(a, x)		mem_write(SR_Q*SR_NB, a, x, 1)
#define nMEMw(a, x)		mem_write(SR_Q*SR_NB, a, x, 0)

#define MEMNB(a)		mem_read(SR_NB, a, 1)
#define nMEMNB(a)		mem_read(SR_NB, a, 0)
#define MEMNBw(a, x)	mem_write(SR_NB, a, x, 1)
#define nMEMNBw(a, x)	mem_write(SR_NB, a, x, 0)

#define MEMNBb(a)		mem_read_byte(SR_NB, a, 1)
#define MEMNBwb(a, x)	mem_write_byte(SR_NB, a, x, 1)

#define MEMB(b, a)		mem_read(b, a, 1)
#define nMEMB(b, a)		mem_read(b, a, 0)
#define MEMBw(b, a, x)	mem_write(b, a, x, 1)
#define nMEMBw(b, a, x)	mem_write(b, a, x, 0)

#endif

// vim: tabstop=4
