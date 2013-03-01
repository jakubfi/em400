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
#include <string.h>
#include <unistd.h>

#include "em400.h"
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
void em400_shutdown()
{
#ifdef WITH_DEBUGGER
	log_shutdown();
	dbg_shutdown();
#endif
	timer_shutdown();
	io_shutdown();
	mem_shutdown();
	printf("EM400 exits.\n");
}

// -----------------------------------------------------------------------
void eerr(const char *message, int ecode)
{
	printf("%s: %s\n", message, get_error(ecode));
	em400_shutdown();
	exit(EXIT_FAILURE);
}

// -----------------------------------------------------------------------
void em400_init()
{
	int res;

	printf("Starting EM400 version %s ...\n", EM400_VERSION);

	printf("Initializing memory\n");
	res = mem_init();
	if (res != E_OK) {
		eerr("Error initializing memory", res);
	}

	printf("Initializing I/O\n");
	res = io_init();
	if (res != E_OK) {
		eerr("Error initializing I/O", res);
	}

	printf("Starting timer\n");
	res = timer_init();
	if (res != E_OK) {
		eerr("Error initializing CPU timer", res);
	}

	mem_clear();
	cpu_reset();

	if (em400_opts.program_name) {
		printf("Loading image '%s' into OS memory\n", em400_opts.program_name);
		int res = mem_load_image(em400_opts.program_name, 0);
		if (res != E_OK) {
			eerr("Could not load program", res);
		}
	}

#ifdef WITH_DEBUGGER
	printf("Initializing debugger\n");
	res = dbg_init();
	if (res != E_OK) {
		eerr("Error initializing debugger", res);
	}
	printf("Initializing logging\n");
	res = log_init("em400.log");
	if (res != E_OK) {
		eerr("Error initializing logging", res);
	}
#endif

}

// -----------------------------------------------------------------------
void print_usage()
{
	printf("Usage: em400 [option] ...\n");
	printf("\nOptions:\n");
	printf("   -p program_image   : load program image into OS memory\n");
#ifdef WITH_DEBUGGER
	printf("\nDebuger-only options:\n");
	printf("   -s                 : use simple debugger interface\n");
	printf("   -t test_expression : execute test_expression after HLT 077 (implies -s)\n");
	printf("   -x pre_expression  : execute pre_expression before program start\n");
#endif
}

// -----------------------------------------------------------------------
void parse_arguments(int argc, char **argv)
{
	int option;
	int len;

	em400_opts.pre_expr = NULL;

    while ((option = getopt(argc, argv,"c:p:t:x:s")) != -1) {
        switch (option) {
			case 'c':
				em400_opts.config_file = strdup(optarg);
				break;
			case 'p':
				em400_opts.program_name = strdup(optarg);
				break;
#ifdef WITH_DEBUGGER
			case 't':
				em400_opts.autotest = 1;
				em400_opts.ui_simple = 1;
				len = strlen(optarg);
				em400_opts.test_expr = malloc(len+3);
				strcpy(em400_opts.test_expr, optarg);
				strcpy(em400_opts.test_expr + len, "\n\0");
				break;
			case 'x':
				len = strlen(optarg);
				em400_opts.pre_expr = malloc(len+3);
				strcpy(em400_opts.pre_expr, optarg);
				strcpy(em400_opts.pre_expr + len, "\n\0");
				break;
			case 's':
				em400_opts.ui_simple = 1;
				break;
#endif
            default:
				print_usage();
                exit(1);
        }
    }
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	parse_arguments(argc, argv);
	em400_init();

#ifdef WITH_DEBUGGER
	if (em400_opts.pre_expr) {
		dbg_parse(em400_opts.pre_expr);
	}
#endif

	while (em400_quit == 0) {
#ifdef WITH_DEBUGGER
		if (em400_opts.autotest != 1) {
			dbg_step();
			if (em400_quit) {
				break;
			}
		}
#endif
		cpu_step();
		int_serve();
	}

#ifdef WITH_DEBUGGER
	if (em400_opts.autotest == 1) {
		printf("TEST RESULT: ");
		dbg_parse(em400_opts.test_expr);
		free(em400_opts.test_expr);
		free(em400_opts.program_name);
	}
#endif

	em400_shutdown();
	return 0;
}

// vim: tabstop=4
