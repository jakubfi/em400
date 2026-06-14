//  Copyright (c) 2012-2026 Jakub Filipowicz <jakubf@gmail.com>
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
#include <strings.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libem400.h"
#include "ui/ui.h"
#include "appcfg.h"
#include "cfg.h"
#include "log.h"


// -----------------------------------------------------------------------
int em400_top_init(em400_cfg *cfg)
{
	const char *log_file_name = cfg_getstr(cfg, "log:file", CFG_DEFAULT_LOG_FILE);
	em400_log_buf_type_t log_buf_type =
		cfg_getbool(cfg, "log:line_buffered", CFG_DEFAULT_LOG_LINE_BUFFERED)
		? EM400_LOG_LINE_BUFFERED
		: EM400_LOG_FULL_BUFFERED;
	const char *log_components = cfg_getstr(cfg, "log:components", CFG_DEFAULT_LOG_COMPONENTS);
	bool log_enabled = cfg_getbool(cfg, "log:enabled", CFG_DEFAULT_LOG_ENABLED);

	if (log_init(log_file_name, log_buf_type) != E_OK) {
		return LOGERR("Failed to initialize logging");
	}
	if (log_enabled) {
		if (log_enable() != E_OK) {
			return LOGERR("Failed to enable logging");
		}
	}
	if (log_setup_components(log_components)) {
		return LOGERR("Failed to set which components to log");
	}

	if (appcfg_build_from_ini(cfg) != E_OK) {
		return LOGERR("Failed to build EM400 configuration");
	}

	if (em400_init(appcfg_active_machine(&appcfg), &appcfg.host) != E_OK) {
		return LOGERR("Failed to initialize EM400 core");
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void em400_top_shutdown()
{
	em400_shutdown();
	appcfg_free();
	log_shutdown();
}

// -----------------------------------------------------------------------
void em400_usage()
{
	fprintf(stdout, "EM400 version %s\n", em400_version());
	fprintf(stdout,
		"Usage: em400 [option] ...\n"
		"\n"
		"Options:\n"
		"   -h               : Display help\n"
		"   -c config        : Config file to use instead of the default one (~/.em400/em400.cfg)\n"
		"   -p program       : Load program image into OS memory at address 0\n"
		"   -l cmp,cmp,...   : Enable logging for specified components. Available components:\n"
		"                      reg, mem, cpu, op, int, io, mx, px, cchar, cmem, term\n"
		"                      wnch, flop, pnch, pnrd, tape, crk5, em4h, all\n"
		"   -L               : Disable logging\n"
		"   -u ui            : User interface to use. Available UIs (first is the default):"
	);
	ui_print_uis(stdout);
	fprintf(stdout, "\n");
	fprintf(stdout,
		"   -O sec:key=value : Override configuration entry \"key\" in section [sec] with a specific value\n"
	);
}

// -----------------------------------------------------------------------
void em400_mkconfdir()
{
	const char *home = getenv("HOME");
	const char *cdir = "/.em400";
	char *conf_dirname = (char *) malloc(strlen(home) + strlen(cdir) + 1);
	if (conf_dirname) {
		sprintf(conf_dirname, "%s%s", home, cdir);
		mkdir(conf_dirname, 0700);
		free(conf_dirname);
	}
}

const char em400_cmdline_opts[] = "hc:p:l:Lu:O:";

// -----------------------------------------------------------------------
int em400_cmdline_1(int argc, char **argv, int *print_help, char **config)
{
	int option;

	while ((option = getopt(argc, argv, em400_cmdline_opts)) != -1) {
		switch (option) {
			case 'h':
				*print_help = 1;
				break;
			case 'c':
				*config = strdup(optarg);
				break;
            case 'p':
            case 'L':
            case 'l':
            case 'u':
			case 'O':
				break;
            default:
                return E_ERR;
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int em400_cmdline_2(em400_cfg *cfg, int argc, char **argv, const char **program)
{
    int option;
    optind = 1; // reset to 1 so consecutive calls work

    while ((option = getopt(argc, argv, em400_cmdline_opts)) != -1) {
        switch (option) {
            case 'h':
            case 'c':
                break;
            case 'p':
                *program = optarg;
                break;
            case 'L':
				cfg_set(cfg, "log:enabled", "false");
                break;
            case 'l':
				cfg_set(cfg, "log:enabled", "true");
				cfg_set(cfg, "log:components", optarg);
                break;
            case 'u':
                cfg_set(cfg, "ui:interface", optarg);
                break;
			case 'O':
				const char *colon = strchr(optarg, ':');
				const char *key = strtok(optarg, "=");
				const char *val = strtok(NULL, "=");
				if (!colon || !key || !val) {
					return LOGERR("Syntax error for -O switch. Should be: -O section:key=value");
				}
				cfg_set(cfg, key, val);
				break;
            default:
                return E_ERR;
        }
    }

    return E_OK;
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	int return_code = -1;

	int print_help = 0;
	char *config = NULL;
	const char *program = NULL;
	char *ui_name = NULL;
	em400_cfg *cfg = NULL;
	struct ui *ui = NULL;

	em400_mkconfdir();

	if (em400_cmdline_1(argc, argv, &print_help, &config) != E_OK) {
		LOGERR("Failed to parse commandline arguments (pass 1)");
		goto done;
	}

	if (print_help) {
		em400_usage();
		return_code = 0;
		goto done;
	}

	if (!config) {
		const char *home = getenv("HOME");
		const char *cfile = "/.em400/em400.ini";
		config = (char *) malloc(strlen(home) + strlen(cfile) + 1);
		if (!config) {
			LOGERR("Config filename memory allocation error");
			goto done;
		}
		sprintf(config, "%s%s", home, cfile);
	}

	cfg = cfg_load(config);
	if (!cfg) {
		LOGERR("Failed to load config file: %s", config);
		goto done;
	}

	// read the commandline again to build final configuration
	if (em400_cmdline_2(cfg, argc, argv, &program) != E_OK) {
		LOGERR("Failed to parse commandline arguments (pass 2)");
		goto done;
	}

	if (em400_top_init(cfg) != E_OK) {
		LOGERR("Failed to initialize EM400");
		goto done;
	}

	// -p is session-only load, kept out of cfg so it can't be persisted
	if (program && !em400_load_os_image_path(program)) {
		LOGERR("Preloading OS memory failed: %s", program);
		goto done;
	}

	const char *ui_iface = cfg_getstr(cfg, "ui:interface", NULL);
	if (ui_iface) {
		ui_name = strdup(ui_iface);
	}

	// nothing needs cfg anymore
	cfg_free(cfg);
	cfg = NULL;

	if (!(ui = ui_create(ui_name))) {
		LOGERR("Failed to initialize UI");
		goto done;
	}

	if (ui_run(ui) != E_OK) {
		LOGERR("Failed to start the UI: %s", ui->drv->name);
		goto done;
	}

	return_code = 0;

done:
	ui_shutdown(ui);
	em400_top_shutdown();
	cfg_free(cfg);
	free(ui_name);
	free(config);
	return return_code;
}

// vim: tabstop=4 shiftwidth=4 autoindent
