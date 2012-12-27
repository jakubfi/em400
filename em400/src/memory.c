//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#include "errors.h"
#include "memory.h"
#include "registers.h"
#include "interrupts.h"
#include "io.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif

pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;

// memory configuration provided by the user: number of segments in a module
int mem_conf[MEM_MAX_MODULES] = { 2, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// physical memory: modules with segments inside
uint16_t *mem_segment[MEM_MAX_MODULES][MEM_MAX_SEGMENTS] = { {NULL} };

// logical mapping from (NB,AB) to physical segment pointer
uint16_t *mem_map[MEM_MAX_NB][MEM_MAX_AB] = { {NULL} };

// -----------------------------------------------------------------------
int mem_init()
{
	if (mem_conf[0] <= 0) {
		return E_MEM_NO_OS_MEM;
	}

	// create configured physical segments
	for (int mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		if (mem_conf[mp] > MEM_MAX_SEGMENTS) {
			return E_MEM_BAD_SEGMENT_COUNT;
		} else {
			for (int seg=0 ; seg<mem_conf[mp] ; seg++) {
				mem_segment[mp][seg] = malloc(2 * MEM_SEGMENT_SIZE);
				if (!mem_segment[mp][seg]) {
					return E_MEM_CANNOT_ALLOCATE;
				}
			}
		}
	}

	// hardwire segments for OS
	// TODO: MEM_MAX_SEGMENTS for OS block may be too much... (?)
	for (int ab=0 ; ab<MEM_MAX_SEGMENTS ; ab++) {
		if (mem_segment[0][ab]) {
			mem_map[0][ab] = mem_segment[0][ab];
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_shutdown()
{
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
	if (nb < MEM_MAX_NB) {
		mem_map[nb][ab] = mem_segment[mp][segment];
		if (!mem_map[nb][ab]) {
			return IO_NO;
		}
	} else {
		return IO_NO;
	}
	return IO_OK;
}

// -----------------------------------------------------------------------
void mem_remove_maps()
{
	// remove all memory mappings
	for (int nb=1 ; nb<MEM_MAX_NB ; nb++) {
		for (int ab=0 ; ab<MEM_MAX_AB ; ab ++) {
			mem_map[nb][ab] = NULL;
		}
	}
}

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

// -----------------------------------------------------------------------
// read from any block
uint16_t mem_read(int nb, uint16_t addr, int trace)
{
	uint16_t *ptr = mem_ptr(nb, addr);
	if (ptr) {
#ifdef WITH_DEBUGGER
		pthread_mutex_lock(&mem_mutex);
		// leave trace for debugger to display
		if (trace) {
			if (mem_actr_max == -1) {
				mem_act_block = nb;
				mem_actr_min = addr;
			}
			mem_actr_max = addr;
		}
#endif
		uint16_t value = *ptr;
		pthread_mutex_unlock(&mem_mutex);
		return value;
	} else {
		if (SR_Q) {
			int_set(INT_NO_MEM);
		} else {
			// TODO: ALARM
		}
		return 0xdead;
	}
}

// -----------------------------------------------------------------------
// write to any block
void mem_write(int nb, uint16_t addr, uint16_t val, int trace)
{
	uint16_t *ptr = mem_ptr(nb, addr);
	if (ptr) {
		pthread_mutex_lock(&mem_mutex);
#ifdef WITH_DEBUGGER
		// leave trace for debugger to display
		if (trace) {
			if (mem_actw_max == -1) {
				mem_act_block = nb;
				mem_actw_min = addr;
			}
			mem_actw_max = addr;
		}
#endif
		*ptr = val;
		pthread_mutex_unlock(&mem_mutex);
	} else {
		if (SR_Q) {
			int_set(INT_NO_MEM);
		} else {
			// TODO: ALARM
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
	uint16_t *ptr;

	FILE *f = fopen(fname, "rb");
	if (f == NULL) {
		return E_FILE_OPEN;
	}

	int res = 1;
	int chunk = 0;
	while (res > 0) {
		// get pointer to segment in a block
		ptr = mem_ptr(nb, chunk*MEM_SEGMENT_SIZE);
		if (!ptr) {
			return E_MEM_BLOCK_TOO_SMALL;
		}

		// read chunk of data
		res = fread((void*)ptr, sizeof(*ptr), MEM_SEGMENT_SIZE, f);
		if (ferror(f)) {
			fclose(f);
			return E_FILE_OPERATION;
		}

		// we swap bytes from big-endian to host-endianness at load time
		uint16_t *i = ptr;
		while (i < ptr + res) {
			*i = ntohs(*i);
			i++;
		}
		chunk++;
	}

	fclose(f);

	return E_OK;
}


// vim: tabstop=4
