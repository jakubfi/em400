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
#include <stdio.h>

#include "cpu.h"
#include "memory.h"
#include "io.h"
#include "interrupts.h"
#include "timer.h"
#include "errors.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "debugger/log.h"
#endif

int em400_quit = 0;

// -----------------------------------------------------------------------
static void eerr(const char *message, int ecode)
{
	printf("%s: %s\n", message, get_error(ecode));
	exit(EXIT_FAILURE);
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	int res;

	printf("Starting EM400 version %s ...\n", EM400_VERSION);

	printf("Initializing memory\n");
	res = mem_init();
	if (res != E_OK) {
		mem_shutdown();
		eerr("Error initializing memory", res);
	}

	printf("Initializing I/O\n");
	res = io_init();
	if (res != E_OK) {
		io_shutdown();
		mem_shutdown();
		eerr("Error initializing I/O", res);
	}

	printf("Starting timer\n");
	res = timer_init();
	if (res != E_OK) {
		timer_shutdown();
		io_shutdown();
		mem_shutdown();
		eerr("Error initializing CPU timer", res);
	}

#ifdef WITH_DEBUGGER
	printf("Initializing debugger\n");
	res = dbg_init();
	if (res != E_OK) {
		dbg_shutdown();
		timer_shutdown();
		io_shutdown();
		mem_shutdown();
		eerr("Error initializing debugger", res);
	}
	log_init("em400.log");
#endif

	mem_clear();
	cpu_reset();

	while (em400_quit == 0) {
#ifdef WITH_DEBUGGER
		dbg_step();
		if (em400_quit) break;
#endif
		cpu_step();
		int_serve();
	}

#ifdef WITH_DEBUGGER
	log_shutdown();
	dbg_shutdown();
#endif

	timer_shutdown();
	io_shutdown();
	mem_shutdown();
	printf("EM400 exits.\n");

	return 0;
}

// vim: tabstop=4
