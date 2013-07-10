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

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "cpu/memory.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"
#include "io/io.h"

#include "em400.h"
#include "cfg.h"
#include "errors.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif
#include "debugger/log.h"

// physical memory: modules with segments inside
uint16_t *mem_segment[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];

// logical mapping from (NB,AB) to physical segment pointer
uint16_t *mem_map[MEM_MAX_NB][MEM_MAX_AB];

// -----------------------------------------------------------------------
int mem_init()
{
	eprint("Initializing memory\n");

	if (em400_cfg.mem[0].segments <= 0) {
		return E_MEM_NO_OS_MEM;
	}

	if ((em400_cfg.mem_os < 1) || (em400_cfg.mem_os > 2)) {
		return E_MEM_WRONG_OS_MEM;
	}

	// create configured physical segments
	for (int mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		if (em400_cfg.mem[mp].segments > MEM_MAX_SEGMENTS) {
			return E_MEM_BAD_SEGMENT_COUNT;
		} else {
			int seg;
			for (seg=0 ; seg<em400_cfg.mem[mp].segments ; seg++) {
				mem_segment[mp][seg] = malloc(sizeof(uint16_t) * MEM_SEGMENT_SIZE);
				if (!mem_segment[mp][seg]) {
					return E_MEM_CANNOT_ALLOCATE;
				}
			}
			eprint("  Module %i: added %i segments\n", mp, seg);
		}
	}

	// hardwire segments for OS
	for (int ab=0 ; ab<em400_cfg.mem_os ; ab++) {
		if (mem_segment[0][ab]) {
			mem_map[0][ab] = mem_segment[0][ab];
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_shutdown()
{
	eprint("Shutdown memory\n");
	mem_remove_maps();

	// disconnect memory modules
	for (int mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		for (int seg=0 ; seg<MEM_MAX_SEGMENTS ; seg++) {
			free(mem_segment[mp][seg]);
		}
	}
}

// -----------------------------------------------------------------------
int mem_add_map(int nb, int ab, int mp, int segment)
{
	LOG(D_MEM, 1, "Add map: NB = %d, AB = %d, MP = %d, SEG = %d", nb, ab, mp, segment);

	if ((nb == 0) && (ab<em400_cfg.mem_os)) {
		LOG(D_MEM, 1, "Add map: Can't configure hardwired segment");
		return IO_NO;
	}

	if (nb < MEM_MAX_NB) {
		mem_map[nb][ab] = mem_segment[mp][segment];
		if (!mem_map[nb][ab]) {
			LOG(D_MEM, 1, "Add map: No such segment");
			return IO_NO;
		}
	} else {
		LOG(D_MEM, 1, "Add map: NB > MEM_MAX_NB");
		return IO_NO;
	}
	return IO_OK;
}

// -----------------------------------------------------------------------
void mem_remove_maps()
{
	int min_ab;

	// remove all memory mappings
	for (int nb=0 ; nb<MEM_MAX_NB ; nb++) {
		if (nb == 0) {
			min_ab = em400_cfg.mem_os;
		} else {
			min_ab = 0;
		}
		for (int ab=min_ab ; ab<MEM_MAX_AB ; ab ++) {
			mem_map[nb][ab] = NULL;
		}
	}
}

#ifndef WITH_SPEEDOPT
// -----------------------------------------------------------------------
// low-level memory access (bypassing emulation)
uint16_t * mem_ptr(int nb, uint16_t addr)
{
	int ab = (addr & 0b1111000000000000) >> 12;
	int addr12 = addr & 0b0000111111111111;

	uint16_t *seg_addr = mem_map[nb][ab];

	if (seg_addr) {
		return seg_addr + addr12;
	} else {
		return NULL;
	}
}
#endif

// -----------------------------------------------------------------------
// read from any block
uint16_t mem_read(int nb, uint16_t addr, int trace)
{
	uint16_t *ptr = mem_ptr(nb, addr);
	if (ptr) {
		uint16_t value = *ptr;
#ifdef WITH_DEBUGGER
		// leave trace for debugger to display
		if (trace) {
			LOG(D_MEM, 20, "[%d:%d] -> 0x%04x", nb, addr, value);
			dbg_touch_add(&touch_mem, TOUCH_R, nb, addr, value);
		} else {
			LOG(D_MEM, 40, "[%d:%d] -> 0x%04x", nb, addr, value);
		}
#endif
		return value;
	} else {
#ifdef WITH_DEBUGGER
		LOG(D_MEM, 5, "[%d:%d] -> ERROR", nb, addr);
#endif
		int_set(INT_NO_MEM);
		if (!SR_Q) {
			nRw(R_ALARM, 1);
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_quit = E_QUIT_NO_MEM;
#endif
		}
		return 0xdead;
	}
}

// -----------------------------------------------------------------------
uint8_t mem_read_byte(int nb, uint16_t addr, int trace)
{
	int shift = 8 * (~addr & 1);
	uint16_t addr17;
	if (em400_cfg.cpu.mod_17bit) {
		addr17 = (addr >> 1) | (nR(R_ZC17) << 15);
	} else {
		addr17 = addr >> 1;
	}
	uint16_t data = mem_read(nb, addr17, trace) >> shift;
	return data;
}

// -----------------------------------------------------------------------
void mem_write_byte(int nb, uint16_t addr, uint8_t val, int trace)
{
	int shift = 8 * (~addr & 1);
	uint16_t addr17;
	if (em400_cfg.cpu.mod_17bit) {
		addr17 = (addr >> 1) | (nR(R_ZC17) << 15);
	} else {
		addr17 = addr >> 1;
	}
	uint16_t data = mem_read(nb, addr17, 0) & ((uint16_t)0b1111111100000000>>shift);
	mem_write(nb, addr17, data | (((uint16_t)val) << shift), trace);
}

// -----------------------------------------------------------------------
// write to any block
void mem_write(int nb, uint16_t addr, uint16_t val, int trace)
{
	uint16_t *ptr = mem_ptr(nb, addr);
	if (ptr) {
#ifdef WITH_DEBUGGER
		// leave trace for debugger to display
		if (trace) {
			LOG(D_MEM, 10, "[%d:%d] <- 0x%04x", nb, addr, val);
			dbg_touch_add(&touch_mem, TOUCH_W, nb, addr, *ptr);
		} else {
			LOG(D_MEM, 30, "[%d:%d] <- 0x%04x", nb, addr, val);
		}
#endif
		*ptr = val;
	} else {
		LOG(D_MEM, 5, "[%d:%d] <- 0x%04x ERROR", nb, addr, val);
		int_set(INT_NO_MEM);
		if (!SR_Q) {
			nRw(R_ALARM, 1);
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_quit = E_QUIT_NO_MEM;
#endif
		}
	}
}

// -----------------------------------------------------------------------
void mem_clear()
{
	for (int mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		for (int seg=0 ; seg<MEM_MAX_SEGMENTS ; seg++) {
			if (mem_segment[mp][seg]) {
				for (int addr=0 ; addr<MEM_SEGMENT_SIZE ; addr++) {
					mem_segment[mp][seg][addr] = 0;
				}
			}
		}
	}
}

// -----------------------------------------------------------------------
int mem_load_image(const char* fname, int nb)
{
	int ret = E_OK;

	FILE *f = fopen(fname, "rb");
	if (f == NULL) {
		return E_FILE_OPEN;
	}

	LOG(D_MEM, 1, "Loading memory image: %s -> %d", fname, nb);

	uint16_t buf[MEM_SEGMENT_SIZE];
	int res;
	uint16_t chunk = 0;
	do {
		// read chunk of data
		res = fread((void*)buf, sizeof(uint16_t), MEM_SEGMENT_SIZE, f);
		if (ferror(f)) {
			ret = E_FILE_OPERATION;
			break;
		}

		if (res == 0) {
			break;
		}

		// get pointer to segment in a block
		uint16_t *ptr = mem_ptr(nb, chunk*MEM_SEGMENT_SIZE);
		if (!ptr) {
			ret = E_MEM_BLOCK_TOO_SMALL;
			break;
		}

		// we swap bytes from big-endian to host-endianness at load time
		for (int i=0 ; i<res ; i++) {
			*(ptr+i) = ntohs(*(buf+i));
		}
		chunk++;
	} while (res == MEM_SEGMENT_SIZE);

	fclose(f);

	return ret;
}


// vim: tabstop=4 shiftwidth=4 autoindent
