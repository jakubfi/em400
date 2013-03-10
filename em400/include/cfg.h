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

extern int cfg_error;

struct mem_module_cfg_t {
	int segments;
	int is_mega;
};

struct em400_cfg_t {

    char *program_name;
    char *config_file;
    int exit_on_hlt;

	struct cpu_c {
		int speed_real;
		int timer_step;
		int mod_17bit;
		int mod_sint;
	} cpu;

	struct mem_module_cfg_t mem[16];

#ifdef WITH_DEBUGGER
    int autotest;
    char *pre_expr;
    char *test_expr;
    int ui_simple;
#endif

} em400_cfg;

int cfg_load(char *cfg_file);
int cfg_set_mem(int module, int is_mega, int segments);

#endif

// vim: tabstop=4
