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

#include "debugger/debugger.h"
#include "debugger/eval.h"

#define MEMDUMP_COLS 16

struct cmd_t {
	char *cmd;
	int tok;
	char *doc;
	char *help;
};

int dbg_is_cmd(char *cmd);
void dbg_c_step();
void dbg_c_quit();
void dbg_c_run();
void dbg_c_help(int wid, char *cmd);
void dbg_c_regs(int wid);
void dbg_c_sregs(int wid);
void dbg_c_stack(int wid, int size);
void dbg_c_reset();
void dbg_c_mem(int wid, int block, int start, int end, int maxcols, int maxlines);
void dbg_c_clmem();
void dbg_c_dt(int wid, int dasm_mode, int start, int count);
void dbg_c_load(int wid, char* image, int bank);
void dbg_c_memcfg(int wid);
void dbg_c_brk_add(int wid, char *label, struct node_t *n);
void dbg_c_brk_list(int wid);
void dbg_c_brk_del(int wid, int nr);
void dbg_c_brk_test(int wid, int nr);
void dbg_c_brk_disable(int wid, int nr, int disable);

#endif

// vim: tabstop=4
