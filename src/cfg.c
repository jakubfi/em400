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
#include <unistd.h>

#include "mem/mem.h"
#include "io/defs.h"
#include "errors.h"
#include "cfg.h"
#include "cfg_parser.h"

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
	cfg->exit_on_hlt = 0;
	cfg->benchmark = 0;
	cfg->print_help = 0;
	cfg->script_name = NULL;

	// emulation
	cfg->speed_real = 0;

	// cpu
	cfg->timer_step = 10;
	cfg->timer_start = 1;
	cfg->cpu_mod = 0;
	cfg->cpu_user_io_illegal = 1;
	cfg->cpu_awp = 1;
	cfg->cpu_stop_on_nomem = 1;

	// technical console
	cfg->keys = 0;

	// mem
	cfg->mem_elwro = 1;
	cfg->mem_mega = 0;
	cfg->mem_mega_prom = NULL;
	cfg->mem_mega_boot = 0;
	cfg->mem_os = 2;

	// log
	cfg->log_enabled = 0;
	cfg->log_file = strdup("em400.log");
	cfg->log_levels = NULL;
	cfg->log_pname_offset = 0;

	// I/O
	cfg->chans = NULL;

#ifdef WITH_DEBUGGER
	cfg->autotest = 0;
	cfg->pre_expr = NULL;
	cfg->test_expr = NULL;
	cfg->ui_simple = 0;
#endif

	return cfg;
}

// -----------------------------------------------------------------------
void cfg_destroy(struct cfg_em400 *cfg)
{
	if (!cfg) return;

	free(cfg->program_name);
	free(cfg->cfg_filename);
	free(cfg->script_name);

	free(cfg->mem_mega_prom);

	free(cfg->log_file);
	free(cfg->log_levels);

	cfg_drop_chans(cfg->chans);

#ifdef WITH_DEBUGGER
	free(cfg->pre_expr);
	free(cfg->test_expr);
#endif

	free(cfg);
}

// -----------------------------------------------------------------------
struct cfg_em400 * cfg_from_args(int argc, char **argv)
{
	struct cfg_em400 *cfg = cfg_create_default();
	if (!cfg) return NULL;

	int option;

#ifdef WITH_DEBUGGER
	int len;
#endif

	while ((option = getopt(argc, argv,"bhec:p:k:rl:Lt:x:s")) != -1) {
		switch (option) {
			case 'L':
				cfg->log_enabled = 0;
				break;
			case 'l':
				cfg->log_enabled = 1;
				cfg->log_levels = strdup(optarg);
				break;
			case 'b':
				cfg->benchmark = 1;
				break;
			case 'h':
				cfg_destroy(cfg);
				return NULL;
				break;
			case 'c':
				cfg->cfg_filename = strdup(optarg);
				break;
			case 'p':
				cfg->program_name = strdup(optarg);
				break;
			case 'k':
				cfg->keys = atoi(optarg);
				break;
			case 'e':
				cfg->exit_on_hlt = 1;
#ifdef WITH_DEBUGGER
				cfg->autotest = 1;
#endif
				break;
#ifdef WITH_DEBUGGER
			case 't':
				cfg->autotest = 1;
				cfg->ui_simple = 1;
				cfg->exit_on_hlt = 1;
				len = strlen(optarg);
				cfg->test_expr = malloc(len+3);
				strcpy(cfg->test_expr, optarg);
				strcpy(cfg->test_expr + len, "\n\0");
				break;
			case 'r':
				cfg->script_name = strdup(optarg);
				break;
			case 'x':
				len = strlen(optarg);
				cfg->pre_expr = malloc(len+3);
				strcpy(cfg->pre_expr, optarg);
				strcpy(cfg->pre_expr + len, "\n\0");
				break;
			case 's':
				cfg->ui_simple = 1;
				break;
#endif
			default:
				cfg_destroy(cfg);
				return NULL;
		}
	}

	return cfg;
}


// -----------------------------------------------------------------------
struct cfg_em400 * cfg_from_file(char *cfg_file)
{
	FILE * cfgf = fopen(cfg_file, "r");
	if (!cfgf) {
		return NULL;
	}

	struct cfg_em400 *cfg = cfg_create_default();

	cyyin = cfgf;
	do {
		cyyparse(cfg);
	} while (!feof(cyyin));

	fclose(cfgf);
	cyylex_destroy();

	if (cfg_error) {
		cfg_destroy(cfg);
		return NULL;
	}

	return cfg;
}

// -----------------------------------------------------------------------
struct cfg_em400 * cfg_overlay(struct cfg_em400 *a, struct cfg_em400 *b)
{

#define CHI(v) if (b->v != def->v) a->v = b->v
#define CHS(v) if (b->v != def->v) {free(a->v); a->v = b->v; b->v = NULL;}

	struct cfg_em400 *def = cfg_create_default();

	// emulator
	CHS(program_name);
	CHS(cfg_filename);
	CHI(exit_on_hlt);
	CHI(benchmark);
	CHI(print_help);
	CHS(script_name);

	// emulation
	CHI(speed_real);

	// cpu
	CHI(timer_step);
	CHI(timer_start);
	CHI(cpu_mod);
	CHI(cpu_user_io_illegal);
	CHI(cpu_awp);
	CHI(cpu_stop_on_nomem);

	// technical console
	CHI(keys);

	// mem
	CHI(mem_elwro);
	CHI(mem_mega);
	CHS(mem_mega_prom);
	CHI(mem_mega_boot);
	CHI(mem_os);

	// log
	CHI(log_enabled);
	CHS(log_file);
	CHS(log_levels);
	CHI(log_pname_offset);

	// I/O
	if (b->chans != def->chans) {
		cfg_drop_chans(a->chans);
		a->chans = b->chans;
		b->chans = NULL;
	}

#ifdef WITH_DEBUGGER
	CHI(autotest);
	CHS(pre_expr);
	CHS(test_expr);
	CHI(ui_simple);
#endif

	cfg_destroy(def);
	return a;
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
