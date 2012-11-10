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

#ifndef DEBUGER_CMD_H
#define DEBUGER_CMD_H

#include "debuger.h"
#include "debuger_eval.h"

#define MEMDUMP_COLS 16

struct cmd_t {
	char *cmd;
	int tok;
	char *doc;
	char *help;
};

int debuger_is_cmd(char *cmd);
void em400_debuger_c_step();
void em400_debuger_c_quit();
void em400_debuger_c_run();
void em400_debuger_c_help(int wid, char *cmd);
void em400_debuger_c_regs(int wid);
void em400_debuger_c_sregs(int wid);
void em400_debuger_c_reset();
void em400_debuger_c_mem(int wid, int block, int start, int end);
void em400_debuger_c_clmem();
void em400_debuger_c_dt(int wid, int dasm_mode, int start, int count);
void em400_debuger_c_load(int wid, char* image, int bank);
void em400_debuger_c_memcfg(int wid);
void em400_debuger_c_brk_add(char *label, struct node_t *n);
void em400_debuger_c_brk_list();
int em400_debuger_c_brk_del(int nr);
int em400_debuger_c_brk_test(int nr);
int em400_debuger_c_brk_disable(int nr, int disable);

#endif

// vim: tabstop=4
