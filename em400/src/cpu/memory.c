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
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "cpu/cpu.h"
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

pthread_spinlock_t mem_spin;

// physical memory: modules with segments inside
uint16_t *mem_segment[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];
// physical memory: segment types
int mem_type[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];
// logical mapping from (NB,AB) to physical segment pointer
uint16_t *mem_map[MEM_MAX_NB][MEM_MAX_AB];

// -----------------------------------------------------------------------
int mem_init()
{
	int mp;
	int seg;
	int ab;
	uint16_t *mptr;

	eprint("Initializing memory\n");

	if ((em400_cfg.mem_elwro < 1) || (em400_cfg.mem_elwro > MEM_MAX_MODULES)
		|| (em400_cfg.mem_mega < 0) || (em400_cfg.mem_elwro > MEM_MAX_MODULES)
		|| (em400_cfg.mem_os < 1) || (em400_cfg.mem_os > 2)
		|| ((em400_cfg.mem_elwro > 1) && (em400_cfg.mem_elwro+em400_cfg.mem_mega > MEM_MAX_MODULES))
	) {
		return E_MEM;
	}

	pthread_spin_init(&mem_spin, 0);

	// create physical segments for elwro
	for (mp=0 ; mp<em400_cfg.mem_elwro ; mp++) {
		for (seg=0 ; seg<MEM_MAX_ELWRO_SEGMENTS ; seg++) {
			mptr = malloc(sizeof(uint16_t) * MEM_SEGMENT_SIZE);
			if (!mptr) {
				return E_ALLOC;
			}
			pthread_spin_lock(&mem_spin);
			mem_segment[mp][seg] = mptr;
			pthread_spin_unlock(&mem_spin);
			mem_type[mp][seg] = MEM_ELWRO;
		}
		eprint("  Elwro module %i: added %i segments\n", mp, seg);
	}

	// create physical segments for mega
	for (mp=MEM_MAX_MODULES-1 ; mp>=MEM_MAX_MODULES-em400_cfg.mem_mega ; mp--) {
		for (seg=0 ; seg<MEM_MAX_MEGA_SEGMENTS ; seg++) {
			if (mem_type[mp][seg] != MEM_NONE) {
				mptr = malloc(sizeof(uint16_t) * MEM_SEGMENT_SIZE);
				if (!mptr) {
					return E_ALLOC;
				}
				pthread_spin_lock(&mem_spin);
				mem_segment[mp][seg] = mptr;
				pthread_spin_unlock(&mem_spin);
			}
			mem_type[mp][seg] = MEM_MEGA;
		}
		eprint("  MEGA module %i: added %i segments\n", mp, seg);
	}


	// hardwire segments for OS
	for (ab=0 ; ab<em400_cfg.mem_os ; ab++) {
		pthread_spin_lock(&mem_spin);
		if (mem_segment[0][ab]) {
			mem_map[0][ab] = mem_segment[0][ab];
		}
		pthread_spin_unlock(&mem_spin);
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_shutdown()
{
	int mp;
	int seg;

	eprint("Shutdown memory\n");

	// only if mem_init() was done
	if (mem_map[0][0]) {
		mem_remove_maps();
	}

	// disconnect memory modules
	for (mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		for (seg=0 ; seg<MEM_MAX_SEGMENTS ; seg++) {
			free(mem_segment[mp][seg]);
			mem_segment[mp][seg] = NULL;
		}
	}
}

// -----------------------------------------------------------------------
int mem_add_map(int nb, int ab, int mp, int segment)
{
	LOG(L_MEM, 1, "Add map: NB = %d, AB = %d, MP = %d, SEG = %d", nb, ab, mp, segment);

	if ((nb == 0) && (ab < em400_cfg.mem_os)) {
		LOG(L_MEM, 1, "Add map: Can't configure hardwired segment");
		return IO_NO;
	}

	if (nb >= MEM_MAX_NB) {
		LOG(L_MEM, 1, "Add map: NB > MEM_MAX_NB");
		return IO_NO;
	}

	if (!mem_segment[mp][segment]) {
		LOG(L_MEM, 1, "Add map: No such segment");
		return IO_NO;
	}

	pthread_spin_lock(&mem_spin);
	mem_map[nb][ab] = mem_segment[mp][segment];
	pthread_spin_unlock(&mem_spin);

	return IO_OK;
}

// -----------------------------------------------------------------------
void mem_remove_maps()
{
	int nb;
	int ab;
	int ab_min;

	// remove all memory mappings
	for (nb=0 ; nb<MEM_MAX_NB ; nb++) {
		if (nb == 0) {
			ab_min = em400_cfg.mem_os;
		} else {
			ab_min = 0;
		}
		for (ab=ab_min ; ab<MEM_MAX_AB ; ab++) {
			pthread_spin_lock(&mem_spin);
			mem_map[nb][ab] = NULL;
			pthread_spin_unlock(&mem_spin);
		}
	}
}

// -----------------------------------------------------------------------
// read from any block
uint16_t mem_read(int nb, uint16_t addr, int trace)
{
	uint16_t *ptr;
	uint16_t value;

	ptr = mem_ptr(nb, addr);

	if (ptr) {
		pthread_spin_lock(&mem_spin);
		value = *ptr;
		pthread_spin_unlock(&mem_spin);
#ifdef WITH_DEBUGGER
		// leave trace for debugger to display
		if (trace) {
			LOG(L_MEM, 20, "[%d:%d] -> 0x%04x", nb, addr, value);
			dbg_touch_add(&touch_mem, TOUCH_R, nb, addr, value);
		} else {
			LOG(L_MEM, 100, "[%d:%d] -> 0x%04x", nb, addr, value);
		}
#endif
		return value;
	} else {
		LOG(L_MEM, 1, "[%d:%d] -> ERROR", nb, addr);
		int_set(INT_NO_MEM);
		if (!SR_Q) {
			nRw(R_ALARM, 1);
			cpu_stop = 1;
		}
		return 0xdead;
	}
}

// -----------------------------------------------------------------------
uint8_t mem_read_byte(int nb, uint16_t addr, int trace)
{
	int shift;
	uint16_t addr17;

	shift = 8 * (~addr & 1);

	if (em400_cfg.mod) {
		addr17 = (addr >> 1) | (nR(R_ZC17) << 15);
	} else {
		addr17 = addr >> 1;
	}

	return mem_read(nb, addr17, trace) >> shift;
}

// -----------------------------------------------------------------------
void mem_write_byte(int nb, uint16_t addr, uint8_t val, int trace)
{
	int shift;
	uint16_t addr17;
	uint16_t *ptr;
	uint16_t data;

	shift = 8 * (~addr & 1);

	// TODO: optimize maybe?
	if (em400_cfg.mod) {
		addr17 = (addr >> 1) | (nR(R_ZC17) << 15);
	} else {
		addr17 = addr >> 1;
	}

	ptr = mem_ptr(nb, addr17);

	if (ptr) {
		data = (*ptr & (0b1111111100000000 >> shift)) | ((uint16_t)val << shift);
	} else {
		data = 0xdead;
	}

	mem_write(nb, addr17, data, trace);
}

// -----------------------------------------------------------------------
// write to any block
void mem_write(int nb, uint16_t addr, uint16_t val, int trace)
{
	uint16_t *ptr;
	
	ptr = mem_ptr(nb, addr);

	if (ptr) {
#ifdef WITH_DEBUGGER
		// leave trace for debugger to display
		if (trace) {
			LOG(L_MEM, 20, "[%d:%d] <- 0x%04x", nb, addr, val);
			dbg_touch_add(&touch_mem, TOUCH_W, nb, addr, *ptr);
		} else {
			LOG(L_MEM, 100, "[%d:%d] <- 0x%04x", nb, addr, val);
		}
#endif
		pthread_spin_lock(&mem_spin);
		*ptr = val;
		pthread_spin_unlock(&mem_spin);
	} else {
		LOG(L_MEM, 1, "[%d:%d] <- 0x%04x ERROR", nb, addr, val);
		int_set(INT_NO_MEM);
		if (!SR_Q) {
			nRw(R_ALARM, 1);
			cpu_stop = 1;
		}
	}
}

// -----------------------------------------------------------------------
void mem_clear()
{
	int mp;
	int seg;

	for (mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		for (seg=0 ; seg<MEM_MAX_SEGMENTS ; seg++) {
			pthread_spin_lock(&mem_spin);
			if (mem_segment[mp][seg]) {
				memset(mem_segment[mp][seg], 0, MEM_SEGMENT_SIZE);
			}
			pthread_spin_unlock(&mem_spin);
		}
	}
}

// -----------------------------------------------------------------------
int mem_load_image(const char* fname, int nb, int len)
{
	int ret = E_OK;
	int loaded = 0;

	FILE *f = fopen(fname, "rb");
	if (f == NULL) {
		return E_FILE_OPEN;
	}

	LOG(L_MEM, 1, "Loading memory image: %s -> %d", fname, nb);

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

		if (res <= 0) {
			break;
		}
		loaded += res;

		pthread_spin_lock(&mem_spin);

		// get pointer to segment in a block
		uint16_t *ptr = mem_ptr(nb, chunk*MEM_SEGMENT_SIZE);
		if (!ptr) {
			pthread_spin_unlock(&mem_spin);
			ret = E_MEM_BLOCK_TOO_SMALL;
			break;
		}

		// we swap bytes from big-endian to host-endianness at load time
		for (int i=0 ; i<res ; i++) {
			*(ptr+i) = ntohs(*(buf+i));
		}
		pthread_spin_unlock(&mem_spin);
		chunk++;
	} while ((res == MEM_SEGMENT_SIZE) && ((loaded < len) || (len <= 0)));

	fclose(f);

	if (loaded > 0) {
		return loaded;
	} else {
		return ret;
	}
}


// vim: tabstop=4 shiftwidth=4 autoindent
