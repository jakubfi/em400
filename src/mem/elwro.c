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

#include "io/defs.h"
#include "mem/mem.h"
#include "mem/elwro.h"

#include "log.h"

#define MEM_ELWRO_FRAMES 8

#define RAL(segment, page) (((segment)<<4) + (page))

static uint16_t *mem_elwro[MEM_MODULES][MEM_ELWRO_FRAMES];		// physical memory frames
static int mem_elwro_ral[MEM_MODULES][MEM_ELWRO_FRAMES];		// internal physical->logical mapping

static int mem_elwro_os_frames;									// number of frames hardwired for OS
static int mem_elwro_first_module, mem_elwro_last_module;		// modules allocated for Elwro

// -----------------------------------------------------------------------
int mem_elwro_init(int module_count, int os_pages)
{
	if (module_count <= 0) {
		LOG(L_MEM, "No Elwro modules");
		return E_OK;
	}

	if (module_count > MEM_MODULES) {
		return LOGERR("Wrong number of Elwro modules: %i. Should be < %i.", module_count, MEM_MODULES);
	}
	if ((os_pages < 1) || (os_pages > 2)) {
		return LOGERR("Wrong number of OS segment memory pages: %i. Should be 1 or 2.", os_pages);
	}

	mem_elwro_os_frames = os_pages;
	mem_elwro_first_module = 0;
	mem_elwro_last_module = module_count - 1;

	LOG(L_MEM, "Elwro modules: %d-%d, %d frames each, %d hardwired for OS pages", mem_elwro_first_module, mem_elwro_last_module, MEM_ELWRO_FRAMES, os_pages);

	for (int module=mem_elwro_first_module ; module<=mem_elwro_last_module ; module++) {
		for (int frame=0 ; frame<MEM_ELWRO_FRAMES ; frame++) {
			mem_elwro[module][frame] = (uint16_t *) calloc(sizeof(uint16_t), MEM_FRAME_SIZE);
			if (!mem_elwro[module][frame]) {
				return LOGERR("Memory allocation failed for Elwro map.");
			}
		}
	}

	mem_elwro_reset();

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_elwro_shutdown()
{
	for (int module=mem_elwro_first_module ; module<=mem_elwro_last_module ; module++) {
		for (int frame=0 ; frame<MEM_ELWRO_FRAMES ; frame++) {
			free(mem_elwro[module][frame]);
		}
	}
}

// -----------------------------------------------------------------------
void mem_elwro_reset()
{
	for (int module=mem_elwro_first_module ; module<=mem_elwro_last_module ; module++) {
		for (int frame=0 ; frame<MEM_ELWRO_FRAMES ; frame++) {
			mem_elwro_ral[module][frame] = RAL(0, 0);
		}
	}

	for (int frame=0 ; frame<mem_elwro_os_frames ; frame++) {
		mem_elwro_ral[0][frame] = RAL(0, frame);
	}
}

// -----------------------------------------------------------------------
bool mem_elwro_get_seg_ptrs(int segment, int page, uint16_t **rd_frame_ptr, uint16_t **wr_frame_ptr)
{
	// find lowest frame that has segment:page stored in its RAL
	for (int module=mem_elwro_first_module ; module<=mem_elwro_last_module ; module++) {
		for (int frame=0 ; frame<MEM_ELWRO_FRAMES ; frame++) {
			if (mem_elwro_ral[module][frame] == RAL(segment, page)) {
				*rd_frame_ptr = mem_elwro[module][frame];
				*wr_frame_ptr = mem_elwro[module][frame];
				return true;
			}
		}
	}

	return false;
}

// -----------------------------------------------------------------------
int mem_elwro_cmd(int segment, int page, int module, int frame, int flags)
{
	frame &= 0b111; // Elwro uses 3-bit frame address

	if (!mem_elwro[module][frame]) {
		LOG(L_MEM, "Elwro: ignored mapping to a nonexistent frame: logical [%d, %d] -> physical [%d, %d]", segment, page, module, frame);
		return IO_NO;
	}

	if ((module == 0) && (frame < mem_elwro_os_frames)) {
		LOG(L_MEM, "Elwro: ignored mapping to a hardwired frame: logical [%d, %d] -> physical [%d, %2d]", segment, page, module, frame);
		return IO_NO;
	}

	LOG(L_MEM, "Elwro: adding map: logical [%d, %d] -> physical [%d, %d]", segment, page, module, frame);

	mem_elwro_ral[module][frame] = RAL(segment, page);

	return IO_OK;
}

// vim: tabstop=4 shiftwidth=4 autoindent
