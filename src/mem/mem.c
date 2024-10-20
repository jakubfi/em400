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
#include <inttypes.h>
#include <stdbool.h>

#include "mem/elwro.h"
#include "mem/mega.h"
#include "mem/mega_prom.h"
#include "mem/mem.h"
#include "io/defs.h"

#include "cfg.h"

#include "log.h"

uint16_t * mem_map[MEM_SEGMENTS][MEM_PAGES]; // final (as seen by emulation) logical->physical segment mapping

static int mega_modules = 0;

// -----------------------------------------------------------------------
static uint16_t *mem_ptr(int nb, uint16_t addr)
{
	uint16_t *frame_ptr = mem_map[nb][addr >> 12];
	return frame_ptr ? frame_ptr + (addr & 0b0000111111111111) : NULL;
}

// -----------------------------------------------------------------------
static void mem_update_map()
{
	for (int segment=0 ; segment<MEM_SEGMENTS ; segment++) {
		for (int page=0 ; page<MEM_PAGES ; page++) {
			mem_map[segment][page] = mem_elwro_get_frame_ptr(segment, page);
			if ((mega_modules > 0) && !mem_map[segment][page]) {
				mem_map[segment][page] = mem_mega_get_frame_ptr(segment, page);
			}
		}
	}
}

// -----------------------------------------------------------------------
int mem_init(em400_cfg *cfg)
{
	int res;

	const int elwro_modules = cfg_getint(cfg, "memory:elwro_modules", CFG_DEFAULT_MEMORY_ELWRO_MODULES);
	mega_modules = cfg_getint(cfg, "memory:mega_modules", CFG_DEFAULT_MEMORY_MEGA_MODULES);
	const int cfg_os = cfg_getint(cfg, "memory:hardwired_segments", CFG_DEFAULT_MEMORY_HARDWIRED_SEGMENTS);
	const char *mega_prom_image = cfg_getstr(cfg, "memory:mega_prom", CFG_DEFAULT_MEMORY_MEGA_PROM);

	if (elwro_modules + mega_modules > MEM_MODULES+1) {
		return LOGERR("Sum of Elwro and MEGA memory modules is greater than allowed %i.", MEM_MODULES+1);
	}

	res = mem_elwro_init(elwro_modules, cfg_os);
	if (res != E_OK) {
		return LOGERR("Failed to initialize Elwro memory.");
	}

	res = mem_mega_init(mega_modules, mega_prom_image);
	if (res != E_OK) {
		return LOGERR("Failed to initialize MEGA memory.");
	}

	mem_update_map();

	LOG(L_MEM, "Memory initialized. Elwro modules: %d, MEGA modules: %d, hardwired OS segments: %d, MEGA prom: %s",
		elwro_modules,
		mega_modules,
		cfg_os,
		mega_prom_image
	);

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_shutdown()
{
	LOG(L_MEM, "Shutdown memory");

	mem_mega_shutdown();
	mem_elwro_shutdown();
}

// -----------------------------------------------------------------------
int mem_cmd(uint16_t n, uint16_t r)
{
	int res;
	int segment	= (r & 0b0000000000001111);
	int page	= (r & 0b1111000000000000) >> 12;
	int module	= (n & 0b0000000000011110) >> 1;
	int frame	= (n & 0b0000000111100000) >> 5;
	int flags	= (n & 0b1111111000000000) >> 9;

	// if MEGA is present and MEM_MEGA_ALLOC is set => command for MEGA
	if ((mega_modules > 0) && (flags & MEM_MEGA_ALLOC)) {
		res = mem_mega_cmd(segment, page, module, frame, flags);
	// Elwro otherwise (but mask frame number to 3 bits)
	} else {
		res = mem_elwro_cmd(segment, page, module, frame & 0b0111);
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
bool mem_mega_boot()
{
	if ((mega_modules > 0) && (mem_map[0][15] == mem_mega_prom)) {
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------
bool mem_read_1(int nb, uint16_t addr, uint16_t *data)
{
	uint16_t *ptr = mem_ptr(nb, addr);
	if (ptr) {
		*data = *ptr;
	} else {
		// TODO: sems that MEGA should store -1
		*data = 0;
		return false;
	}
	return true;
}

// -----------------------------------------------------------------------
bool mem_write_1(int nb, uint16_t addr, uint16_t data)
{
	uint16_t *ptr = mem_ptr(nb, addr);
	if (ptr) {
		if (mem_map[nb][addr>>12] != mem_mega_prom) {
			*ptr = data;
		}
	} else {
		return false;
	}
	return true;
}

// -----------------------------------------------------------------------
bool mem_read_n(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	for ( ; count>0 ; count--, saddr++, dest++) {
		if (!mem_read_1(nb, saddr, dest)) return false;
	}
	return true;
}

// -----------------------------------------------------------------------
bool mem_write_n(int nb, uint16_t saddr, uint16_t *src, int count)
{
	for ( ; count>0 ; count--, saddr++, src++) {
		if (!mem_write_1(nb, saddr, *src)) return false;
	}
	return true;
}

// -----------------------------------------------------------------------
uint16_t mem_get_map(int segment)
{
	uint16_t map = 0;
	for (int page=0 ; page<MEM_PAGES ; page++) {
		if (mem_map[segment][page]) {
			map |= 1 << page;
		}
	}
	return map;
}

// vim: tabstop=4 shiftwidth=4 autoindent
