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
#include <sys/stat.h>
#include <sys/types.h>

#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "cpu/timer.h"
#include "io/io.h"

#include "em400.h"
#include "cfg.h"
#include "errors.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "debugger/log.h"
#endif

int em400_console = CONSOLE_NONE;
int em400_state = STATE_WORK;

// -----------------------------------------------------------------------
void em400_shutdown()
{
#ifdef WITH_DEBUGGER
	log_close();
	dbg_shutdown();
#endif
	timer_shutdown();
	io_shutdown();
	cpu_shutdown();
	mem_shutdown();
	eprint("EM400 exits.\n");
}

// -----------------------------------------------------------------------
void em400_eerr(int err_code, char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf(": %s\n", get_error(err_code));
	em400_shutdown();
	exit(EXIT_FAILURE);
}

// -----------------------------------------------------------------------
void em400_init()
{
	int res;

#ifdef WITH_DEBUGGER
	em400_console = CONSOLE_DEBUGGER;
#endif

	res = mem_init();
	if (res != E_OK) {
		em400_eerr(res, "Error initializing memory");
	}

	res = cpu_init();
	if (res != E_OK) {
		em400_eerr(res, "Error initializing CPU");
	}

	res = timer_init();
	if (res != E_OK) {
		em400_eerr(res, "Error initializing CPU timer");
	}

	res = io_init();
	if (res != E_OK) {
		em400_eerr(res, "Error initializing I/O");
	}

	regs[R_KB] = em400_cfg.keys;

	if (em400_cfg.program_name) {
		eprint("Loading image '%s' into OS memory\n", em400_cfg.program_name);
		int res = mem_load(em400_cfg.program_name, 0, 0, 2*4096);
		if (res < E_OK) {
			em400_eerr(res, "Could not load program '%s'", em400_cfg.program_name);
		} else {
			printf("OS memory block image loaded: \"%s\", %i words\n", em400_cfg.program_name, res);
		}
	}

#ifdef WITH_DEBUGGER
	res = dbg_init();
	if (res != E_OK) {
		em400_eerr(res, "Error initializing debugger");
	}
#endif

}

// -----------------------------------------------------------------------
void em400_usage()
{
	printf("Usage: em400 [option] ...\n");
	printf("\nOptions:\n");
	printf("   -h           : display help\n");
	printf("   -c config    : use given config file instead of defaults\n");
	printf("   -p program   : load program image into OS memory\n");
	printf("   -k value     : set keys to given value\n");
	printf("   -e           : terminate emulation on HLT >= 040\n");
	printf("   -b           : benchmark emulator\n");
	printf("   -v           : enable verbose messages\n");
#ifdef WITH_DEBUGGER
	printf("\nDebuger-only options:\n");
	printf("   -s           : use simple debugger interface\n");
	printf("   -l script    : load and execute script on startup\n");
	printf("   -t test_expr : execute expression when program halts (implies -e -s)\n");
	printf("   -x pre_expr  : execute expression on emulator startup\n");
	printf("   -d           : enable full debug logging\n");
#endif
}

// -----------------------------------------------------------------------
void em400_parse_args(int argc, char **argv)
{
	int option;

	em400_cfg.keys = 0;
#ifdef WITH_DEBUGGER
	int len;
	em400_cfg.pre_expr = NULL;
	em400_cfg.test_expr = NULL;
#endif

	while ((option = getopt(argc, argv,"bvhec:p:k:l:t:x:sd")) != -1) {
		switch (option) {
			case 'b':
				em400_cfg.benchmark = 1;
				break;
			case 'v':
				em400_cfg.verbose = 1;
				break;
			case 'h':
				em400_usage();
				exit(0);
			case 'c':
				em400_cfg.cfg_provided = strdup(optarg);
				break;
			case 'p':
				em400_cfg.program_name = strdup(optarg);
				break;
			case 'k':
				em400_cfg.keys = atoi(optarg);
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
			case 'l':
				script_name = strdup(optarg);
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
			case 'd':
				log_setlevel(-1, 100);
				log_open("em400.log");
				log_enable();
				break;
#endif
			default:
				em400_usage();
				exit(1);
		}
	}
}

