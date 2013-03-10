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

	if (em400_cfg.program_name) {
		printf("Loading image '%s' into OS memory\n", em400_cfg.program_name);
		int res = mem_load_image(em400_cfg.program_name, 0);
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
	printf("   -c config_file     : use config_file instead of default ones\n");
	printf("   -p program_image   : load program image into OS memory\n");
	printf("   -e                 : exit after HLT 077\n");
#ifdef WITH_DEBUGGER
	printf("\nDebuger-only options:\n");
	printf("   -s                 : use simple debugger interface\n");
	printf("   -t test_expression : execute test_expression when program halts (implies -e -s)\n");
	printf("   -x pre_expression  : execute pre_expression before program start\n");
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

	while ((option = getopt(argc, argv,"ec:p:t:x:s")) != -1) {
		switch (option) {
			case 'c':
				em400_cfg.config_file = strdup(optarg);
				break;
			case 'p':
				em400_cfg.program_name = strdup(optarg);
			case 'e':
				em400_cfg.exit_on_hlt = 1;
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
		printf("Trying config file: %s\n", cfgf->name);
		if (cfg_load(cfgf->name) < 0) {
			if (cfgf->name == em400_cfg.config_file) {
				printf("Error loading required config file: %s\n", cfgf->name);
				exit(EXIT_FAILURE);
			}
		} else {
			printf("Config loaded.\n");
			return;
		}
		cfgf++;
	}

	printf("Error loading default config files.\n");
	exit(EXIT_FAILURE);
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
#ifdef WITH_DEBUGGER
	printf("Starting EM400 version %s (with debugger) ...\n", EM400_VERSION);
#else
	printf("Starting EM400 version %s ...\n", EM400_VERSION);
#endif

	parse_arguments(argc, argv);
	em400_configure();
	em400_init();

#ifdef WITH_DEBUGGER
	if (em400_cfg.pre_expr) {
		dbg_parse(em400_cfg.pre_expr);
	}
#endif

	gettimeofday(&ips_start, NULL);

	while (em400_quit == 0) {
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

	gettimeofday(&ips_end, NULL);

#ifdef WITH_DEBUGGER
	if (em400_cfg.autotest == 1) {
		printf("TEST RESULT: ");
		dbg_parse(em400_cfg.test_expr);
		free(em400_cfg.test_expr);
		free(em400_cfg.program_name);
	}
#endif

	em400_shutdown();

	double ips_time_spent = (double)(ips_end.tv_usec - ips_start.tv_usec)/1000000 + (ips_end.tv_sec - ips_start.tv_sec);
	if (ips_time_spent > 0) {
		printf("IPS: %i\n", (int) (ips_counter/ips_time_spent));
	} else {
		printf("IPS: 0\n");
	}

	return 0;
}

// vim: tabstop=4
