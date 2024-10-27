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

#include "io/defs.h"
#include "mem/mem.h"
#include "mem/mega.h"
#include "mem/mega_prom.h"
#include "utils/utils.h"

#include "log.h"

#define MEM_MEGA_FRAMES MEM_FRAMES	// frames in mega module

uint16_t mem_mega_frame_devnull[MEM_FRAME_SIZE];			// writes to nonexistent frames land here
uint16_t mem_mega_frame_ffff[MEM_FRAME_SIZE];				// reads from nonexistent frames return 0xffff
uint16_t *mem_mega[MEM_MODULES][MEM_MEGA_FRAMES];			// physical memory frames
uint16_t *mem_mega_map[MEM_SEGMENTS][MEM_PAGES];			// internal logical->physical mapping

static int mem_mega_first_module, mem_mega_last_module;   	// modules allocated for MEGA
static bool mem_mega_prom_hidden;							// is PROM hidden?
static bool mem_mega_alloc_done;							// is initialization done?

// -----------------------------------------------------------------------
int mem_mega_init(int module_count, const char *prom_image)
{
	if (module_count <= 0) {
		LOG(L_MEM, "No MEGA modules");
		return E_OK;
	}

	if (module_count > MEM_MODULES) {
		return LOGERR("Wrong number of MEGA modules: %i. Should be < %i", module_count, MEM_MODULES);
	}

	memset(mem_mega_frame_ffff, 0xff, 2*MEM_FRAME_SIZE);

	mem_mega_first_module = MEM_MODULES - module_count;
	mem_mega_last_module = MEM_MODULES - 1;

	LOG(L_MEM, "MEGA modules: %d-%d, %d frames each", mem_mega_first_module, mem_mega_last_module, MEM_MEGA_FRAMES);

	for (int module=0 ; module<MEM_MODULES ; module++) {
		for (int frame=0 ; frame<MEM_MEGA_FRAMES ; frame++) {
			if (module < mem_mega_first_module) {
				mem_mega[module][frame] = mem_mega_frame_ffff;
			} else {
				mem_mega[module][frame] = (uint16_t *) calloc(sizeof(uint16_t), MEM_FRAME_SIZE);
				if (!mem_mega[module][frame]) {
					return LOGERR("Memory allocation failed for MEGA map.");
				}
			}
		}
	}

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

	mem_mega_reset(true);

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_mega_shutdown()
{
	LOG(L_MEM, "Shutting down MEGA memory");

	for (int module=mem_mega_first_module ; module<=mem_mega_last_module ; module++) {
		for (int frame=0 ; frame<MEM_MEGA_FRAMES ; frame++) {
			free(mem_mega[module][frame]);
		}
	}
}

// -----------------------------------------------------------------------
void mem_mega_reset(bool long_reset)
{
	// 1. Documentation states that CLM unhides PROM. Doesn't say anything about "alloc done" flag.
	// 2. Schematic shows that CLM acts the same way on both prom and alloc flip-flops.
	// 3. In practice, memory configurator routine stored in PROM does not (and cannot)
	//    work correctly if allocation flag is reset during software (MCL) reset.
	//    This routine doesn't seem to care about prom flag.
	// 4. Analysis of modifications on an actual ME-GA-400-3 PCB suggest that both prom and alloc flags
	//    are reset only on sufficiently long (~10ms?) CLM signal. MCL generates only ~15us signal,
	//    so that means in practice memory allocation and prom flags are only reset with manual CLEAR
	//    from the control panel (human toggling a switch should generate signal >10ms).

	LOG(L_MEM, "MEGA reset: %s", long_reset ? "honored" : "ignored");

	if (long_reset) {
		mem_mega_alloc_done = false;
		mem_mega_prom_hidden = false;
	}
}

// -----------------------------------------------------------------------
bool mem_mega_get_seg_ptrs(int segment, int page, uint16_t **rd_frame_ptr, uint16_t **wr_frame_ptr)
{
	// if PROM is shown, use it, overriding mapping
	if ((segment == 0) && (page == 15) && !mem_mega_prom_hidden) {
		*rd_frame_ptr = mem_mega_prom;
		*wr_frame_ptr = mem_mega_frame_devnull;
		return true;
	// if MEGA configuration is not done, all memory accesses fail
	} else if (!mem_mega_alloc_done) {
		return false;
	// otherwise return segment pointer as internal MEGA configuration says
	} else {
		*rd_frame_ptr = mem_mega_map[segment][page];
		// for missing modules, all writes go to /dev/null
		if (*rd_frame_ptr == mem_mega_frame_ffff) {
			*wr_frame_ptr = mem_mega_frame_devnull;
		} else {
			*wr_frame_ptr = mem_mega_map[segment][page];
		}
		return true;
	}
}

// -----------------------------------------------------------------------
int mem_mega_cmd(int segment, int page, int module, int frame, int flags)
{
	LOG(L_MEM, "MEGA mapping: logical [%d, %d] -> physical [%d, %d]  flags: %s%s%s%s",
		segment, page, module, frame,
		flags & MEM_MEGA_DEALLOC ? "dealloc " : "",
		flags & MEM_MEGA_PROM_SHOW ? "prom_show " : "",
		flags & MEM_MEGA_PROM_HIDE ? "prom_hide " : "",
		flags & MEM_MEGA_ALLOC_DONE ? "alloc_done" : ""
	);

	// MEGA PROM flag is stored in a J-K flip-flop...
	mem_mega_prom_hidden = (!mem_mega_prom_hidden && (flags & MEM_MEGA_PROM_HIDE)) || (mem_mega_prom_hidden && !(flags & MEM_MEGA_PROM_SHOW));

	if ((flags & MEM_MEGA_DEALLOC)) {
		mem_mega_map[segment][page] = NULL;
	} else {
		mem_mega_map[segment][page] = mem_mega[module][frame];
	}

	if ((flags & MEM_MEGA_ALLOC_DONE)) {
		mem_mega_alloc_done = true;
	}

	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
