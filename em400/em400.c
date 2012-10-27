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
#include "errors.h"

#ifdef WITH_DEBUGER
#include "debuger.h"
#endif

int em400_quit = 0;

// -----------------------------------------------------------------------
void eerr(char *message, int ecode)
{
	printf("%s: %s\n", message, e400_gerror(ecode));
	exit(1);
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	int res;

	printf("Starting EM400...\n");

	res = em400_mem_init();
	if (res) {
		em400_mem_shutdown();
		eerr("Error initializing EM400", res);
	}

	em400_mem_clear();
	mjc400_reset();

	res = mjc400_timer_start();
	if (res) {
		em400_mem_shutdown();
		eerr("Error initializing CPU timer", res);
	}

#ifdef WITH_DEBUGER
	res = em400_debuger_init();
	if (res) {
		em400_debuger_shutdown();
		em400_mem_shutdown();
		eerr("Error initializing debuger", res);
	}
#endif

	while (!em400_quit) {
#ifdef WITH_DEBUGER
		em400_debuger_loop();
		if (em400_quit) {
			break;
		}
#endif
		mjc400_step();
	}

#ifdef WITH_DEBUGER
	em400_debuger_shutdown();
#endif

	em400_mem_shutdown();

	printf("EM400 exits.\n");

	return 0;
}

// vim: tabstop=4
