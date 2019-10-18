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

#ifndef DEBUGGER_CMD_H
#define DEBUGGER_CMD_H

#include "ui/curses/debugger.h"
#include "ui/curses/eval.h"

#define MEMDUMP_COLS 16

struct cmd_t {
	const char *cmd;
	int tok;
	const char *doc;
	const char *help;
};

int dbg_is_cmd(char *cmd);
void dbg_c_cycle();
void dbg_c_quit();
void dbg_c_start();
void dbg_c_stop();
void dbg_c_help(int wid, char *cmd);
void dbg_c_regs(int wid);
void dbg_c_sregs(int wid);
void dbg_c_stack(int wid, int size);
void dbg_c_clear();
void dbg_c_mem(int wid, int block, int start, int end, int maxcols, int maxlines);
void dbg_c_bin(int wid, uint16_t addr);
void dbg_c_clmem();
void dbg_c_dt(int wid, uint16_t start, int count);
void dbg_c_load(int wid, char* image);
void dbg_c_memcfg(int wid);
void dbg_c_memmap(int wid, int seg, int page, int module, int frame);
void dbg_c_brk_add(int wid, char *label, struct node_t *n);
void dbg_c_brk_list(int wid);
void dbg_c_brk_del(int wid, int nr);
void dbg_c_brk_test(int wid, int nr);
void dbg_c_brk_disable(int wid, int nr, int disable);
void dbg_c_brk_disable_all(int wid, int disable);
void dbg_c_log_info(int wid);
void dbg_c_log_enable(int wid);
void dbg_c_log_disable(int wid);
void dbg_c_log_set_state(int wid, char *component, int state);
void dbg_c_watch_add(int wid, char *label, struct node_t *n);
void dbg_c_watch_list(int wid, int count);
void dbg_c_watch_del(int wid, int nr);
void dbg_c_list_decoders(int wid);
void dbg_c_decode(int wid, char *name, uint16_t addr, int arg);
void dbg_c_find(int wid, uint16_t block, uint16_t value);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
