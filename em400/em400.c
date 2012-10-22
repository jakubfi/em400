//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdio.h>

#include "cpu.h"
#include "memory.h"
#include "timer.h"
#include "debuger.h"

int em400_quit = 0;

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	mjc400_reset();
	em400_mem_init();
	em400_mem_clear();
	em400_debuger_init();
	mjc400_timer_start();

	while (!em400_quit) {
		int dbg_res = em400_debuger_step();
		if (dbg_res <= DEBUGER_EM400_SKIP) {
			if (dbg_res <= DEBUGER_EM400_QUIT) {
				em400_quit = 1;
			}
			continue;
		}
		mjc400_step();
	}

	em400_debuger_shutdown();
	em400_mem_shutdown();

	return 0;
}

// vim: tabstop=4
