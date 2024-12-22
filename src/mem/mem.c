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
#include <string.h>

#include "mem/elwro.h"
#include "mem/mega.h"
#include "mem/mega_prom.h"
#include "mem/mem.h"
#include "io/defs.h"

#include "libem400.h"

#include "log.h"

typedef int (*mem_cmd_f)(int segment, int page, int module, int frame, int flags);

static mem_cmd_f mem_cmd_handlers[MEM_MODULES];
static uint16_t *mem_map_reads[MEM_SEGMENTS][MEM_PAGES];
static uint16_t *mem_map_writes[MEM_SEGMENTS][MEM_PAGES];

static struct em400_cfg_mem cfg_mem;

// -----------------------------------------------------------------------
static void mem_update_map()
{
	for (int segment=0 ; segment<MEM_SEGMENTS ; segment++) {
		for (int page=0 ; page<MEM_PAGES ; page++) {
			bool res;
			if (cfg_mem.elwro_modules) {
				res = mem_elwro_get_seg_ptrs(segment, page, &mem_map_reads[segment][page], &mem_map_writes[segment][page]);
				if (res) continue;
			}
			if (cfg_mem.mega_modules) {
				res = mem_mega_get_seg_ptrs(segment, page, &mem_map_reads[segment][page], &mem_map_writes[segment][page]);
				if (res) continue;
			}
			mem_map_reads[segment][page] = NULL;
			mem_map_writes[segment][page] = NULL;
		}
	}
}

// -----------------------------------------------------------------------
int mem_configure(struct em400_cfg_mem *c_mem)
{
	memcpy(&cfg_mem, c_mem, sizeof(struct em400_cfg_mem));

	return E_OK;
}

// -----------------------------------------------------------------------
int mem_init()
{
	int res;

	res = mem_elwro_init(cfg_mem.elwro_modules, cfg_mem.os_segments);
	if (res != E_OK) {
		return LOGERR("Failed to initialize Elwro memory.");
	}
	for (int module=0 ; module<cfg_mem.elwro_modules ; module++) {
		mem_cmd_handlers[module] = mem_elwro_cmd;
	}

	res = mem_mega_init(cfg_mem.mega_modules, cfg_mem.mega_prom_image);
	if (res != E_OK) {
		return LOGERR("Failed to initialize MEGA memory.");
	}
	// no mem_cmd_handlers[] are set for MEGA,
	// as the decision is made based on the command, not the module number

	LOG(L_MEM, "Memory initialized. Elwro modules: %d, MEGA modules: %d, hardwired OS segments: %d, MEGA prom: %s",
		cfg_mem.elwro_modules,
		cfg_mem.mega_modules,
		cfg_mem.os_segments,
		cfg_mem.mega_prom_image
	);

	mem_update_map();

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

	// if MEGA is present and MEM_MEGA_LOC is set, command is always handled by MEGA
	if (cfg_mem.mega_modules && (flags & MEM_MEGA_LOC)) {
		res = mem_mega_cmd(segment, page, module, frame, flags);
	} else if (mem_cmd_handlers[module]) {
		res = mem_cmd_handlers[module](segment, page, module, frame, flags);
	} else {
		LOG(L_MEM, "Memory module %i not installed, command unhandled", module);
		res = IO_NO;
	}

	if (res == IO_OK) {
		mem_update_map();
	}

	return res;
}

// -----------------------------------------------------------------------
void mem_reset(bool long_reset)
{
	if (cfg_mem.elwro_modules) mem_elwro_reset();
	if (cfg_mem.mega_modules) mem_mega_reset(long_reset);

	mem_update_map();
}

// -----------------------------------------------------------------------
bool mem_mega_boot()
{
	// Always boot from MEGA if present
	return (cfg_mem.mega_modules > 0);
}

// -----------------------------------------------------------------------
bool mem_read_1(int nb, uint16_t addr, uint16_t *data)
{
	uint16_t *ptr = mem_map_reads[nb][addr>>12];
	if (ptr) {
		*data = ptr[addr & 0xfff];
		return true;
	} else {
		*data = 0;
		return false;
	}
}

// -----------------------------------------------------------------------
bool mem_write_1(int nb, uint16_t addr, uint16_t data)
{
	uint16_t *ptr = mem_map_writes[nb][addr>>12];
	if (ptr) {
		ptr[addr & 0xfff] = data;
		return true;
	} else {
		return false;
	}
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
		if (mem_map_reads[segment][page]) {
			map |= 1 << page;
		}
	}
	return map;
}

// vim: tabstop=4 shiftwidth=4 autoindent
