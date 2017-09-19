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
#include "atomic.h"
#include "cpu/cpu.h"
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "cpu/timer.h"
#include "io/io.h"

#include "ectl.h"
#include "em400.h"
#include "cfg.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#include "debugger/ui.h"
#endif

#include "log.h"

int em400_console = CONSOLE_NONE;

struct ui *ui;

// -----------------------------------------------------------------------
void em400_shutdown()
{
	if (ui) {
		ui_shutdown(ui);
	} else {
		#ifdef WITH_DEBUGGER
		dbg_shutdown();
		#endif
	}
	ectl_shutdown();
	timer_shutdown();
	io_shutdown();
	cpu_shutdown();
	mem_shutdown();
	log_shutdown();
}

// -----------------------------------------------------------------------
int em400_init(struct cfg_em400 *cfg)
{
	int res;

	res = log_init(cfg);
	if (res != E_OK) {
		return log_err("Failed to initialize logging.");
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
		return log_err("Failed to initialize memory.");
	}

	res = cpu_init(cfg, cfg->ui_name ? 1 : 0);
	if (res != E_OK) {
		return log_err("Failed to initialize CPU.");
	}

	res = timer_init(cfg);
	if (res != E_OK) {
		return log_err("Failed to initialize timer.");
	}

	res = io_init(cfg);
	if (res != E_OK) {
		return log_err("Failed to initialize I/O.");
	}

	rKB = cfg->keys;

	if (cfg->program_name) {
		FILE *f = fopen(cfg->program_name, "rb");
		if (!f) {
			return log_err("Failed to open program file: \"%s\".", cfg->program_name);
		}
		int res = ectl_load(f, cfg->program_name, 0, 0);
		fclose(f);
		if (res < 0) {
			return log_err("Failed to load program file: \"%s\".", cfg->program_name);
		} else {
			LOG(L_EM4H, 1, "OS memory block image loaded: \"%s\", %i words", cfg->program_name, res);
		}
	}

	res = ectl_init();
	if (res != E_OK) {
		return log_err("Failed to initialize ECTL interface.");
	}

	if (cfg->ui_name) {
		ui = ui_create(cfg->ui_name);
		if (!ui) {
			return log_err("Failed to initialize UI.");
		}
	} else {
		#ifdef WITH_DEBUGGER
		res = dbg_init(cfg);
		if (res != E_OK) {
			return log_err("Failed to initialize debugger.");
		}
		#endif
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
		"   -u ui        : EXPERIMENTAL: run specified user interface (available UIs: cmd)\n"
#ifdef WITH_DEBUGGER
		"   -s           : use simple debugger interface\n"
#endif
	);
}

// -----------------------------------------------------------------------
void em400_mkconfdir()
{
	char *conf_dirname = NULL;

	const char *cdir = ".em400";
	char *home = getenv("HOME");
	conf_dirname = malloc(strlen(home) + strlen(cdir) + 2);
	if (conf_dirname) {
		sprintf(conf_dirname, "%s/%s", home, cdir);
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
		// wrong usage
		return NULL;
	}

	// will only print help
	if (cfg_cmdline->print_help) {
		return cfg_cmdline;
	}

	// load configuration from file (there is always a config file, either default one or provided by the user)
	cfg_file = cfg_from_file(cfg_cmdline->cfg_filename);
	if (!cfg_file) {
		log_err("Failed to load config file: \"%s\".", cfg_cmdline->cfg_filename);
		cfg_destroy(cfg_cmdline);
		return NULL;
	}

	// build final configuration by overlaying config file with commandline
	cfg_final = cfg_overlay(cfg_file, cfg_cmdline);
	if (!cfg_final) {
		log_err("Failed to overlay commandline options onto config file: \"%s\".", cfg_cmdline->cfg_filename);
		cfg_destroy(cfg_file);
		cfg_destroy(cfg_cmdline);
		return NULL;
	}

	return cfg_final;
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	int ret = -1;
	em400_mkconfdir();

	struct cfg_em400 *cfg = em400_configure(argc, argv);
	if (!cfg) {
		log_err("Failed to configure EM400.");
		goto done;
	}

	if (cfg->print_help) {
		em400_usage();
		ret = 0;
		goto done;
	}

	int res = em400_init(cfg);
	if (res != E_OK) {
		goto done;
	}

	if (ui) {
		int res = ui_run(ui);
		if (res != E_OK) {
			log_err("Failed to start the UI: %s.", ui->drv->name);
			goto done;
		}
	}

	cpu_loop(ui ? 1 : 0);

	ret = 0;

done:
	em400_shutdown();
	cfg_destroy(cfg);
	return ret;
}

// vim: tabstop=4 shiftwidth=4 autoindent
