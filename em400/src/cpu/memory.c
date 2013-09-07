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

uint16_t *mem_seg[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];	// physical segments
int mem_type[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];		// segment types
uint16_t *mem_map[MEM_MAX_NB][MEM_MAX_AB];				// segment mapping
uint16_t *mem_map_mega[MEM_MAX_NB][MEM_MAX_AB];			// MEGA segment mapping (internal)
uint16_t *mem_mega_prom;								// MEGA PROM contents
uint16_t *mem_mega_prom_hidden;							// pointer to segment that PROM covers
int mem_mega_start;

// -----------------------------------------------------------------------
int mem_create_mp(int mp, int type, int segments, int offset)
{
	int seg;

	eprint("  Module %2i: %5s, %2i segments (offset %i)\n", mp, type == MEM_MEGA ? "MEGA" : "Elwro", segments, offset);

	for (seg=offset ; seg<segments; seg++) {
		mem_type[mp][seg] = type;
		mem_seg[mp][seg] = malloc(sizeof(uint16_t) * MEM_SEGMENT_SIZE);
		if (!mem_seg[mp][seg]) {
			return E_ALLOC;
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int mem_init()
{
	int mp;
	int ab;
	int res;

	eprint("Initializing memory\n");

	if ((em400_cfg.mem_elwro < 1) || (em400_cfg.mem_elwro > MEM_MAX_MODULES)
		|| (em400_cfg.mem_mega < 0) || (em400_cfg.mem_elwro > MEM_MAX_MODULES)
		|| (em400_cfg.mem_os < 1) || (em400_cfg.mem_os > 2)
		|| ((em400_cfg.mem_elwro > 1) && (em400_cfg.mem_elwro+em400_cfg.mem_mega > MEM_MAX_MODULES))
	) {
		return E_MEM;
	}

	mem_mega_start = MEM_MAX_MODULES - em400_cfg.mem_mega;

	pthread_spin_init(&mem_spin, 0);

	// create physical segments
	for (mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		if (mp < em400_cfg.mem_elwro) {
			res = mem_create_mp(mp, MEM_ELWRO, MEM_MAX_ELWRO_SEGMENTS, 0);
			if (res != E_OK) {
				return res;
			}
		}
		// mega module 0 may overlap with elwro at position 0, that's why it's not else'd
		if (mp >= mem_mega_start) {
			if (mp > 0) {
				res = mem_create_mp(mp, MEM_MEGA, MEM_MAX_MEGA_SEGMENTS, 0);
			} else { // MEGA in module 0 acts as Elwro, apparently
				res = mem_create_mp(mp, MEM_ELWRO, MEM_MAX_MEGA_SEGMENTS, MEM_MAX_ELWRO_SEGMENTS);
			}
			if (res != E_OK) {
				return res;
			}
		}
	}

	// allocate and show MEGA PROM if MEGA is there
	if (mem_mega_start < MEM_MAX_MODULES) {
		mem_mega_prom = calloc(sizeof(uint16_t), MEM_SEGMENT_SIZE);
		mem_map[0][15] = mem_mega_prom;
	}

	// hardwire elwro segments for OS
	for (ab=0 ; ab<em400_cfg.mem_os ; ab++) {
		pthread_spin_lock(&mem_spin);
		mem_map[0][ab] = mem_seg[0][ab];
		pthread_spin_unlock(&mem_spin);
	}

	// TODO: mega prom load

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_shutdown()
{
	int mp;
	int seg;

	eprint("Shutdown memory\n");

	free(mem_mega_prom);

	for (mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		for (seg=0 ; seg<MEM_MAX_SEGMENTS ; seg++) {
			free(mem_seg[mp][seg]);
		}
	}
}

// -----------------------------------------------------------------------
int mem_cmd(uint16_t n, uint16_t r)
{
	int nb		= (r & 0b0000000000001111);
	int ab		= (r & 0b1111000000000000) >> 12;
	int mp		= (n & 0b0000000000011110) >> 1;
	int seg		= (n & 0b0000000111100000) >> 5;
	int flags	= (n & 0b1111111000000000) >> 9;

	if (nb >= MEM_MAX_NB) {
		LOG(L_MEM, 1, "Add map: NB > MEM_MAX_NB");
		return IO_NO;
	}

	// if MEGA is present and MEM_MEGA_ALLOC is set => command for MEGA
	if ((mem_mega_start < MEM_MAX_MODULES) && ((flags & MEM_MEGA_ALLOC))) {
		return mem_cmd_mega(nb, ab, mp, seg, flags);
	} else {
		return mem_cmd_elwro(nb, ab, mp, seg);
	}
}

// -----------------------------------------------------------------------
int mem_cmd_elwro(int nb, int ab, int mp, int seg)
{
	LOG(L_MEM, 1, "Add map (Elwro): NB = %d, AB = %d, MP = %d, SEG = %d", nb, ab, mp, seg);

	if (!mem_seg[mp][seg]) {
		LOG(L_MEM, 1, "Add map: trying to configure nonexistent segment %i in block %i", seg, mp);
		return IO_NO;
	}

	if ((nb == 0) && (ab < em400_cfg.mem_os)) {
		LOG(L_MEM, 1, "Add map: Can't configure hardwired segment");
		return IO_NO;
	}

	pthread_spin_lock(&mem_spin);
	mem_map[nb][ab] = mem_seg[mp][seg];
	pthread_spin_unlock(&mem_spin);

	return IO_OK;
}

// -----------------------------------------------------------------------
int mem_cmd_mega(int nb, int ab, int mp, int seg, int flags)
{
	LOG(L_MEM, 1, "Add map (MEGA): NB = %d, AB = %d, MP = %d, SEG = %d, FLAGS = (%i) %s%s%s%s%s",
	nb, ab, mp, seg, flags,
	flags & 0b0000001 ? "alloc " : "",
	flags & 0b0000010 ? "free " : "",
	flags & 0b0010000 ? "show" : "",
	flags & 0b0100000 ? "hide " : "",
	flags & 0b1000000 ? "done " : ""
	);

	pthread_spin_lock(&mem_spin);

	// handle MEGA (de-)allocations, but not for hardwired segments
	if ((mp != 0) || ((mp == 0) && (seg >= em400_cfg.mem_os))) {
		if ((flags & MEM_MEGA_FREE)) {
			mem_map[nb][ab] = NULL;
		} else {
			mem_map[nb][ab] = mem_seg[mp][seg];
		}
	}

	// handle PROM show
	if ((flags & MEM_MEGA_PROM_SHOW)) {
		mem_mega_prom_hidden = mem_map[0][15];
		mem_map[0][15] = mem_mega_prom;
	}

	// hndle PROM hide
	if ((flags & MEM_MEGA_PROM_HIDE)) {
		mem_map[0][15] = mem_mega_prom_hidden;
	}

	// TODO: "allocation done" command is ignored, we're always ready to serve

	pthread_spin_unlock(&mem_spin);

	return IO_OK;
}

// -----------------------------------------------------------------------
void mem_reset()
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

	// show MEGA PROM if MEGA is there
	if (mem_mega_start < MEM_MAX_MODULES) {
		mem_map[0][15] = mem_mega_prom;
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
			if (mem_seg[mp][seg]) {
				memset(mem_seg[mp][seg], 0, MEM_SEGMENT_SIZE);
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
