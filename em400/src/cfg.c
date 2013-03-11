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

#include "cfg.h"
#include "memory.h"
#include "cfg_parser.h"

extern FILE *cyyin;
void cyyerror(char *s, ...);
int cyyparse();
int cfg_error = 0;

// -----------------------------------------------------------------------
int cfg_load(char *cfg_file)
{
	FILE * cfg = fopen(cfg_file, "r");
	if (!cfg) {
		return -1;
	}

	cyyin = cfg;
	do {
		cyyparse();
	} while (!feof(cyyin));

	fclose(cfg);
	if (cfg_error) return -1;
	return 0;
}

// -----------------------------------------------------------------------
void cfg_print()
{
	printf("---- Config: ---------------------------\n");
	printf("  Program to load: %s\n", em400_cfg.program_name);
	printf("  User-provided config: %s\n", em400_cfg.config_file);
	printf("  Exit on HLT 077: %s\n", em400_cfg.exit_on_hlt ? "true" : "false");
	printf("  Emulation speed: %s\n", em400_cfg.cpu.speed_real ? "real" : "max");
	printf("  Timer step: %i\n", em400_cfg.cpu.timer_step);
	printf("  17-bit byte addressing: %s\n", em400_cfg.cpu.mod_17bit ? "true" : "false");
	printf("  High prio software int: %s\n", em400_cfg.cpu.mod_sint ? "true" : "false");
	printf("---- Memory: ---------------------------\n");
	for (int i=0 ; i<MEM_MAX_MODULES ; i++) {
		printf("  Module %2i: %5s: %2i segments\n", i, em400_cfg.mem[i].is_mega ? "MEGA" : "ELWRO", em400_cfg.mem[i].segments);
	}
	printf("---- I/O: ------------------------------\n");
	for (int i=0 ; i<IO_MAX_CHAN ; i++) {
		printf("  Channel %2i: %s\n", i, em400_cfg.chan[i].type);
		struct cfg_unit_t *units = em400_cfg.chan[i].units;
		while (units) {
			struct cfg_arg_t *args = units->args;
			printf("     Unit %2i: %s (args: ", units->number, args->arg);
			args = args->next;
			while (args) {
				printf("%s, ", args->arg);
				args = args->next;
			}
			printf(")\n");
			units = units->next;
		}
	}
	printf("----------------------------------------\n");
}

// -----------------------------------------------------------------------
int cfg_set_mem(int module, int is_mega, int segments)
{
	if ((module < 0) || (module > MEM_MAX_MODULES-1)) {
		cyyerror("Incorrect module number: %i", module);
		return -1;
	}
	if ((segments < 1) || (segments > MEM_MAX_SEGMENTS)) {
		cyyerror("Incorrect segment count: %i", segments);
		return -2;
	}
	if ((module == 0) && (segments < 2)) {
		cyyerror("Incorrect segment count for OS memory block: %i", segments);
		return -3;
	}
	em400_cfg.mem[module].segments = segments;
	em400_cfg.mem[module].is_mega = is_mega;
	return 0;
}

// -----------------------------------------------------------------------
struct cfg_arg_t * cfg_make_arg(char *s)
{
	struct cfg_arg_t *a = malloc(sizeof(struct cfg_arg_t));
	a->arg = strdup(s);
	a->next = NULL;
	return a;
}

// -----------------------------------------------------------------------
struct cfg_unit_t * cfg_make_unit(int number, struct cfg_arg_t *arglist)
{
	struct cfg_unit_t *u = malloc(sizeof(struct cfg_unit_t));

	u->number = number;
	u->args = arglist;

	return u;
}

// -----------------------------------------------------------------------
int cfg_make_chan(int number, char *type, struct cfg_unit_t *units)
{
	if ((number < 0) || (number > IO_MAX_CHAN)) {
		return -1;
	}
	em400_cfg.chan[number].type = type;
	em400_cfg.chan[number].units = units;
	return 0;
}

// vim: tabstop=4
