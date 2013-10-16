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
#include "cpu/registers.h"
#include "cpu/memory.h"
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

uint16_t *mem_map[MEM_MAX_NB][MEM_MAX_AB];						// logical->physical segment mapping

uint16_t *mem_elwro[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];			// Elwro: physical segments
uint16_t **mem_elwro_ral[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];	// Elwro: internal physical->logical segment mapping

uint16_t *mem_mega[MEM_MAX_MODULES][MEM_MAX_SEGMENTS];			// MEGA: physical segments
uint16_t *mem_mega_map[MEM_MAX_NB][MEM_MAX_AB];					// MEGA: internal logical->physical segment mapping
uint16_t *mem_mega_prom;										// MEGA: PROM contents

// -----------------------------------------------------------------------
int mem_create_mp(int mp, int segments, uint16_t **sptr)
{
	int seg;

	eprint("  Module %2i: %2i adding segments\n", mp, segments);

	for (seg=0 ; seg<segments; seg++) {
		*(sptr+seg) = malloc(sizeof(uint16_t) * MEM_SEGMENT_SIZE);
		if (!*(sptr+seg)) {
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

	pthread_spin_init(&mem_spin, 0);

	// create physical segments
	for (mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		if (mp < em400_cfg.mem_elwro) {
			res = mem_create_mp(mp, MEM_MAX_ELWRO_SEGMENTS, mem_elwro[mp]);
			if (res != E_OK) {
				return res;
			}
		}
		// mega modules may overlap with elwro, that's why it's not else'd
		if (mp >= MEM_MAX_MODULES - em400_cfg.mem_mega) {
			res = mem_create_mp(mp, MEM_MAX_MEGA_SEGMENTS, mem_mega[mp]);
			if (res != E_OK) {
				return res;
			}
		}
	}

	// allocate and show MEGA PROM if MEGA is there
	if (em400_cfg.mem_mega > 0) {
		mem_mega_prom = calloc(sizeof(uint16_t), MEM_SEGMENT_SIZE);
		pthread_spin_lock(&mem_spin);
		mem_map[0][15] = mem_mega_prom;
		pthread_spin_unlock(&mem_spin);
		if (em400_cfg.mem_mega_prom) {
			res = mem_load_image(em400_cfg.mem_mega_prom, 0, 15, 4096);
			eprint("  Loaded MEGA PROM image: %s (%i words)\n", em400_cfg.mem_mega_prom, res);
		}
	}

	// hardwire elwro segments for OS
	for (ab=0 ; ab<em400_cfg.mem_os ; ab++) {
		pthread_spin_lock(&mem_spin);
		mem_map[0][ab] = mem_elwro[0][ab];
		pthread_spin_unlock(&mem_spin);
	}

	mem_clear();

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
			free(mem_elwro[mp][seg]);
			free(mem_mega[mp][seg]);
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

	// if MEGA is present and MEM_MEGA_ALLOC is set => command for MEGA
	if ((em400_cfg.mem_mega > 0) && ((flags & MEM_MEGA_ALLOC))) {
		return mem_cmd_mega(nb, ab, mp, seg, flags);
	// Elwro otherwise
	} else {
		return mem_cmd_elwro(nb, ab, mp, seg & 0b0111);
	}
}

// -----------------------------------------------------------------------
int mem_cmd_elwro(int nb, int ab, int mp, int seg)
{
	if (!mem_elwro[mp][seg]) {
		LOG(L_MEM, 1, "Elwro: ignore nonexistent segment (%2d, %2d)", mp, seg);
		return IO_NO;
	}

	if ((mp == 0) && (seg < em400_cfg.mem_os)) {
		LOG(L_MEM, 1, "Elwro: ignore hardwired segment (%2d, %2d)", mp, seg);
		return IO_NO;
	}

	LOG(L_MEM, 1, "Elwro: add map (%2d, %2d) -> (%2d, %2d)", nb, ab, mp, seg);

	// clear p->l mapping if set
	if (mem_elwro_ral[mp][seg]) {
		*mem_elwro_ral[mp][seg] = NULL;
	}

	// make new l->p mapping
	pthread_spin_lock(&mem_spin);
	mem_map[nb][ab] = mem_elwro[mp][seg];
	// remember new p->l mapping
	mem_elwro_ral[mp][seg] = &mem_map[nb][ab];
	pthread_spin_unlock(&mem_spin);

	return IO_OK;
}

// -----------------------------------------------------------------------
int mem_cmd_mega(int nb, int ab, int mp, int seg, int flags)
{
	// touch only non-OS ab
	if ((nb > 0) || ((nb == 0) && (ab > 1))) {
		// 'free'
		if ((flags & MEM_MEGA_FREE)) {
			LOG(L_MEM, 1, "MEGA: del map (%2d, %2d)     %s%s", nb, ab, flags & MEM_MEGA_PROM_SHOW ? "[prom show]" : "", flags & MEM_MEGA_PROM_HIDE ? "[prom hide]" : "");
			pthread_spin_lock(&mem_spin);
			if (mem_map[nb][ab] == mem_mega_map[nb][ab]) {
				mem_map[nb][ab] = mem_mega_map[nb][ab] = NULL;
			}
			pthread_spin_unlock(&mem_spin);
		// 'alloc'
		} else {
			LOG(L_MEM, 1, "MEGA: add map (%2d, %2d) -> (%2d, %2d)     %s%s", nb, ab, mp, seg, flags & MEM_MEGA_PROM_SHOW ? "[prom show]" : "", flags & MEM_MEGA_PROM_HIDE ? "[prom hide]" : "");
			pthread_spin_lock(&mem_spin);
			mem_map[nb][ab] = mem_mega_map[nb][ab] = mem_mega[mp][seg];
			pthread_spin_unlock(&mem_spin);
		}
	} else {
		LOG(L_MEM, 1, "MEGA: ignored map (%2d, %2d) -> (%2d, %2d)     %s%s", nb, ab, mp, seg, flags & MEM_MEGA_PROM_SHOW ? "[prom show]" : "", flags & MEM_MEGA_PROM_HIDE ? "[prom hide]" : "");
	}

	// 'PROM hide'
	if ((flags & MEM_MEGA_PROM_HIDE)) {
		pthread_spin_lock(&mem_spin);
		mem_map[0][15] = mem_mega_map[0][15];
		pthread_spin_unlock(&mem_spin);
	}

	// 'PROM show'
	if ((flags & MEM_MEGA_PROM_SHOW)) {
		pthread_spin_lock(&mem_spin);
		mem_map[0][15] = mem_mega_prom;
		pthread_spin_unlock(&mem_spin);
	}

	// TODO: 'allocation done'
	if ((flags & MEM_MEGA_ALLOC_DONE)) {
		LOG(L_MEM, 1, "MEGA: initializarion done");
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
void mem_reset()
{
	int nb;
	int ab;

	// remove all memory mappings
	for (nb=0 ; nb<MEM_MAX_NB ; nb++) {
		for (ab=0 ; ab<MEM_MAX_AB ; ab++) {
			if ((nb>0) || ((nb==0) && (ab>1))) {
				pthread_spin_lock(&mem_spin);
				mem_map[nb][ab] = NULL;
				pthread_spin_unlock(&mem_spin);
			}
			mem_mega_map[nb][ab] = NULL;
			mem_elwro_ral[nb][ab] = NULL;
		}
	}

	// show MEGA PROM if MEGA is there
	if (em400_cfg.mem_mega > 0) {
		pthread_spin_lock(&mem_spin);
		mem_map[0][15] = mem_mega_prom;
		pthread_spin_unlock(&mem_spin);
	}
}

// -----------------------------------------------------------------------
int mem_get(int nb, uint16_t addr, uint16_t *data)
{
	uint16_t *ptr;

	pthread_spin_lock(&mem_spin);
	ptr = mem_ptr(nb, addr);
	if (ptr) {
		*data = *ptr;
		pthread_spin_unlock(&mem_spin);
	} else {
		pthread_spin_unlock(&mem_spin);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_put(int nb, uint16_t addr, uint16_t data)
{
	uint16_t *ptr;

	pthread_spin_lock(&mem_spin);
	ptr = mem_ptr(nb, addr);
	if (ptr) {
		if (!mem_mega_prom || (mem_map[nb][addr>>12] != mem_mega_prom)) {
			*ptr = data;
		}
		pthread_spin_unlock(&mem_spin);
	} else {
		pthread_spin_unlock(&mem_spin);
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	int i;
	uint16_t *ptr;

	pthread_spin_lock(&mem_spin);
	for (i=0 ; i<count ; i++) {
		ptr = mem_ptr(nb, (uint16_t) (saddr+i));
		if (ptr) {
			*(dest+i) = *ptr;
		} else {
			pthread_spin_unlock(&mem_spin);
			return 0;
		}
	}
	pthread_spin_unlock(&mem_spin);
	return 1;
}

// -----------------------------------------------------------------------
int mem_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	int i;
	uint16_t *ptr;

	pthread_spin_lock(&mem_spin);
	for (i=0 ; i<count ; i++) {
		ptr = mem_ptr(nb, (uint16_t) (saddr+i));
		if (ptr) {
			if (!mem_mega_prom || (mem_map[nb][(saddr+i)>>12] != mem_mega_prom)) {
				*ptr = *(src+i);
			}
		} else {
			pthread_spin_unlock(&mem_spin);
			return 0;
		}
	}
	pthread_spin_unlock(&mem_spin);
	return 1;
}

// -----------------------------------------------------------------------
int mem_cpu_get(int nb, uint16_t addr, uint16_t *data)
{
	if (!mem_get(nb, addr, data)) {
		int_set(INT_NO_MEM);
		if (nb == 0) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
			return 0;
		}
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_cpu_put(int nb, uint16_t addr, uint16_t data)
{
	if (!mem_put(nb, addr, data)) {
		int_set(INT_NO_MEM);
		if (nb == 0) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
			return 0;
		}
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_cpu_mget(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	if (!mem_mget(nb, saddr, dest, count)) {
		int_set(INT_NO_MEM);
		if (nb == 0) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
			return 0;
		}
	}
	return 1;
}


// -----------------------------------------------------------------------
int mem_cpu_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	if (!mem_mput(nb, saddr, src, count)) {
		int_set(INT_NO_MEM);
		if (nb == 0) {
			regs[R_ALARM] = 1;
#ifdef WITH_DEBUGGER
			dbg_enter = 1;
#else
			em400_state = STATE_MEM_FAIL;
#endif
			return 0;
		}
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_get_byte(int nb, uint16_t addr, uint8_t *data)
{
	int shift;
	uint16_t addr17;
	uint16_t orig = 0;

	shift = 8 * (~addr & 1);

	if (cpu_mod) {
		addr17 = (addr >> 1) | (regs[R_ZC17] << 15);
		regs[R_ZC17] = 0;
	} else {
		addr17 = addr >> 1;
	}

	if (!mem_cpu_get(nb, addr17, &orig)) return 0;
	*data = orig >> shift;

	return 1;
}

// -----------------------------------------------------------------------
int mem_put_byte(int nb, uint16_t addr, uint8_t data)
{
	int shift;
	uint16_t addr17;
	uint16_t orig = 0;

	shift = 8 * (~addr & 1);

	if (cpu_mod) {
		addr17 = (addr >> 1) | (regs[R_ZC17] << 15);
		regs[R_ZC17] = 0;
	} else {
		addr17 = addr >> 1;
	}

	if (!mem_cpu_get(nb, addr17, &orig)) return 0;
	orig = (orig & (0b1111111100000000 >> shift)) | (((uint16_t) data) << shift);
	if (!mem_cpu_put(nb, addr17, orig)) return 0;

	return 1;
}

// -----------------------------------------------------------------------
void mem_clear()
{
	int mp;
	int seg;

	for (mp=0 ; mp<MEM_MAX_MODULES ; mp++) {
		for (seg=0 ; seg<MEM_MAX_SEGMENTS ; seg++) {
			pthread_spin_lock(&mem_spin);
			if (mem_elwro[mp][seg]) {
				memset(mem_elwro[mp][seg], 0, MEM_SEGMENT_SIZE);
			}
			if (mem_mega[mp][seg]) {
				memset(mem_mega[mp][seg], 0, MEM_SEGMENT_SIZE);
			}
			pthread_spin_unlock(&mem_spin);
		}
	}
}

// -----------------------------------------------------------------------
int mem_load_image(const char* fname, int nb, int start_seg, int len)
{
	int ret = E_OK;
	int loaded = 0;
	int res;
	uint16_t seg = start_seg;

	FILE *f = fopen(fname, "rb");
	if (f == NULL) {
		return E_FILE_OPEN;
	}

	LOG(L_MEM, 1, "Loading memory image: %s -> %d:%d", fname, nb, start_seg);

	do {
		// get pointer to segment in a block
		pthread_spin_lock(&mem_spin);
		uint16_t *ptr = mem_map[nb][seg];
		if (!ptr) {
			ret = E_MEM_BLOCK_TOO_SMALL;
			pthread_spin_unlock(&mem_spin);
			break;
		}
		pthread_spin_unlock(&mem_spin);

		// read chunk of data
		res = fread(ptr, sizeof(uint16_t), MEM_SEGMENT_SIZE, f);
		if (ferror(f)) {
			ret = E_FILE_OPERATION;
			break;
		}

		if (res <= 0) {
			break;
		}

		// we swap bytes from big-endian to host-endianness at load time
		pthread_spin_lock(&mem_spin);
		for (int i=0 ; i<res ; i++) {
			*(ptr+i) = ntohs(*(ptr+i));
		}
		pthread_spin_unlock(&mem_spin);

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
