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

#ifndef DEBUGER_H
#define DEBUGER_H

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#define MEMDUMP_COLS 16

enum _debuger_loop_ret {
	DEBUGER_EM400_CONT = 0,
	DEBUGER_EM400_QUIT = 1,
	DEBUGER_LOOP = 2
};

enum _debuger_cmd_res {
	DEBUGER_CMD_OK = 0,
	DEBUGER_CMD_ERR = 1
};

typedef struct {
	char *cmd;
	int tok;
	char *doc;
	char *help;
} cmd_s;

extern int debuger_fin;
extern cmd_s em400_debuger_commands[];

int debuger_is_cmd(char *cmd);

void em400_debuger_c_step();
void em400_debuger_c_quit();
void em400_debuger_c_help(WINDOW *win, int cmd_tok);
void em400_debuger_c_regs(WINDOW *win);
void em400_debuger_c_sregs(WINDOW *win);
void em400_debuger_c_reset();
void em400_debuger_c_mem(WINDOW *win, int block, int start, int end);
void em400_debuger_c_clmem();
void em400_debuger_c_dt(WINDOW *win, int dasm_mode, int start, int count);
void em400_debuger_c_load(WINDOW *win, char* image, int bank);
void em400_debuger_c_memcfg(WINDOW *win);

void em400_debuger_loop();
int em400_debuger_init();
void em400_debuger_shutdown();

#endif

// vim: tabstop=4
