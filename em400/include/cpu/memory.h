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
#include <pthread.h>
#include "cpu.h"

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

extern pthread_spinlock_t mem_spin;
extern uint16_t *mem_elwro[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];
extern uint16_t *mem_map[MEM_MAX_NB][MEM_MAX_AB];
extern uint16_t *mem_mega[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];
extern uint16_t *mem_mega_prom;

#define mem_ptr(nb, addr) (mem_map[nb][(addr) >> 12] ? mem_map[nb][(addr) >> 12] + ((addr) & 0b0000111111111111) : NULL)

int mem_init();
void mem_shutdown();
int mem_cmd(uint16_t n, uint16_t r);
int mem_cmd_elwro(int nb, int ab, int mp, int seg);
int mem_cmd_mega(int nb, int ab, int mp, int seg, int flags);
void mem_reset();

int mem_get(int nb, uint16_t addr, uint16_t *data);
int mem_put(int nb, uint16_t addr, uint16_t data);
int mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count);
int mem_mput(int nb, uint16_t saddr, uint16_t *src, int count);
int mem_cpu_get(int nb, uint16_t addr, uint16_t *data);
int mem_cpu_put(int nb, uint16_t addr, uint16_t data);
int mem_cpu_mget(int nb, uint16_t saddr, uint16_t *dest, int count);
int mem_cpu_mput(int nb, uint16_t saddr, uint16_t *src, int count);
int mem_get_byte(int nb, uint16_t addr, uint8_t *data);
int mem_put_byte(int nb, uint16_t addr, uint8_t data);

void mem_clear();
int mem_load_image(const char* fname, int nb, int start_seg, int len);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
