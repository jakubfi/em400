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

#include "mem/mem_elwro.h"
#include "mem/mem_mega.h"
#include "mem/mem.h"
#include "io/defs.h"

#include "em400.h"
#include "cfg.h"

#include "log.h"

struct mem_slot_t mem_map[MEM_MAX_NB][MEM_MAX_AB];	// final (as seen by emulation) logical->physical segment mapping

static int mega_modules = 0;
static int mega_boot = 0;

// -----------------------------------------------------------------------
static uint16_t *mem_ptr(int nb, uint16_t addr)
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
	for (int nb=0 ; nb<MEM_MAX_NB ; nb++) {
		for (int ab=0 ; ab<MEM_MAX_AB ; ab++) {
			mem_elwro_seg_set(nb, ab, &mem_map[nb][ab]);
			if (!mem_map[nb][ab].seg) {
				mem_mega_seg_set(nb, ab, &mem_map[nb][ab]);
			}
		}
	}
}

// -----------------------------------------------------------------------
int mem_init(struct cfg_em400 *cfg)
{
	int res;

	LOG(L_MEM, 1, "Initializing memory (Elwro: %d modules, MEGA: %d modules)", cfg->mem_elwro, cfg->mem_mega);

	if (cfg->mem_elwro + cfg->mem_mega > MEM_MAX_MODULES+1) {
		return LOGERR("Sum of Elwro and MEGA memory modules is greater than allowed %i.", MEM_MAX_MODULES+1);
	}

	res = mem_elwro_init(cfg->mem_elwro, cfg->mem_os);
	if (res != E_OK) {
		return LOGERR("Failed to initialize Elwro memory.");
	}

	res = mem_mega_init(cfg->mem_mega, cfg->mem_mega_prom);
	if (res != E_OK) {
		return LOGERR("Failed to initialize MEGA memory.");
	}

	mem_update_map();

	mega_modules = cfg->mem_mega;
	mega_boot = cfg->mem_mega_boot;

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_shutdown()
{
	LOG(L_MEM, 1, "Shutdown memory");

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
	uint16_t *ptr = mem_ptr(nb, addr);
	if (ptr) {
		*data = *ptr;
	} else {
		return 0;
	}
	return 1;
}

// -----------------------------------------------------------------------
int mem_put(int nb, uint16_t addr, uint16_t data)
{
	uint16_t *ptr = mem_ptr(nb, addr);
	if (ptr) {
		if (!mem_mega_prom || (mem_map[nb][addr>>12].seg != mem_mega_prom)) {
			*ptr = data;
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
	for (i=0 ; i<count ; i++) {
		uint16_t *ptr = mem_ptr(nb, (uint16_t) (saddr+i));
		if (ptr) {
			*(dest+i) = *ptr;
		} else {
			break;
		}
	}
	return i;
}

// -----------------------------------------------------------------------
int mem_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	int i;
	for (i=0 ; i<count ; i++) {
		uint16_t *ptr = mem_ptr(nb, (uint16_t) (saddr+i));
		if (ptr) {
			if (!mem_mega_prom || (mem_map[nb][(saddr+i)>>12].seg != mem_mega_prom)) {
				*ptr = *(src+i);
			}
		} else {
			break;
		}
	}
	return i;
}

// -----------------------------------------------------------------------
uint16_t mem_get_map(int seg)
{
	uint16_t map = 0;
	for (int page=0 ; page<MEM_MAX_AB ; page++) {
		if (mem_map[seg][page].seg) {
			map |= 1 << page;
		}
	}
	return map;
}

// vim: tabstop=4 shiftwidth=4 autoindent
