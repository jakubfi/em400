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
#include <stdarg.h>
#include <sys/time.h>

#include "em400.h"
#include "cfg.h"
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
unsigned int ips_counter = 0;
struct timeval ips_start;
struct timeval ips_end;

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
	eprint("EM400 exits.\n");
}

// -----------------------------------------------------------------------
void eerr(char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	em400_shutdown();
	exit(EXIT_FAILURE);
}

// -----------------------------------------------------------------------
void em400_init()
{
	int res;

	res = mem_init();
	if (res != E_OK) {
		eerr("Error initializing memory: %s\n", get_error(res));
	}

	res = io_init();
	if (res != E_OK) {
		eerr("Error initializing I/O: %s\n", get_error(res));
	}

	res = timer_init();
	if (res != E_OK) {
		eerr("Error initializing CPU timer: %s\n", get_error(res));
	}

	mem_clear();
	cpu_reset();

	if (em400_cfg.program_name) {
		eprint("Loading image '%s' into OS memory\n", em400_cfg.program_name);
		int res = mem_load_image(em400_cfg.program_name, 0);
		if (res != E_OK) {
			eerr("Could not load program '%s': %s\n", em400_cfg.program_name, get_error(res));
		}
	}

#ifdef WITH_DEBUGGER
	res = dbg_init();
	if (res != E_OK) {
		eerr("Error initializing debugger: %s\n", get_error(res));
	}
	res = log_init("em400.log");
	if (res != E_OK) {
		eerr("Error initializing logging: %s\n", get_error(res));
	}
#endif

}

// -----------------------------------------------------------------------
void print_usage()
{
	printf("Usage: em400 [option] ...\n");
	printf("\nOptions:\n");
	printf("   -h           : display help\n");
	printf("   -c config    : use given config file instead of defaults\n");
	printf("   -p program   : load program image into OS memory\n");
	printf("   -e           : exit emulator after HLT 077\n");
	printf("   -b           : benchmark emulator\n");
	printf("   -v           : enable verbose messages\n");
#ifdef WITH_DEBUGGER
	printf("\nDebuger-only options:\n");
	printf("   -s           : use simple debugger interface\n");
	printf("   -t test_expr : execute expression when program halts (implies -e -s)\n");
	printf("   -x pre_expr  : execute expression on emulator startup\n");
#endif
}

// -----------------------------------------------------------------------
void parse_arguments(int argc, char **argv)
{
	int option;

#ifdef WITH_DEBUGGER
	int len;
	em400_cfg.pre_expr = NULL;
#endif

	while ((option = getopt(argc, argv,"bvhec:p:t:x:s")) != -1) {
		switch (option) {
			case 'b':
				em400_cfg.benchmark = 1;
				break;
			case 'v':
				em400_cfg.verbose = 1;
				break;
			case 'h':
				print_usage();
				exit(0);
			case 'c':
				em400_cfg.config_file = strdup(optarg);
				break;
			case 'p':
				em400_cfg.program_name = strdup(optarg);
				break;
			case 'e':
				em400_cfg.exit_on_hlt = 1;
#ifdef WITH_DEBUGGER
				em400_cfg.autotest = 1;
#endif
				break;
#ifdef WITH_DEBUGGER
			case 't':
				em400_cfg.autotest = 1;
				em400_cfg.ui_simple = 1;
				em400_cfg.exit_on_hlt = 1;
				len = strlen(optarg);
				em400_cfg.test_expr = malloc(len+3);
				strcpy(em400_cfg.test_expr, optarg);
				strcpy(em400_cfg.test_expr + len, "\n\0");
				break;
			case 'x':
				len = strlen(optarg);
				em400_cfg.pre_expr = malloc(len+3);
				strcpy(em400_cfg.pre_expr, optarg);
				strcpy(em400_cfg.pre_expr + len, "\n\0");
				break;
			case 's':
				em400_cfg.ui_simple = 1;
				break;
#endif
			default:
				print_usage();
				exit(1);
		}
	}
}

// -----------------------------------------------------------------------
void em400_configure()
{
	// default configuration
	em400_cfg.cpu.speed_real = 0;
	em400_cfg.cpu.timer_step = 10;
	em400_cfg.cpu.mod_17bit = 1;
	em400_cfg.cpu.mod_sint = 1;

	// config files to consider
	struct cfgfile_t {
		char *name;
	} cfgfile[10];
	struct cfgfile_t *cfgf = cfgfile;

	// user-provided config file
	if (em400_cfg.config_file) {
		cfgf++->name = em400_cfg.config_file;
	}

	// prepare default config file locations
	char homefile[1024];
	sprintf(homefile, "%s/.em400/em400.cfg", getenv("HOME"));
	cfgf++->name = "em400.cfg";
	cfgf++->name = homefile;
	cfgf++->name = "/etc/em400/em400.cfg";
	cfgf++->name = NULL;

	// try to load one of configuration files
	cfgf = cfgfile;
	while (cfgf->name) {
		int res = cfg_load(cfgf->name);
		if (res != E_OK) {
			printf("Failed to load config '%s'\n", cfgf->name);
			if (cfgf->name == em400_cfg.config_file) {
				eerr("Error loading user config file\n");
			}
		} else {
			eprint("Config loaded\n");
			return;
		}
		cfgf++;
	}

	eerr("Error loading default config files\n");
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	parse_arguments(argc, argv);

#ifdef WITH_DEBUGGER
	printf("EM400 v%s (+debugger)\n", EM400_VERSION);
#else
	printf("EM400 v%s\n", EM400_VERSION);
#endif

	em400_configure();
	cfg_print();
	em400_init();

#ifdef WITH_DEBUGGER
	if (em400_cfg.pre_expr) {
		dbg_parse(em400_cfg.pre_expr);
	}
#endif

	gettimeofday(&ips_start, NULL);

	while (em400_quit == E_OK) {
#ifdef WITH_DEBUGGER
		if (em400_cfg.autotest != 1) {
			dbg_step();
			if (em400_quit) {
				break;
			}
		}
#endif
		cpu_step();
		ips_counter++;
		int_serve();
	}

	if (em400_quit != E_QUIT_OK) {
		eerr("Emulation terminated unexpectedly: %s\n", get_error(em400_quit));
	}

	gettimeofday(&ips_end, NULL);

#ifdef WITH_DEBUGGER
	if (em400_cfg.autotest && em400_cfg.test_expr) {
		printf("TEST RESULT: ");
		dbg_parse(em400_cfg.test_expr);
		free(em400_cfg.test_expr);
		free(em400_cfg.program_name);
	}
#endif

	em400_shutdown();

	if (em400_cfg.benchmark) {
		double ips_time_spent = (double)(ips_end.tv_usec - ips_start.tv_usec)/1000000 + (ips_end.tv_sec - ips_start.tv_sec);
		if (ips_time_spent > 0) {
			printf("IPS: %i (instructions: %i time: %f)\n", (int) (ips_counter/ips_time_spent), ips_counter, ips_time_spent);
		} else {
			printf("IPS: 0\n");
		}
	}

	return 0;
}

// vim: tabstop=4
