//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef DEBUGGER_CMD_H
#define DEBUGGER_CMD_H

#include "debugger.h"
#include "debugger_eval.h"

#define MEMDUMP_COLS 16

struct cmd_t {
	char *cmd;
	int tok;
	char *doc;
	char *help;
};

int debugger_is_cmd(char *cmd);
void em400_debugger_c_step();
void em400_debugger_c_quit();
void em400_debugger_c_run();
void em400_debugger_c_help(int wid, char *cmd);
void em400_debugger_c_regs(int wid);
void em400_debugger_c_sregs(int wid);
void em400_debugger_c_reset();
void em400_debugger_c_mem(int wid, int block, int start, int end, int maxcols, int maxlines);
void em400_debugger_c_clmem();
void em400_debugger_c_dt(int wid, int dasm_mode, int start, int count);
void em400_debugger_c_load(int wid, char* image, int bank);
void em400_debugger_c_memcfg(int wid);
void em400_debugger_c_brk_add(unsigned int wid, char *label, struct node_t *n);
void em400_debugger_c_brk_list(unsigned int wid);
void em400_debugger_c_brk_del(unsigned int wid, int nr);
void em400_debugger_c_brk_test(unsigned int wid, int nr);
void em400_debugger_c_brk_disable(unsigned int wid, int nr, int disable);

#endif

// vim: tabstop=4
