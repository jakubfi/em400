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

#include "atomic.h"
#include "mem/mem_elwro.h"
#include "mem/mem_mega.h"
#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "io/io.h"

#include "em400.h"
#include "cfg.h"
#include "errors.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif

#include "emulog.h"

struct mem_slot_t mem_map[MEM_MAX_NB][MEM_MAX_AB];	// final (as seen by emulation) logical->physical segment mapping

static int mega_modules = 0;
static int mega_boot = 0;
static int nomem_stop = 0;

// -----------------------------------------------------------------------
inline uint16_t *mem_ptr(int nb, uint16_t addr)
{
	uint16_t *seg_ptr = mem_map[nb][addr >> 12].seg;
	if (seg_ptr) {
		return seg_ptr + (addr & 0b0000111111111111);
	}
	return NULL;
}

// -----------------------------------------------------------------------
void mem_update_map()
{
	int nb, ab;

	for (nb=0 ; nb<MEM_MAX_NB ; nb++) {
		for (ab=0 ; ab<MEM_MAX_AB ; ab++) {
			mem_elwro_seg_set(nb, ab, &mem_map[nb][ab]);
			if (!mem_map[nb][ab].seg) {
				mem_mega_seg_set(nb, ab, &mem_map[nb][ab]);
			}
			atom_fence();
		}
	}
}

