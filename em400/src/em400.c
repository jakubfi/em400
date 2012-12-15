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

#include <stdlib.h>
#include <stdio.h>

#include "cpu.h"
#include "memory.h"
#include "timer.h"
#include "registers.h"
#include "errors.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#endif

int em400_quit = 0;

// -----------------------------------------------------------------------
void eerr(char *message, int ecode)
{
	printf("%s: %s\n", message, get_error(ecode));
	exit(1);
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	int res;

	printf("Starting EM400...\n");

	res = mem_init();
	if (res) {
		mem_shutdown();
		eerr("Error initializing EM400", res);
	}

	mem_clear();
	cpu_reset();

	res = timer_start();
	if (res) {
		mem_shutdown();
		eerr("Error initializing CPU timer", res);
	}

#ifdef WITH_DEBUGGER
	res = dbg_init();
	if (res) {
		dbg_shutdown();
		mem_shutdown();
		eerr("Error initializing debugger", res);
	}
#endif

	while (!em400_quit) {
#ifdef WITH_DEBUGGER
		dbg_step();
		if (em400_quit) {
			break;
		}
		mem_actr_max = -1;
		mem_actw_max = -1;
		for (int r=0 ; r<R_MAX ; r++) {
			reg_act[r] = C_DATA;
		}
#endif
		cpu_step();
	}

#ifdef WITH_DEBUGGER
	dbg_shutdown();
#endif

	mem_shutdown();
	timer_remove();
	printf("EM400 exits.\n");

	return 0;
}

// vim: tabstop=4
