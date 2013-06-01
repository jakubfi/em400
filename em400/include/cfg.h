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

#ifndef CFG_H
#define CFG_H

#include "cpu/memory.h"
#include "io/io.h"

extern int cfg_error;

// -----------------------------------------------------------------------
struct cfg_cpu_t {
	int speed_real;
	int timer_step;
	int mod_17bit;
	int mod_sint;
};

// -----------------------------------------------------------------------
struct cfg_mem_module_t {
	int segments;
	int is_mega;
};

// -----------------------------------------------------------------------
struct cfg_arg_t {
	char *text;
	struct cfg_arg_t *next;
};

// -----------------------------------------------------------------------
struct cfg_unit_t {
	char *name;
	struct cfg_arg_t *args;
};

// -----------------------------------------------------------------------
struct cfg_chan_t {
	char *name;
	struct cfg_unit_t units[256];
};

// -----------------------------------------------------------------------
struct cfg_em400_t {

    char *program_name;
    char *config_file;
    int exit_on_hlt;
	int verbose;
	int benchmark;

	struct cfg_cpu_t cpu;
	struct cfg_mem_module_t mem[MEM_MAX_MODULES];
	int mem_os;
	struct cfg_chan_t chans[IO_MAX_CHAN];

#ifdef WITH_DEBUGGER
    int autotest;
    char *pre_expr;
    char *test_expr;
    int ui_simple;
#endif

} em400_cfg;

void eprint(char *format, ...);
int cfg_load(char *cfg_file);
void cfg_print();
void cfg_set_mem(int module, int is_mega, int segments);
void cfg_set_os_mem(int segments);
struct cfg_arg_t * cfg_make_arg(char *arg);
int args_to_cfg(struct cfg_arg_t *arg, const char *format, ...);
void cfg_make_unit(int c_num, int u_num, char *name, struct cfg_arg_t *args);
void cfg_make_chan(int c_num, char *name);

#endif

// vim: tabstop=4
