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

#ifndef DEBUGGER_DEBUGGER_H
#define DEBUGGER_DEBUGGER_H

#include <inttypes.h>

#include "registers.h"

#define INPUT_BUF_SIZE 200

struct break_t {
	int nr;
	char *label;
	int counter;
	int disabled;
	struct node_t *n;
	struct break_t *next;
};

enum touch_types {
	TOUCH_NONE = 0,
	TOUCH_R = 1,
	TOUCH_W = 2
};

struct touch_t {
	int type;
	int block;
	int pos;
	struct touch_t *next;
};

extern struct touch_t *touch_mem;
extern struct touch_t *touch_reg;

extern int dbg_loop_fin;
extern volatile int dbg_enter;
extern char input_buf[];

extern struct break_t *brk_stack;
extern struct break_t *brk_last;

extern uint32_t int_act;
extern int int_serve_stack[32];
extern int int_serve_top;

struct break_t * dbg_brk_check();
void dbg_step();
int dbg_init();
void dbg_shutdown();
void dbg_fin_cycle();
void dbg_parse(char *c);
void dbg_touch(struct touch_t **t, int type, int block, int pos);
struct touch_t * dbg_touch_get(struct touch_t **t, int block, int pos);
int dbg_touch_check(struct touch_t **t, int block, int pos);
void dbg_drop_touches(struct touch_t **t);
int dbg_touch2attr(int t);

#endif

// vim: tabstop=4
