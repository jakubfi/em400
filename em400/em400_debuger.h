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

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MEMDUMP_COLS 16

typedef struct {
	char *cmd;
	int (*fun)(char*);
	char *doc;
} cmd_s;

extern cmd_s em400_debuger_commands[];

int em400_debuger_c_quit(char* args);
int em400_debuger_c_step(char* args);
int em400_debuger_c_help(char* args);
int em400_debuger_c_regs(char* args);
int em400_debuger_c_reset(char* args);
int em400_debuger_c_mem(char* args);
int em400_debuger_c_memq(char* args);
int em400_debuger_c_memnb(char* args);
int em400_debuger_c_clmem(char* args);
int em400_debuger_c_dasm(char* args);
int em400_debuger_c_trans(char* args);

int em400_debuger_execute(char* line);
void em400_debuger_step();
