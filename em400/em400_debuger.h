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

#ifndef EM400_DEBUGER_H
#define EM400_DEBUGER_H

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#define MEMDUMP_COLS 16

#define DEBUGER_EM400_QUIT	-10
#define DEBUGER_LOOP_ERR	-7
#define DEBUGER_EM400_SKIP	-5	// here and below, we don't execute next instruction
#define DEBUGER_EM400_STEP	100

typedef struct {
	char *cmd;
	int (*fun)(WINDOW *win, char*);
	char *doc;
	char *help;
} cmd_s;

extern cmd_s em400_debuger_commands[];

int em400_debuger_c_quit(WINDOW *win, char* args);
int em400_debuger_c_step(WINDOW *win, char* args);
int em400_debuger_c_help(WINDOW *win, char* args);
int em400_debuger_c_regs(WINDOW *win, char* args);
int em400_debuger_c_reset(WINDOW *win, char* args);
int __em400_debuger_dump_mem(WINDOW *win, int block, int start, int end);
int em400_debuger_c_mem(WINDOW *win, char* args);
int em400_debuger_c_memq(WINDOW *win, char* args);
int em400_debuger_c_memnb(WINDOW *win, char* args);
int em400_debuger_c_clmem(WINDOW *win, char* args);
int __em400_debuger_c_dt(WINDOW *win, char* args, int dasm_mode);
int em400_debuger_c_dasm(WINDOW *win, char* args);
void em400_debuger_dt(WINDOW *win, int dasm_mode, int start, int count);
int em400_debuger_c_trans(WINDOW *win, char* args);
int em400_debuger_c_load(WINDOW *win, char* args);
int em400_debuger_c_save(WINDOW *win, char* args);

int em400_debuger_execute(char* line);
int em400_debuger_step();
int em400_debuger_init();
void em400_debuger_shutdown();

#endif

// vim: tabstop=4
