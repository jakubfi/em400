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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "mem/mem.h"

#include "errors.h"
#include "cfg.h"
#include "cfg_parser.h"

extern FILE *cyyin;
void cyyerror(char *s, ...);
int cyyparse();
int cyylex_destroy();
int cfg_error = 0;

// -----------------------------------------------------------------------
int cfg_default()
{
	em400_cfg.program_name = NULL;
	em400_cfg.cfg_provided = NULL;
	em400_cfg.exit_on_hlt = 0;
	em400_cfg.verbose = 0;
	em400_cfg.benchmark = 0;
	em400_cfg.cfg_dir = NULL;
	em400_cfg.home_cfg_file = NULL;
	em400_cfg.hist_file = NULL;

	em400_cfg.speed_real = 0;
	em400_cfg.timer_step = 10;
	em400_cfg.timer_start = 1;
	em400_cfg.cpu_mod = 0;
	em400_cfg.cpu_user_io_illegal = 1;
	em400_cfg.cpu_awp = 1;
	em400_cfg.keys = 0;
	em400_cfg.mem_elwro = 1;
	em400_cfg.mem_mega = 0;
	em400_cfg.mem_mega_prom = NULL;
	em400_cfg.mem_mega_boot = 0;
	em400_cfg.cpu_stop_on_nomem = 1;
	em400_cfg.mem_os = 2;
	em400_cfg.emulog_enabled = 0;
	em400_cfg.emulog_file = strdup("em400.log");
	em400_cfg.emulog_format = strdup("%t | %4c | %3l | %m");

	em400_cfg.chans = NULL;

#ifdef WITH_DEBUGGER
	em400_cfg.autotest = 0;
	em400_cfg.pre_expr = NULL;
	em400_cfg.test_expr = NULL;
	em400_cfg.ui_simple = 0;
#endif

	// setup paths
	const char cfg_dir[] = ".em400";
	const char cfg_file[] = "em400.cfg";
	const char cfg_hist_file[] = "history";
	char *home = getenv("HOME");

	em400_cfg.cfg_dir = malloc(strlen(home) + strlen(cfg_dir) + 2);
	em400_cfg.home_cfg_file = malloc(strlen(home) + strlen(cfg_dir) + strlen(cfg_file) + 3);
	em400_cfg.hist_file = malloc(strlen(home) + strlen(cfg_dir) + strlen(cfg_hist_file) + 3);

	if (!em400_cfg.cfg_dir || !em400_cfg.home_cfg_file || !em400_cfg.hist_file) {
		return E_ALLOC;
	}

	sprintf(em400_cfg.cfg_dir, "%s/%s", home, cfg_dir);
	sprintf(em400_cfg.home_cfg_file, "%s/%s/%s", home, cfg_dir, cfg_file);
	sprintf(em400_cfg.hist_file, "%s/%s/%s", home, cfg_dir, cfg_hist_file);

	return E_OK;
}

// -----------------------------------------------------------------------
int cfg_load(char *cfg_file)
{
	eprint("Loading config file: %s\n", cfg_file);
	FILE * cfg = fopen(cfg_file, "r");
	if (!cfg) {
		return E_CFG_OPEN;
	}

	cyyin = cfg;
	do {
		cyyparse();
	} while (!feof(cyyin));

	fclose(cfg);
	cyylex_destroy();

	if (cfg_error) return E_CFG_PARSE;
	return E_OK;
}

