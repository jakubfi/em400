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
#include <string.h>

#include "io/defs.h"
#include "mem/mem_mega.h"
#include "utils/utils.h"

#include "log.h"

uint16_t *mem_mega[MEM_MAX_MODULES][MEM_MAX_MEGA_SEGMENTS];	// physical memory segments
uint16_t *mem_mega_map[MEM_MAX_NB][MEM_MAX_AB];				// internal logical->physical segment mapping
int mem_mega_mp_start, mem_mega_mp_end;   					// modules allocated for MEGA
uint16_t *mem_mega_prom;									// PROM contents
int mem_mega_prom_hidden;									// is PROM hidden?
int mem_mega_init_done;										// is initialization done?

// -----------------------------------------------------------------------
int mem_mega_init(int modc, char *prom_image)
{
	int res;
	int mp, seg;
	FILE *f;

	if (modc == 0) {
		return E_OK;
	}

	if ((modc < 0) || (modc > MEM_MAX_MODULES)) {
		return LOGERR("Wrong number of MEGA modules: %i. Should be 1-%i", modc, MEM_MAX_MODULES);
	}

	mem_mega_mp_start = MEM_MAX_MODULES-modc;
	mem_mega_mp_end = MEM_MAX_MODULES-1;

	LOG(L_MEM, 1, "MEGA modules: %d-%d, %d segments", mem_mega_mp_start, mem_mega_mp_end, MEM_MAX_MEGA_SEGMENTS);

	for (mp=mem_mega_mp_start ; mp<=mem_mega_mp_end ; mp++) {
		for (seg=0 ; seg<MEM_MAX_MEGA_SEGMENTS; seg++) {
			mem_mega[mp][seg] = calloc(sizeof(uint16_t), MEM_SEGMENT_SIZE);
			if (!mem_mega[mp][seg]) {
				return LOGERR("Memory allocation failed for MEGA map.");
			}
		}
	}

	// allocate memory for MEGA PROM
	mem_mega_prom_hidden = 0;
	mem_mega_prom = malloc(sizeof(uint16_t) * MEM_SEGMENT_SIZE);
	if (!mem_mega_prom) {
		return LOGERR("Memory allocation error for MEGA PROM.");
	}

	// load PROM image
	if (prom_image && *prom_image) {
		f = fopen(prom_image, "rb");
		if (!f) {
			return LOGERR("Failed to open PROM image: \"%s\".", prom_image);
		}
		res = fread(mem_mega_prom, sizeof(uint16_t), MEM_SEGMENT_SIZE, f);
		if (res != MEM_SEGMENT_SIZE) {
			fclose(f);
			return LOGERR("Read only %i words of MEGA PROM. Expecting %i.", res, MEM_SEGMENT_SIZE);
		}
		fclose(f);
		endianswap(mem_mega_prom, res);
		LOG(L_MEM, 1, "Loaded MEGA PROM image: %s (%i words)", prom_image, res);
	} else {
		LOG(L_MEM, 1, "Empty MEGA PROM");
	}

	mem_mega_init_done = 0;

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_mega_shutdown()
{
	int mp, seg;

	for (mp=mem_mega_mp_start ; mp<=mem_mega_mp_end ; mp++) {
		for (seg=0 ; seg<MEM_MAX_MEGA_SEGMENTS ; seg++) {
			free(mem_mega[mp][seg]);
		}
	}
	
	free(mem_mega_prom);
}

// -----------------------------------------------------------------------
void mem_mega_reset()
{
	int nb, ab;
	for (nb=0 ; nb<MEM_MAX_NB ; nb++) {
		for (ab=0 ; ab<MEM_MAX_AB ; ab++) {
			mem_mega_map[nb][ab] = NULL;
		}
	}
	mem_mega_prom_hidden = 0;
}

// -----------------------------------------------------------------------
void mem_mega_seg_set(int nb, int ab, struct mem_slot_t *slot)
{
	// if PROM is shown, use it, ignore mem_mega_init_done
	if ((nb == 0) && (ab == 15) && !mem_mega_prom_hidden) {
		slot->seg = mem_mega_prom;
	// if MEGA configuration is not done, all memory access fails
	} else if (!mem_mega_init_done) {
		slot->seg = NULL;
	// return segment pointer as internal MEGA configuration says
	} else {
		slot->seg = mem_mega_map[nb][ab];
	}
}

// -----------------------------------------------------------------------
int mem_mega_cmd(int nb, int ab, int mp, int seg, int flags)
{
	LOG(L_MEM, 1, "MEGA: (%2d, %2d) -> (%2d, %2d)  flags: %s%s%s%s%s",
		nb,
		ab,
		mp,
		seg,
		flags & MEM_MEGA_ALLOC ? "alloc " : "",
		flags & MEM_MEGA_FREE ? "free " : "",
		flags & MEM_MEGA_PROM_SHOW ? "pshow " : "",
		flags & MEM_MEGA_PROM_HIDE ? "phide " : "",
		flags & MEM_MEGA_ALLOC_DONE ? "done" : ""
	);

	// 'free'
	if ((flags & MEM_MEGA_FREE)) {
		mem_mega_map[nb][ab] = NULL;
	// 'alloc'
	} else {
		mem_mega_map[nb][ab] = mem_mega[mp][seg];
	}

	// 'PROM hide'
	if ((flags & MEM_MEGA_PROM_HIDE)) {
		mem_mega_prom_hidden = 1;
	}

	// 'PROM show'
	if ((flags & MEM_MEGA_PROM_SHOW)) {
		mem_mega_prom_hidden = 0;
	}

	// 'allocation done'
	if ((flags & MEM_MEGA_ALLOC_DONE)) {
		mem_mega_init_done = 1;
	}

	return IO_OK;
}


// vim: tabstop=4 shiftwidth=4 autoindent
