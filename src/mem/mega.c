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

#include "io/defs.h"
#include "mem/mega.h"
#include "mem/mega_prom.h"
#include "utils/utils.h"

#include "log.h"

uint16_t *mem_mega[MEM_MODULES][MEM_MEGA_FRAMES];	// physical memory frames
uint16_t *mem_mega_map[MEM_SEGMENTS][MEM_PAGES];	// internal logical->physical mapping
int mem_mega_first_module, mem_mega_last_module;   	// modules allocated for MEGA
bool mem_mega_prom_hidden;							// is PROM hidden?
bool mem_mega_alloc_done;							// is initialization done?

// -----------------------------------------------------------------------
int mem_mega_init(int module_count, const char *prom_image)
{
	if (module_count == 0) {
		return E_OK;
	}

	if ((module_count < 0) || (module_count > MEM_MODULES)) {
		return LOGERR("Wrong number of MEGA modules: %i. Should be 1-%i", module_count, MEM_MODULES);
	}

	mem_mega_first_module = MEM_MODULES - module_count;
	mem_mega_last_module = MEM_MODULES - 1;

	LOG(L_MEM, "MEGA modules: %d-%d, %d frames each", mem_mega_first_module, mem_mega_last_module, MEM_MEGA_FRAMES);

	for (int module=mem_mega_first_module ; module<=mem_mega_last_module ; module++) {
		for (int frame=0 ; frame<MEM_MEGA_FRAMES ; frame++) {
			mem_mega[module][frame] = (uint16_t *) calloc(sizeof(uint16_t), MEM_FRAME_SIZE);
			if (!mem_mega[module][frame]) {
				return LOGERR("Memory allocation failed for MEGA map.");
			}
		}
	}

	mem_mega_prom_hidden = false;

	// load custom PROM image
	if (prom_image && *prom_image) {
		LOG(L_MEM, "Loading custom MEGA PROM (max %i words) from image: %s", MEM_PAGE_SIZE, prom_image);
		FILE *f = fopen(prom_image, "rb");
		if (!f) {
			return LOGERR("Failed to open MEGA PROM image: \"%s\".", prom_image);
		}
		int res = fread(mem_mega_prom, sizeof(uint16_t), MEM_FRAME_SIZE, f);
		fclose(f);
		endianswap(mem_mega_prom, res);
		LOG(L_MEM, "Loaded %i words into MEGA PROM segmet", res);
	} else {
		LOG(L_MEM, "Using default MEGA PROM");
	}

	mem_mega_alloc_done = false;

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_mega_shutdown()
{
	for (int module=mem_mega_first_module ; module<=mem_mega_last_module ; module++) {
		for (int frame=0 ; frame<MEM_MEGA_FRAMES ; frame++) {
			free(mem_mega[module][frame]);
		}
	}
}

// -----------------------------------------------------------------------
void mem_mega_reset()
{
	for (int segment=0 ; segment<MEM_SEGMENTS ; segment++) {
		for (int page=0 ; page<MEM_PAGES ; page++) {
			mem_mega_map[segment][page] = NULL;
		}
	}
	// TODO: why doesn't work?
	// mem_mega_alloc_done = false;
	mem_mega_prom_hidden = false;
}

// -----------------------------------------------------------------------
uint16_t * mem_mega_get_frame_ptr(int segment, int page)
{
	// if PROM is shown, use it, ignore mem_mega_init_done
	if ((segment == 0) && (page == 15) && !mem_mega_prom_hidden) {
		return mem_mega_prom;
	// if MEGA configuration is not done, all memory access fails
	} else if (!mem_mega_alloc_done) {
		return NULL;
	// otherwise return segment pointer as internal MEGA configuration says
	} else {
		return mem_mega_map[segment][page];
	}
}

// -----------------------------------------------------------------------
int mem_mega_cmd(int segment, int page, int module, int frame, int flags)
{
	LOG(L_MEM, "MEGA: (%2d, %2d) -> (%2d, %2d)  flags: %s%s%s%s%s",
		segment, page, module, frame,
		flags & MEM_MEGA_ALLOC ? "alloc " : "",
		flags & MEM_MEGA_FREE ? "free " : "",
		flags & MEM_MEGA_PROM_SHOW ? "pshow " : "",
		flags & MEM_MEGA_PROM_HIDE ? "phide " : "",
		flags & MEM_MEGA_ALLOC_DONE ? "done" : ""
	);

	if ((flags & MEM_MEGA_FREE)) {
		mem_mega_map[segment][page] = NULL;
	} else {
		mem_mega_map[segment][page] = mem_mega[module][frame];
	}

	// MEGA PROM flag is stored in a J-K flip-flop...
	mem_mega_prom_hidden = (!mem_mega_prom_hidden && (flags & MEM_MEGA_PROM_HIDE)) || (mem_mega_prom_hidden && !(flags & MEM_MEGA_PROM_SHOW));

	if ((flags & MEM_MEGA_ALLOC_DONE)) {
		mem_mega_alloc_done = true;
	}

	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
