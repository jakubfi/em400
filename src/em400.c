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

#include "ui/ui.h"
#include "cpu/cp.h"
#include "cpu/cpu.h"
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "cpu/clock.h"
#include "io/io.h"
#include "fpga/iobus.h"

#include "em400.h"
#include "cfg.h"

#include "log.h"

int em400_console = CONSOLE_NONE;

struct ui *ui;

// -----------------------------------------------------------------------
void em400_shutdown()
{
	ui_shutdown(ui);
	ectl_shutdown();
	clock_shutdown();
	io_shutdown();
	cpu_shutdown();
	cp_shutdown();
	mem_shutdown();
	log_shutdown();
}

// -----------------------------------------------------------------------
int em400_init(struct cfg_em400 *cfg)
{
	int res;

	res = log_init(cfg);
	if (res != E_OK) {
		return LOGERR("Failed to initialize logging.");
	}

	// log configuration as early as possible
	cfg_log(cfg);

	// TODO: ???
	em400_console = CONSOLE_DEBUGGER;

	if (cfg->fpga) {
		if (iob_init(cfg->fpga_dev, cfg->fpga_speed) == E_ERR) {
			return LOGERR("Failed to set up FPGA I/O bus.");
		}
	}

	res = mem_init(cfg);
	if (res != E_OK) {
		return LOGERR("Failed to initialize memory.");
	}

	res = cp_init(cfg);
	if (res != E_OK) {
		return LOGERR("Failed to initialize control panel.");
	}

	res = cpu_init(cfg);
	if (res != E_OK) {
		return LOGERR("Failed to initialize CPU.");
	}

	res = clock_init(cfg);
	if (res != E_OK) {
		return LOGERR("Failed to initialize clock.");
	}

	res = io_init(cfg);
	if (res != E_OK) {
		return LOGERR("Failed to initialize I/O.");
	}

	rKB = cfg->keys;

	res = ectl_init();
	if (res != E_OK) {
		return LOGERR("Failed to initialize ECTL interface.");
	}

	if (cfg->program_name) {
		FILE *f = fopen(cfg->program_name, "rb");
		if (!f) {
			return LOGERR("Failed to open program file: \"%s\".", cfg->program_name);
		}
		int res = ectl_load(f, cfg->program_name, 0, 0);
		fclose(f);
		if (res < 0) {
			return LOGERR("Failed to load program file: \"%s\".", cfg->program_name);
		} else {
			LOG(L_EM4H, "OS memory block image loaded: \"%s\", %i words", cfg->program_name, res);
		}
	}

	ui = ui_create(cfg->ui_name);
	if (!ui) {
		return LOGERR("Failed to initialize UI %s.", cfg->ui_name);
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void em400_usage()
{
	fprintf(stdout,
		"EM400 version " EM400_VERSION "\n"
		"Usage: em400 [option] ...\n"
		"\n"
		"Options:\n"
		"   -h                : display help\n"
		"   -c config         : config file to use instead of ~/.em400/em400.cfg\n"
		"   -p program        : load program image into OS memory at address 0\n"
		"   -l cmp[,cmp[,...] : enable logging for specified components, allowed are:\n"
		"                       reg, mem, cpu, op, int, io, mx, px, cchar, cmem, term\n"
		"                       wnch, flop, pnch, pnrd, tape, crk5, em4h, ectl, fpga, all\n"
		"   -L                : disable logging\n"
		"   -k value          : value to initially set keys to\n"
		"   -u ui             : user interface to use (available UIs: cmd, curses. Default: curses)\n"
		"   -F                : use FPGA implementation of the CPU and external memory\n"
	);
}

// -----------------------------------------------------------------------
void em400_mkconfdir()
{
	char *conf_dirname = NULL;

	const char *cdir = ".em400";
	char *home = getenv("HOME");
	conf_dirname = (char *) malloc(strlen(home) + strlen(cdir) + 2);
	if (conf_dirname) {
		sprintf(conf_dirname, "%s/%s", home, cdir);
		mkdir(conf_dirname, 0700);
		free(conf_dirname);
	}
}

// -----------------------------------------------------------------------
struct cfg_em400 * em400_configure(int argc, char** argv)
{
	struct cfg_em400 *cfg = cfg_create_default();

	// first, parse the commandline, because:
	//  * user may need help (-h)
	//  * user may provide non-default config file
	if (cfg_from_args(cfg, argc, argv)) {
		goto fail;
	}

	// will only print help
	if (cfg->print_help) {
		return cfg;
	}

	// load configuration from a file (either the default one or provided by the user)
	if (cfg_from_file(cfg)) {
		LOGERR("Failed to load config file: \"%s\".", cfg->cfg_filename);
		goto fail;
	}

	// read the commendline again to build final configuration
	if (cfg_from_args(cfg, argc, argv)) {
		goto fail;
	}

	return cfg;

fail:
	cfg_destroy(cfg);
	return NULL;
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	int res;
	int ret = -1;
	em400_mkconfdir();

	struct cfg_em400 *cfg = em400_configure(argc, argv);
	if (!cfg) {
		LOGERR("Failed to configure EM400.");
		goto done;
	}

	if (cfg->print_help) {
		em400_usage();
		ret = 0;
		goto done;
	}

	res = em400_init(cfg);
	if (res != E_OK) {
		goto done;
	}

	res = ui_run(ui);
	if (res != E_OK) {
		LOGERR("Failed to start the UI: %s.", ui->drv->name);
		goto done;
	}

	if (cfg->fpga) {
		iob_loop();
	} else {
		cpu_loop();
	}

	ret = 0;

done:
	em400_shutdown();
	cfg_destroy(cfg);
	return ret;
}

// vim: tabstop=4 shiftwidth=4 autoindent
