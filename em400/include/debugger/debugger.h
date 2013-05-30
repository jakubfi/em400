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

struct evlb_t {
	int nr;
	char *label;
	int value;
	int disabled;
	struct node_t *n;
	struct evlb_t *next;
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
	int oval;
	struct touch_t *next;
};

extern struct touch_t *touch_mem;
extern struct touch_t *touch_reg;
extern struct touch_t *touch_int;

extern char *script_name;

extern int dbg_loop_fin;
extern volatile int dbg_enter;
extern char input_buf[];

extern struct evlb_t *brk_stack;
extern struct evlb_t *brk_top;

extern struct evlb_t *watch_stack;
extern struct evlb_t *watch_top;

extern uint32_t int_act;
extern int int_serve_stack[32];
extern int int_serve_top;

struct evlb_t * dbg_brk_check();
void dbg_step();
int dbg_init();
void dbg_shutdown();
void dbg_fin_cycle();
int dbg_parse(char *c);

void dbg_touch_add(struct touch_t **t, int type, int block, int pos, int oval);
struct touch_t * dbg_touch_get(struct touch_t **t, int block, int pos);
int dbg_touch_check(struct touch_t **t, int block, int pos);
void dbg_touch_pop(struct touch_t **t);
void dbg_touch_drop_all(struct touch_t **t);
int dbg_touch2attr(int t);
int read_script(char *filename);


#endif

// vim: tabstop=4