// -----------------------------------------------------------------------
void cfg_print()
{
	eprint("---- Config: ---------------------------\n");
	eprint("  Program to load: %s\n", em400_cfg.program_name);
	eprint("  User-provided config: %s\n", em400_cfg.cfg_provided);
	eprint("  Exit on HLT>=040: %s\n", em400_cfg.exit_on_hlt ? "true" : "false");
	eprint("  Emulation speed: %s\n", em400_cfg.speed_real ? "real" : "max");
	eprint("  Timer step: %i (%s at power-on)\n", em400_cfg.timer_step, em400_cfg.timer_start ? "enabled" : "disabled");
	eprint("  CPU modifications: %s\n", em400_cfg.cpu_mod ? "true" : "false");
	eprint("  CPU stop on nomem: %s\n", em400_cfg.cpu_stop_on_nomem ? "true" : "false");
	eprint("  -- Memory: ---------------------------\n");
	eprint("  Elwro modules: %i\n", em400_cfg.mem_elwro);
	eprint("  MEGA modules: %i\n", em400_cfg.mem_mega);
	eprint("  MEGA PROM image: %s\n", em400_cfg.mem_mega_prom);
	eprint("  MEGA boot: %s\n", em400_cfg.mem_mega_boot ? "true" : "false");
	eprint("  Segments for OS: %i\n", em400_cfg.mem_os);
	eprint("  -- I/O: ------------------------------\n");
	struct cfg_chan_t *chanc = em400_cfg.chans;
	while (chanc) {
		eprint("  Channel %2i: %s\n", chanc->num, chanc->name);
		struct cfg_unit_t *unitc = chanc->units;
		while (unitc) {
			eprint("     Unit %2i: %s (args: ", unitc->num, unitc->name);
			struct cfg_arg_t *args = unitc->args;
			while (args) {
				eprint("%s, ", args->text);
				args = args->next;
			}
			eprint(")\n");
			unitc = unitc->next;
		}
		chanc = chanc->next;
	}
	eprint("----------------------------------------\n");
}

// -----------------------------------------------------------------------
struct cfg_arg_t * cfg_make_arg(char *s)
{
	struct cfg_arg_t *a = malloc(sizeof(struct cfg_arg_t));
	a->text = s;
	a->next = NULL;
	return a;
}

// -----------------------------------------------------------------------
int cfg_args_decode(struct cfg_arg_t *arg, const char *format, ...)
{
	int *i;
	char *c;
	char **s;
	char **eptr = NULL;

	if (!format) {
		return E_ARG_FORMAT;
	}

	va_list ap;
	va_start(ap, format);
	while (*format) {
		if (!arg || !arg->text) {
			return E_ARG_NOT_ENOUGH;
		}
		switch (*format) {
			case 'i':
				i = va_arg(ap, int*);
				*i = strtol(arg->text, eptr, 10);
				if (eptr) {
					return E_ARG_CONVERSION;
				}
				break;
			case 'c':
				c = va_arg(ap, char*);
				*c = *(arg->text);
				break;
			case 's':
				s = va_arg(ap, char**);
				*s = strdup(arg->text);
				if (!*s) {
					return E_ALLOC;
				}
				break;
			default:
				return E_ARG_FORMAT;
		}
		format++;
		arg = arg->next;
	}

	va_end(ap);

	return E_OK;
}

// -----------------------------------------------------------------------
void cfg_make_unit(int u_num, char *name, struct cfg_arg_t *arglist)
{
	if (u_num < 0) {
		cyyerror("Incorrect unit number: %i", u_num);
	}

	struct cfg_unit_t *u = malloc(sizeof(struct cfg_unit_t));
	u->name = name;
	u->num = u_num;
	u->args = arglist;
	u->next = em400_cfg.chans->units;
	em400_cfg.chans->units = u;
}

// -----------------------------------------------------------------------
void cfg_make_chan(int c_num, char *name)
{
	if ((c_num < 0) || (c_num > IO_MAX_CHAN)) {
		cyyerror("Incorrect channel number: %i", c_num);
	}

	struct cfg_chan_t *c = malloc(sizeof(struct cfg_chan_t));
	c->num = c_num;
	c->name = name;
	c->next = em400_cfg.chans;
	c->units = NULL;
	em400_cfg.chans = c;
}

// -----------------------------------------------------------------------
void cfg_drop_args(struct cfg_arg_t *a)
{
	if (!a) {
		return;
	}
	cfg_drop_args(a->next);
	free(a->text);
	free(a);
}

// -----------------------------------------------------------------------
void cfg_drop_units(struct cfg_unit_t *u)
{
	if (!u) {
		return;
	}
	cfg_drop_units(u->next);
	cfg_drop_args(u->args);
	free(u->name);
	free(u);
}

// -----------------------------------------------------------------------
void cfg_drop_chans(struct cfg_chan_t *c)
{
	if (!c) {
		return;
	}
	cfg_drop_chans(c->next);
	cfg_drop_units(c->units);
	free(c->name);
	free(c);
}

// vim: tabstop=4 shiftwidth=4 autoindent
