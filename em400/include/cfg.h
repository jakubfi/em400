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
struct cfg_arg_t {
	char *text;
	struct cfg_arg_t *next;
};

// -----------------------------------------------------------------------
struct cfg_unit_t {
	char *name;
	int num;
	struct cfg_arg_t *args;
	struct cfg_unit_t *next;
};

// -----------------------------------------------------------------------
struct cfg_chan_t {
	char *name;
	int num;
	struct cfg_unit_t *units;
	struct cfg_chan_t *next;
};

// -----------------------------------------------------------------------
struct cfg_em400_t {

	char *program_name;
	char *cfg_provided;
	int exit_on_hlt;
	int verbose;
	int benchmark;
	char *cfg_dir;
	char *cfg_file;
	char *hist_file;

	int speed_real;
	int timer_step;
	int mod;
	int mem_elwro;
	int mem_mega;
	int mem_os;

	struct cfg_chan_t *chans;

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
struct cfg_arg_t * cfg_make_arg(char *arg);
int cfg_args_decode(struct cfg_arg_t *arg, const char *format, ...);
void cfg_make_unit(int u_num, char *name, struct cfg_arg_t *args);
void cfg_make_chan(int c_num, char *name);
void cfg_drop_chans(struct cfg_chan_t *c);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
