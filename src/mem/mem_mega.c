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

#include "io/io.h"
#include "mem/mem.h"
#include "mem/mem_mega.h"
#include "errors.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif
#include "emulog.h"

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
		return E_MEM;
	}

	mem_mega_mp_start = MEM_MAX_MODULES-modc;
	mem_mega_mp_end = MEM_MAX_MODULES-1;

	eprint("  MEGA modules: %d-%d, %d segments\n", mem_mega_mp_start, mem_mega_mp_end, MEM_MAX_MEGA_SEGMENTS);

	for (mp=mem_mega_mp_start ; mp<=mem_mega_mp_end ; mp++) {
		for (seg=0 ; seg<MEM_MAX_MEGA_SEGMENTS; seg++) {
			mem_mega[mp][seg] = calloc(sizeof(uint16_t), MEM_SEGMENT_SIZE);
			if (!mem_mega[mp][seg]) {
				return E_ALLOC;
			}
		}
	}

	// allocate and load MEGA PROM
	mem_mega_prom_hidden = 0;
	mem_mega_prom = calloc(sizeof(uint16_t), MEM_SEGMENT_SIZE);
	if (prom_image && *prom_image) {
		f = fopen(prom_image, "rb");
		if (!f) {
			return E_FILE_OPEN;
		}
		res = mem_seg_load(f, mem_mega_prom);
		if (res < E_OK) {
			fclose(f);
			return res;
		}
		fclose(f);
		eprint("  Loaded MEGA PROM image: %s (%i words)\n", prom_image, res);
	} else {
		eprint("  Empty MEGA PROM\n");
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
void mem_mega_clear()
{
	int mp, seg;
	for (mp=mem_mega_mp_end ; mp<=mem_mega_mp_end ; mp++) {
		for (seg=0 ; seg<MEM_MAX_MEGA_SEGMENTS ; seg++) {
			memset(mem_mega[mp][seg], 0, MEM_SEGMENT_SIZE);
		}
	}
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
	// 'free'
	if ((flags & MEM_MEGA_FREE)) {
		EMULOG(L_MEM, 1, "MEGA: del map (%2d, %2d)	 %s%s%s", nb, ab, flags & MEM_MEGA_PROM_SHOW ? "[prom show]" : "", flags & MEM_MEGA_PROM_HIDE ? "[prom hide]" : "", flags & MEM_MEGA_ALLOC_DONE ? " [done]" : "");
		mem_mega_map[nb][ab] = NULL;
	// 'alloc'
	} else {
		EMULOG(L_MEM, 1, "MEGA: add map (%2d, %2d) -> (%2d, %2d)	 %s%s%s", nb, ab, mp, seg, flags & MEM_MEGA_PROM_SHOW ? "[prom show]" : "", flags & MEM_MEGA_PROM_HIDE ? "[prom hide]" : "", flags & MEM_MEGA_ALLOC_DONE ? " [done]" : "");
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
