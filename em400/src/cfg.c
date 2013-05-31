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

#include "errors.h"
#include "cfg.h"
#include "cpu/memory.h"
#include "cfg_parser.h"
#include "drv/drivers.h"

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
	for (int c_num=0 ; c_num<IO_MAX_CHAN ; c_num++) {
		if (em400_cfg.chans[c_num].name) {
			eprint("  Channel %2i: %s\n", c_num, em400_cfg.chans[c_num].name);
			for (int u_num=0 ; u_num<256 ; u_num++) {
				if (em400_cfg.chans[c_num].units[u_num].name) {
					eprint("     Unit %2i: %s (args: ", u_num, em400_cfg.chans[c_num].units[u_num].name);
					struct cfg_arg_t *args = em400_cfg.chans[c_num].units[u_num].args;
					while (args) {
						eprint("%s, ", args->text);
						args = args->next;
					}
					eprint(")\n");
				}
			}
		}
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
void cfg_make_unit(int c_num, int u_num, char *name, struct cfg_arg_t *arglist)
{
	// check channel number
	if ((c_num < 0) || (c_num > IO_MAX_CHAN)) {
		cyyerror("Incorrect channel number for unit %i: %i", u_num, c_num);
		return;
	}

	// check if unit driver of that name exists
	struct drv_t *driver = drv_get(DRV_UNIT, CHAN_IGNORE, name);
	if (!driver) {
		cyyerror("Unknown unit: %s", name);
		return;
	}

	// count arguments
	int cnt = 0;
	struct cfg_arg_t *arg = arglist;
	while (arg) {
		cnt++;
		arg = arg->next;
	}

	if (cnt != driver->argc) {
		cyyerror("Wrong number of arguments for driver '%s'. Got: %i, required %i", name, cnt, driver->argc);
		return;
	}

	em400_cfg.chans[c_num].units[u_num].name = name;
	em400_cfg.chans[c_num].units[u_num].args = arglist;
}

// -----------------------------------------------------------------------
void cfg_make_chan(int c_num, char *name)
{
	if ((c_num < 0) || (c_num > IO_MAX_CHAN)) {
		cyyerror("Incorrect channel number: %i", c_num);
	}

	if (!drv_get(DRV_CHAN, CHAN_IGNORE, name)) {
		cyyerror("Unknown channel: %s", name);
	}

	em400_cfg.chans[c_num].name = name;
}

// vim: tabstop=4