// -----------------------------------------------------------------------
int mem_init(struct cfg_em400_t *cfg)
{
	int res;

	eprint("Initializing memory (Elwro: %d modules, MEGA: %d modules)\n", cfg->mem_elwro, cfg->mem_mega);

	if (cfg->mem_elwro + cfg->mem_mega > MEM_MAX_MODULES+1) {
		return E_MEM;
	}

	res = mem_elwro_init(cfg->mem_elwro, cfg->mem_os);
	if (res != E_OK) {
		return res;
	}

	res = mem_mega_init(cfg->mem_mega, cfg->mem_mega_prom);
	if (res != E_OK) {
		return res;
	}

	mem_update_map();

	mega_modules = cfg->mem_mega;
	mega_boot = cfg->mem_mega_boot;
	nomem_stop = cfg->cpu_stop_on_nomem;

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_shutdown()
{
	eprint("Shutdown memory\n");

	mem_mega_shutdown();
	mem_elwro_shutdown();
}

// -----------------------------------------------------------------------
int mem_cmd(uint16_t n, uint16_t r)
{
	int res;
	int nb		= (r & 0b0000000000001111);
	int ab		= (r & 0b1111000000000000) >> 12;
	int mp		= (n & 0b0000000000011110) >> 1;
	int seg		= (n & 0b0000000111100000) >> 5;
	int flags	= (n & 0b1111111000000000) >> 9;

	// if MEGA is present and MEM_MEGA_ALLOC is set => command for MEGA
	if ((mega_modules > 0) && (flags & MEM_MEGA_ALLOC)) {
		res = mem_mega_cmd(nb, ab, mp, seg, flags);
	// Elwro otherwise (but mask segment number to 3 bits)
	} else {
		res = mem_elwro_cmd(nb, ab, mp, seg & 0b0111);
	}
	if (res == IO_OK) {
		mem_update_map();
	}
	return res;
}

// -----------------------------------------------------------------------
void mem_reset()
{
	mem_elwro_reset();
	mem_mega_reset();
	mem_update_map();
}

// -----------------------------------------------------------------------
int mem_mega_boot()
{
	if (mega_boot && mem_mega_prom && (mem_map[0][15].seg == mem_mega_prom)) {
		return 1;
	} else {
		return 0;
	}
}

// -----------------------------------------------------------------------
int mem_get(int nb, uint16_t addr, uint16_t *data)
{
	uint16_t *ptr;

	ptr = mem_ptr(nb, addr);
	if (ptr) {
		*data = atom_load(ptr);
	} else {
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_put(int nb, uint16_t addr, uint16_t data)
{
	uint16_t *ptr;

	ptr = mem_ptr(nb, addr);
	if (ptr) {
		if (!mem_mega_prom || (mem_map[nb][addr>>12].seg != mem_mega_prom)) {
			atom_store(ptr, data);
		}
	} else {
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	int i;
	uint16_t *ptr;

	for (i=0 ; i<count ; i++) {
		ptr = mem_ptr(nb, (uint16_t) (saddr+i));
		if (ptr) {
			*(dest+i) = *ptr;
		} else {
			return 0;
		}
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	int i;
	uint16_t *ptr;

	for (i=0 ; i<count ; i++) {
		ptr = mem_ptr(nb, (uint16_t) (saddr+i));
		if (ptr) {
			if (!mem_mega_prom || (mem_map[nb][(saddr+i)>>12].seg != mem_mega_prom)) {
				*ptr = *(src+i);
			}
		} else {
			atom_fence();
			return 0;
		}
	}
	atom_fence();
	return 1;
}

// -----------------------------------------------------------------------
int mem_cpu_get(int nb, uint16_t addr, uint16_t *data)
{
	if (!mem_get(nb, addr, data)) {
		int_set(INT_NO_MEM);
		if ((nb == 0) && (nomem_stop)) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
		}
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_cpu_put(int nb, uint16_t addr, uint16_t data)
{
	if (!mem_put(nb, addr, data)) {
		int_set(INT_NO_MEM);
		if ((nb == 0) && (nomem_stop)) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
		}
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_cpu_mget(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	if (!mem_mget(nb, saddr, dest, count)) {
		int_set(INT_NO_MEM);
		if ((nb == 0) && (nomem_stop)) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
		}
		return 0;
	}
	return 1;
}


// -----------------------------------------------------------------------
int mem_cpu_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	if (!mem_mput(nb, saddr, src, count)) {
		int_set(INT_NO_MEM);
		if ((nb == 0) && (nomem_stop)) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
		}
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_get_byte(int nb, uint32_t addr, uint8_t *data)
{
	int shift;
	uint16_t orig = 0;

	if (!cpu_mod_active || (Q & BS)) addr &= 0xffff;

	shift = 8 * (~addr & 1);
	addr >>= 1;

	if (!mem_cpu_get(nb, addr, &orig)) return 0;
	*data = orig >> shift;

	return 1;
}

// -----------------------------------------------------------------------
int mem_put_byte(int nb, uint32_t addr, uint8_t data)
{
	int shift;
	uint16_t orig = 0;

	if (!cpu_mod_active || (Q & BS)) addr &= 0xffff;

	shift = 8 * (~addr & 1);
	addr >>= 1;

	if (!mem_cpu_get(nb, addr, &orig)) return 0;
	orig = (orig & (0b1111111100000000 >> shift)) | (((uint16_t) data) << shift);
	if (!mem_cpu_put(nb, addr, orig)) return 0;

	return 1;
}

// -----------------------------------------------------------------------
void mem_clear()
{
	mem_elwro_clear();
	mem_mega_clear();
	atom_fence();
}

// -----------------------------------------------------------------------
int mem_seg_load(FILE *f, uint16_t *ptr)
{
	int res;

	if (!ptr) {
		return E_MEM_BLOCK_TOO_SMALL;
	}

	res = fread(ptr, sizeof(uint16_t), MEM_SEGMENT_SIZE, f);
	if (ferror(f)) {
		return E_FILE_OPERATION;
	}

	// we swap bytes from big-endian to host-endianness at load time
	for (int i=0 ; i<res ; i++) {
		*(ptr+i) = ntohs(*(ptr+i));
	}
	atom_fence();
	
	return res;
}
// -----------------------------------------------------------------------
int mem_load(const char* fname, int nb, int start_ab, int len)
{
	int ret = E_OK;
	int loaded = 0;
	int res;
	uint16_t *ptr;
	uint16_t seg = start_ab;

	FILE *f = fopen(fname, "rb");
	if (f == NULL) {
		return E_FILE_OPEN;
	}

	EMULOG(L_MEM, 1, "Loading memory image: %s -> %d:%d", fname, nb, start_ab);

	do {
		// get pointer to segment in a block
		ptr = mem_map[nb][seg].seg;
		if (!ptr) {
			ret = E_MEM_BLOCK_TOO_SMALL;
			break;
		}

		// read chunk of data
		res = mem_seg_load(f, ptr);
		if (res <= 0) {
			break;
		}

		loaded += res;
		seg++;
	} while ((res == MEM_SEGMENT_SIZE) && ((loaded < len) || (len <= 0)));

	fclose(f);

	if (loaded > 0) {
		return loaded;
	} else {
		return ret;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
