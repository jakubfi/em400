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

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "mem/mem.h"
#include "io/defs.h"
#include "cfg.h"
#include "cfg_parser.h"
#include "log.h"

extern FILE *cyyin;
void cyyerror(char *s, ...);
int cyyparse();
int cyylex_destroy();
int cfg_error = 0;

// -----------------------------------------------------------------------
struct cfg_em400 * cfg_create_default()
{
	struct cfg_em400 *cfg = malloc(sizeof(struct cfg_em400));
	if (!cfg) return NULL;

	const char *cfile = "/.em400/em400.cfg";
	char *home = getenv("HOME");
	char *home_cfg_fname = malloc(strlen(home) + strlen(cfile) + 1);
	if (!home_cfg_fname) {
		cfg_destroy(cfg);
		return NULL;
	}
	sprintf(home_cfg_fname, "%s%s", home, cfile);

	// emulator
	cfg->program_name = NULL;
	cfg->cfg_filename = home_cfg_fname;
	cfg->print_help = 0;
	cfg->ui_name = NULL;

	// emulation
	cfg->speed_real = 0;

	// cpu
	cfg->timer_step = 10;
	cfg->timer_start = 1;
	cfg->cpu_mod = 0;
	cfg->cpu_user_io_illegal = 1;
	cfg->cpu_awp = 1;
	cfg->cpu_stop_on_nomem = 1;

	// control panel
	cfg->keys = 0;

	// mem
	cfg->mem_elwro = 1;
	cfg->mem_mega = 0;
	cfg->mem_os = 2;
	cfg->mem_mega_prom = NULL;
	cfg->mem_mega_boot = 0;

	// log
	cfg->log_enabled = 0;
	cfg->log_file = strdup("em400.log");
	cfg->log_levels = strdup("all=0,em4h=9");

	// FPGA
	cfg->fpga = 0;
	cfg->fpga_dev = strdup("/dev/ttyUSB0");
	cfg->fpga_speed = 1000000;

	// I/O
	cfg->chans = NULL;

#ifdef WITH_DEBUGGER
	cfg->ui_simple = 0;
#endif

	return cfg;
}

// -----------------------------------------------------------------------
void cfg_destroy(struct cfg_em400 *cfg)
{
	if (!cfg) return;

	free(cfg->ui_name);
	free(cfg->program_name);
	free(cfg->cfg_filename);
	free(cfg->mem_mega_prom);
	free(cfg->log_file);
	free(cfg->log_levels);
	free(cfg->fpga_dev);
	cfg_drop_chans(cfg->chans);
	free(cfg);
}

// -----------------------------------------------------------------------
int cfg_from_args(struct cfg_em400 *cfg, int argc, char **argv)
{
	int option;
	optind = 1; // reset to 1 so consecutive calls work

	while ((option = getopt(argc, argv, "hc:p:k:l:Lsu:F")) != -1) {
		switch (option) {
			case 'L':
				cfg->log_enabled = 0;
				break;
			case 'l':
				cfg->log_enabled = 1;
				free(cfg->log_levels);
				cfg->log_levels = strdup(optarg);
				break;
			case 'h':
				cfg->print_help = 1;
				break;
			case 'c':
				free(cfg->cfg_filename);
				cfg->cfg_filename = strdup(optarg);
				break;
			case 'p':
				free(cfg->program_name);
				cfg->program_name = strdup(optarg);
				break;
			case 'k':
				cfg->keys = atoi(optarg);
				break;
#ifdef WITH_DEBUGGER
			case 's':
				cfg->ui_simple = 1;
				break;
#endif
			case 'u':
				free(cfg->ui_name);
				cfg->ui_name = strdup(optarg);
				break;
			case 'F':
				cfg->fpga = 1;
				break;
			default:
				return E_ERR;
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int cfg_from_file(struct cfg_em400 *cfg)
{
	FILE *cfgf = fopen(cfg->cfg_filename, "r");
	if (!cfgf) {
		return E_ERR;
	}

	cyyin = cfgf;
	do {
		cyyparse(cfg);
	} while (!feof(cyyin));

	fclose(cfgf);
	cyylex_destroy();

	if (cfg_error) {
		return E_ERR;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
struct cfg_arg * cfg_make_arg(char *s)
{
	struct cfg_arg *a = malloc(sizeof(struct cfg_arg));
	a->text = s;
	a->next = NULL;
	return a;
}

// -----------------------------------------------------------------------
int cfg_args_decode(struct cfg_arg *arg, const char *format, ...)
{
	int *i;
	char *c;
	char **s;
	char *eptr = NULL;

	assert(format);

	va_list ap;
	va_start(ap, format);
	while (*format) {
		assert(arg);
		assert(arg->text);
		switch (*format) {
			case 'i':
				i = va_arg(ap, int*);
				*i = strtol(arg->text, &eptr, 10);
				if (*eptr != '\0') {
					return E_ERR;
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
					return log_err("Memory allocation error while parsing configuration arguments.");
				}
				break;
			default:
				assert(!"Format unknown to cfg_args_decode()");
				return E_ERR;
		}
		format++;
		arg = arg->next;
	}

	va_end(ap);

	return E_OK;
}

// -----------------------------------------------------------------------
void cfg_make_unit(struct cfg_em400 *cfg, int u_num, char *name, struct cfg_arg *arglist)
{
	if (u_num < 0) {
		cyyerror("Incorrect unit number: %i", u_num);
	}

	struct cfg_unit *u = malloc(sizeof(struct cfg_unit));
	u->name = name;
	u->num = u_num;
	u->args = arglist;
	u->next = cfg->chans->units;
	cfg->chans->units = u;
}

// -----------------------------------------------------------------------
void cfg_make_chan(struct cfg_em400 *cfg, int c_num, char *name)
{
	if ((c_num < 0) || (c_num > IO_MAX_CHAN)) {
		cyyerror("Incorrect channel number: %i", c_num);
	}

	struct cfg_chan *c = malloc(sizeof(struct cfg_chan));
	c->num = c_num;
	c->name = name;
	c->next = cfg->chans;
	c->units = NULL;
	cfg->chans = c;
}

// -----------------------------------------------------------------------
void cfg_drop_args(struct cfg_arg *a)
{
	if (!a) {
		return;
	}
	cfg_drop_args(a->next);
	free(a->text);
	free(a);
}

// -----------------------------------------------------------------------
void cfg_drop_units(struct cfg_unit *u)
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
void cfg_drop_chans(struct cfg_chan *c)
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
