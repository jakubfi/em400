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
#include "registers.h"

#define MEM_SEGMENT_SIZE 4 * 1024	// segment size (16-bit words)
#define MEM_MAX_MODULES 16			// physical memory modules
#define MEM_MAX_SEGMENTS 16			// max physical segments in a module
#define MEM_MAX_ELWRO_SEGMENTS 8	// physical segments in elwro module
#define MEM_MAX_MEGA_SEGMENTS 16	// physical segments in mega module
#define MEM_MAX_NB 16				// logical blocks
#define MEM_MAX_AB 16				// logical segments in a logical block

enum mem_mega_flags {
	MEM_MEGA_ALLOC		= 0b0000001,
	MEM_MEGA_FREE		= 0b0000010,
	MEM_MEGA_PROM_SHOW	= 0b0010000,
	MEM_MEGA_PROM_HIDE	= 0b0100000,
	MEM_MEGA_ALLOC_DONE	= 0b1000000,
};

extern uint16_t *mem_elwro[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];
extern uint16_t *mem_mega[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];
extern uint16_t *mem_map[MEM_MAX_NB][MEM_MAX_AB];

int mem_init();
void mem_shutdown();
int mem_cmd(uint16_t n, uint16_t r);
int mem_cmd_elwro(int nb, int ab, int mp, int seg);
int mem_cmd_mega(int nb, int ab, int mp, int seg, int flags);
void mem_reset();

#define mem_ptr(nb, addr) (mem_map[nb][addr >> 12] ? mem_map[nb][addr >> 12] + (addr & 0b0000111111111111) : NULL)
uint16_t mem_read(int nb, uint16_t addr, int trace);
uint8_t mem_read_byte(int nb, uint16_t addr, int trace);
void mem_write(int nb, uint16_t addr, uint16_t val, int trace);
void mem_write_byte(int nb, uint16_t addr, uint8_t val, int trace);

void mem_clear();
int mem_load_image(const char* fname, int nb, int start_seg, int len);

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

// vim: tabstop=4 shiftwidth=4 autoindent
