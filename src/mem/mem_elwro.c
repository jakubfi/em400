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
#include "mem/mem_elwro.h"
#include "errors.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif

#include "emulog.h"

#define RAL(nb, ab) (((nb)<<4) + (ab))

uint16_t *mem_elwro[MEM_MAX_MODULES][MEM_MAX_ELWRO_SEGMENTS];	// physical memory segments
int mem_elwro_ral[MEM_MAX_MODULES][MEM_MAX_ELWRO_SEGMENTS];		// internal physical->logical mapping
int mem_elwro_os_segments;										// segments hardwired for OS
int mem_elwro_mp_start, mem_elwro_mp_end;						// modules allocated for Elwro

// -----------------------------------------------------------------------
void mem_elwro_os_hardwire()
{
	int seg;

	for (seg=0 ; seg<mem_elwro_os_segments ; seg++) {
		mem_elwro_ral[0][seg] = RAL(0, seg);
	}
}

// -----------------------------------------------------------------------
int mem_elwro_init(int modc, int seg_os)
{
	int mp, seg;

	if ((modc < 1) || (modc > MEM_MAX_MODULES) || (seg_os < 1) || (seg_os > 2)) {
		return E_MEM;
	}

	mem_elwro_os_segments = seg_os;
	mem_elwro_mp_start = 0;
	mem_elwro_mp_end = modc-1;

	eprint("  Elwro modules: %d-%d, %d segments (%d hardwired OS segments)\n", mem_elwro_mp_start, mem_elwro_mp_end, MEM_MAX_ELWRO_SEGMENTS, seg_os);

	for (mp=mem_elwro_mp_start ; mp<=mem_elwro_mp_end ; mp++) {
		for (seg=0 ; seg<MEM_MAX_ELWRO_SEGMENTS; seg++) {
			mem_elwro[mp][seg] = calloc(sizeof(uint16_t), MEM_SEGMENT_SIZE);
			if (!mem_elwro[mp][seg]) {
				return E_ALLOC;
			}
		}
	}

	mem_elwro_os_hardwire();

	return E_OK;
}

// -----------------------------------------------------------------------
void mem_elwro_shutdown()
{
	int mp, seg;

	for (mp=mem_elwro_mp_start ; mp<=mem_elwro_mp_end ; mp++) {
		for (seg=0 ; seg<MEM_MAX_ELWRO_SEGMENTS ; seg++) {
			free(mem_elwro[mp][seg]);
		}
	}
}


// -----------------------------------------------------------------------
void mem_elwro_reset()
{
	int mp, seg;
	for (mp=mem_elwro_mp_start ; mp<=mem_elwro_mp_end ; mp++) {
		for (seg=0 ; seg<MEM_MAX_ELWRO_SEGMENTS ; seg++) {
			mem_elwro_ral[mp][seg] = RAL(0, 0);
		}
	}

	mem_elwro_os_hardwire();
}

// -----------------------------------------------------------------------
void mem_elwro_clear()
{
	int mp, seg;
	for (mp=mem_elwro_mp_start ; mp<=mem_elwro_mp_end ; mp++) {
		for (seg=0 ; seg<MEM_MAX_ELWRO_SEGMENTS ; seg++) {
			memset(mem_elwro[mp][seg], 0, MEM_SEGMENT_SIZE);
		}
	}
}

// -----------------------------------------------------------------------
void mem_elwro_seg_set(int nb, int ab, struct mem_slot_t *slot)
{
	int mp, seg;

	// find lowest segment that has nb:ab in its RAL
	for (mp=mem_elwro_mp_start ; mp<=mem_elwro_mp_end ; mp++) {
		for (seg=0 ; seg<MEM_MAX_ELWRO_SEGMENTS ; seg++) {
			if (mem_elwro_ral[mp][seg] == RAL(nb, ab)) {
				slot->seg = mem_elwro[mp][seg];
				return;
			}
		}
	}
	slot->seg = NULL;
}

// -----------------------------------------------------------------------
int mem_elwro_cmd(int nb, int ab, int mp, int seg)
{
	if (!mem_elwro[mp][seg]) {
		EMULOG(L_MEM, 1, "Elwro: ignore mapping nonexistent physical segment (%2d, %2d) -> (%2d, %2d)", nb, ab, mp, seg);
		return IO_NO;
	}

	if ((mp == 0) && (seg < mem_elwro_os_segments)) {
		EMULOG(L_MEM, 1, "Elwro: ignore mapping hardwired segment (%2d, %2d) -> (%2d, %2d)", nb, ab, mp, seg);
		return IO_NO;
	}

	EMULOG(L_MEM, 1, "Elwro: add map (%2d, %2d) -> (%2d, %2d)", nb, ab, mp, seg);

	mem_elwro_ral[mp][seg] = RAL(nb, ab);

	return IO_OK;
}


// vim: tabstop=4 shiftwidth=4 autoindent
