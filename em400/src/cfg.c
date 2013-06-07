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

#include "cpu/memory.h"

#include "errors.h"
#include "cfg.h"
#include "cfg_parser.h"

extern FILE *cyyin;
void cyyerror(char *s, ...);
int cyyparse();
int cyylex_destroy();
int cfg_error = 0;

// -----------------------------------------------------------------------
void eprint(char *format, ...)
{
	if (!em400_cfg.verbose) return;
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
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
	eprint("  User-provided config: %s\n", em400_cfg.config_file);
	eprint("  Exit on HLT 077: %s\n", em400_cfg.exit_on_hlt ? "true" : "false");
	eprint("  Emulation speed: %s\n", em400_cfg.cpu.speed_real ? "real" : "max");
	eprint("  Timer step: %i\n", em400_cfg.cpu.timer_step);
	eprint("  17-bit byte addressing: %s\n", em400_cfg.cpu.mod_17bit ? "true" : "false");
	eprint("  High prio software int: %s\n", em400_cfg.cpu.mod_sint ? "true" : "false");
	eprint("  -- Memory: ---------------------------\n");
	eprint("  Segments for OS: %i\n", em400_cfg.mem_os);
	for (int i=0 ; i<MEM_MAX_MODULES ; i++) {
		eprint("  Module %2i: %5s: %2i segments\n", i, em400_cfg.mem[i].is_mega ? "MEGA" : "ELWRO", em400_cfg.mem[i].segments);
	}
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
void cfg_set_mem(int module, int is_mega, int segments)
{
	if ((module < 0) || (module > MEM_MAX_MODULES-1)) {
		cyyerror("Incorrect module number: %i", module);
	}
	if ((segments < 1) || (segments > MEM_MAX_SEGMENTS)) {
		cyyerror("Incorrect segment count: %i", segments);
	}
	if ((module == 0) && (segments < 2)) {
		cyyerror("Incorrect segment count for OS memory block: %i", segments);
	}
	em400_cfg.mem[module].segments = segments;
	em400_cfg.mem[module].is_mega = is_mega;
}

// -----------------------------------------------------------------------
void cfg_set_os_mem(int segments)
{
	if ((segments < 1) || (segments > 2)) {
		cyyerror("Incorrect segment count reserved for OS usage: %i", segments);
	}

	em400_cfg.mem_os = segments;
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
	int count = 0;

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
		count++;
		format++;
		arg = arg->next;
	}

	va_end(ap);

	if (!*format && arg) {
		return E_ARG_TOO_MANY;
	}

	return count;
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
