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

#include "atomic.h"
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
#endif

#include "log.h"

int em400_console = CONSOLE_NONE;

// -----------------------------------------------------------------------
void em400_shutdown()
{
#ifdef WITH_DEBUGGER
	dbg_shutdown();
#endif
	timer_shutdown();
	io_shutdown();
	cpu_shutdown();
	mem_shutdown();
	log_shutdown();
}

// -----------------------------------------------------------------------
void em400_exit_error(int err_code, char *format, ...)
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
void em400_init(struct cfg_em400 *cfg)
{
	int res;

	res = log_init(cfg);
	if (res != E_OK) {
		em400_exit_error(res, "Error initializing logging");
	}

	if (LOG_WANTS(L_EM4H, 2)) {
		log_log(L_EM4H, 2, "---- Effective configuration: ----------");
		log_config(L_EM4H, 2, cfg);
		log_log(L_EM4H, 2, "----------------------------------------");
	}

#ifdef WITH_DEBUGGER
	em400_console = CONSOLE_DEBUGGER;
#endif

	res = mem_init(cfg);
	if (res != E_OK) {
		em400_exit_error(res, "Error initializing memory");
	}

	res = cpu_init(cfg);
	if (res != E_OK) {
		em400_exit_error(res, "Error initializing CPU");
	}

	res = timer_init(cfg);
	if (res != E_OK) {
		em400_exit_error(res, "Error initializing CPU timer");
	}

	res = io_init(cfg);
	if (res != E_OK) {
		em400_exit_error(res, "Error initializing I/O");
	}

	regs[R_KB] = cfg->keys;

	if (cfg->program_name) {
		int res = mem_load(cfg->program_name, 0, 0, 2*4096);
		if (res < E_OK) {
			em400_exit_error(res, "Could not load program '%s'", cfg->program_name);
		} else {
			LOG(L_EM4H, 1, "OS memory block image loaded: \"%s\", %i words", cfg->program_name, res);
		}
	}

#ifdef WITH_DEBUGGER
	res = dbg_init(cfg);
	if (res != E_OK) {
		em400_exit_error(res, "Error initializing debugger");
	}
#endif

}

// -----------------------------------------------------------------------
void em400_usage()
{
	printf(
		"Usage: em400 [option] ...\n"
		"\n"
		"Options:\n"
		"   -h           : display help\n"
		"   -c config    : use given config file instead of defaults\n"
		"   -p program   : load program image into OS memory\n"
		"   -l levels    : enable logging with given levels\n"
		"                  levels syntax: component=level[,component=level[,..]]\n"
		"                  components: reg, mem, cpu, op, int, io, mx, px, cchar, cmem,\n"
		"                              term, wnch, flop, pnch, pnrd, crk5, em4h, all\n"
		"                  level: 0-9\n"
		"   -L           : disable logging\n"
		"   -k value     : set keys to given value\n"
		"   -e           : terminate emulation on HLT >= 040\n"
		"   -b           : benchmark emulator\n"
#ifdef WITH_DEBUGGER
		"\n"
		"Debuger-only options:\n"
		"   -s           : use simple debugger interface\n"
		"   -t test_expr : execute expression when program halts (implies -e -s)\n"
#endif
	);
}

// -----------------------------------------------------------------------
void em400_loop(struct cfg_em400 *cfg)
{
	unsigned int ips_counter = 0;
	struct timeval ips_start;
	struct timeval ips_end;
	double ips_time_spent;
	unsigned int ips = 0;

	gettimeofday(&ips_start, NULL);
#ifdef WITH_DEBUGGER
	ips_counter = cpu_loop(cfg->autotest);
#else
	ips_counter = cpu_loop(0);
#endif
	gettimeofday(&ips_end, NULL);

	if (cfg->benchmark) {
		ips_time_spent = (double)(ips_end.tv_usec - ips_start.tv_usec)/1000000 + (ips_end.tv_sec - ips_start.tv_sec);
		if (ips_time_spent > 0) {
			ips = ips_counter/ips_time_spent;
		}
		printf("IPS: %i (instructions: %i time: %f)\n", ips, ips_counter, ips_time_spent);
	}
}

// -----------------------------------------------------------------------
void em400_mkconfdir()
{
	char *conf_dirname = NULL;

	const char *cdir = "/.em400";
	char *home = getenv("HOME");
	conf_dirname = malloc(strlen(home) + strlen(cdir) + 1);
	if (conf_dirname) {
		sprintf(conf_dirname, "%s%s", home, cdir);
		mkdir(conf_dirname, 0700);
		free(conf_dirname);
	}
}

// -----------------------------------------------------------------------
struct cfg_em400 * em400_configure(int argc, char** argv)
{
	struct cfg_em400 *cfg_cmdline = NULL;
	struct cfg_em400 *cfg_file = NULL;
	struct cfg_em400 *cfg_final = NULL;

	// parse commandline first, because:
	//  * user may need help (-h)
	//  * user may provide own config file
	cfg_cmdline = cfg_from_args(argc, argv);
	if (!cfg_cmdline) {
		// wrong usage (print help), or -h (print help), exit anyway
		em400_usage();
		goto cleanup;
	}

	// load configuration from file
	cfg_file = cfg_from_file(cfg_cmdline->cfg_filename);
	if (!cfg_file) {
		printf("Could not load config file: %s\n", cfg_cmdline->cfg_filename);
		goto cleanup;
	}

	// build final configuration by overlaying config file with commandline
	cfg_final = cfg_overlay(cfg_file, cfg_cmdline);

	if (!cfg_final) {
		cfg_destroy(cfg_file);
	}

cleanup:
	cfg_destroy(cfg_cmdline);

	return cfg_final;
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	printf("EM400 v%s ", EM400_VERSION);
#ifdef WITH_DEBUGGER
	printf("+debugger ");
#endif
	printf("\n");

	em400_mkconfdir();

	struct cfg_em400 *cfg = em400_configure(argc, argv);

	if (!cfg) {
		exit(EXIT_FAILURE);
	}

	if (cfg->print_help) {
		em400_usage();
		cfg_destroy(cfg);
		exit(0);
	}

	em400_init(cfg);
	em400_loop(cfg);

	if (atom_load(&cpu_state) == STATE_STOP) {
		printf("Emulation died, guest CPU segmentation fault.\n");
	}

#ifdef WITH_DEBUGGER
	if (cfg->autotest && cfg->test_expr) {
		printf("TEST RESULT @ 0x%04x, regs: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x: ", regs[R_IC], regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7]);
		dbg_parse(cfg->test_expr);
	}
#endif

	em400_shutdown();
	cfg_destroy(cfg);

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