// -----------------------------------------------------------------------
void em400_configure()
{
	// default configuration
	em400_cfg.speed_real = 0;
	em400_cfg.timer_step = 10;
	em400_cfg.cpu_mod = 0;
	em400_cfg.cpu_user_io_illegal = 1;
	em400_cfg.cpu_awp = 1;
	em400_cfg.mem_elwro = 1;
	em400_cfg.mem_mega = 0;
	em400_cfg.mem_os = 2;
	em400_cfg.chans = NULL;
	em400_cfg.mem_mega_prom = NULL;

	// ~/.em400/ files
	char *home = getenv("HOME");
	int home_len = strlen(home);
	em400_cfg.cfg_dir = calloc(1, home_len+100);
	em400_cfg.cfg_file = calloc(1, home_len+100);
	em400_cfg.hist_file = calloc(1, home_len+100);
	sprintf(em400_cfg.cfg_dir, "%s/.em400", home);
	sprintf(em400_cfg.cfg_file, "%s/.em400/em400.cfg", home);
	sprintf(em400_cfg.hist_file, "%s/.em400/history", home);
	mkdir(em400_cfg.cfg_dir, 0700);

	// config files to consider
	char *cfgf[4];

	// if user wants specific config file - use it
	if (em400_cfg.cfg_provided) {
		cfgf[0] = em400_cfg.cfg_provided;
		cfgf[1] = NULL;
	// otherwise, search for known configs
	} else {
		cfgf[0] = em400_cfg.cfg_file;
		cfgf[1] = "em400.cfg";
		cfgf[2] = "/etc/em400/em400.cfg";
		cfgf[3] = NULL;
	}

	// try to load one of configuration files
	char **cfgfile = cfgf;
	while (*cfgfile) {
		int res = cfg_load(*cfgfile);
		if (res == E_OK) {
			printf("Using config: %s\n", *cfgfile);
			return;
		}
		cfgfile++;
	}

	em400_eerr(E_DEFAULT, "Cannot find any usable config file");
}

// -----------------------------------------------------------------------
void em400_loop()
{
	unsigned int ips_counter = 0;
	struct timeval ips_start;
	struct timeval ips_end;
	double ips_time_spent;
	int ips = 0;

	gettimeofday(&ips_start, NULL);

	while (em400_state == STATE_WORK) {
#ifdef WITH_DEBUGGER
		if (em400_cfg.autotest != 1) {
			dbg_step();
			if (em400_state != STATE_WORK) {
				break;
			}
		}
#endif
		cpu_step();

		if (!P && !regs[R_MODc] && !sem_trywait(&int_ready)) {
			int_serve();
		}

		ips_counter++;
	}

	gettimeofday(&ips_end, NULL);

	if (em400_cfg.benchmark) {
		ips_time_spent = (double)(ips_end.tv_usec - ips_start.tv_usec)/1000000 + (ips_end.tv_sec - ips_start.tv_sec);
		if (ips_time_spent > 0) {
			ips = ips_counter/ips_time_spent;
		}
		printf("IPS: %i (instructions: %i time: %f)\n", ips, ips_counter, ips_time_spent);
	}
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	static const char *features[] = {
#if defined(WITH_DEBUGGER)
		"debugger",
#endif
#if defined(HAVE_DASM)
		"dissassembler",
#endif
		NULL
	};
	const char **ft;

	printf("EM400 v%s", EM400_VERSION);
	if (*features) {
		printf(" ");
		for (ft = features; *ft; ft++) {
			if (ft == features) {
				printf("(");
			} else {
				printf(" ");
			}
			printf("+%s", *ft);
		}
		printf(")");
	}
	printf("\n");

	em400_parse_args(argc, argv);
	em400_configure();
	cfg_print();
	em400_init();

#ifdef WITH_DEBUGGER
	if (em400_cfg.pre_expr) {
		dbg_parse(em400_cfg.pre_expr);
	}
#endif

	em400_loop();

	if (em400_state == STATE_MEM_FAIL) {
		printf("Emulation died, guest CPU segmentation fault.\n");
	}

#ifdef WITH_DEBUGGER
	if (em400_cfg.autotest && em400_cfg.test_expr) {
		printf("TEST RESULT @ 0x%04x, regs: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x: ", cycle_ic, regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7]);
		dbg_parse(em400_cfg.test_expr);
		free(em400_cfg.test_expr);
		free(em400_cfg.program_name);
	}
#endif

	em400_shutdown();

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
