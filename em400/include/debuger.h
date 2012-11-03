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

#include <inttypes.h>

#define INPUT_BUF_SIZE 70

extern int debuger_loop_fin;
extern int debuger_enter;
extern char input_buf[];

enum _debuger_loop_ret {
	DEBUGER_EM400_CONT = 0,
	DEBUGER_EM400_QUIT = 1,
	DEBUGER_LOOP = 2
};

enum _debuger_cmd_res {
	DEBUGER_CMD_OK = 0,
	DEBUGER_CMD_ERR = 1
};

struct var_t {
	char *name;
	uint16_t value;
	struct var_t *next;
};

struct break_t {
	int nr;
	char *label;
	struct node_t *n;
	struct break_t *next;
};

extern struct break_t *brkpoints;

void debuger_set_var(char *name, uint16_t value);
struct var_t * debuger_get_var(char *name);

void em400_debuger_brk_check();
void em400_debuger_step();
int em400_debuger_init();
void em400_debuger_shutdown();

#endif

// vim: tabstop=4
