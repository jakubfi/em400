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

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <inttypes.h>

#include "registers.h"

#define INPUT_BUF_SIZE 70

struct break_t {
	int nr;
	char *label;
	int counter;
	int disabled;
	struct node_t *n;
	struct break_t *next;
};

extern int dbg_loop_fin;
extern volatile int dbg_enter;
extern char input_buf[];

extern struct break_t *brk_stack;
extern struct break_t *brk_last;

extern int mem_act_block;
extern int mem_actr_min;
extern int mem_actr_max;
extern int mem_actw_min;
extern int mem_actw_max;

extern int reg_act[R_MAX];

struct break_t * dbg_brk_check();
void dbg_step();
int dbg_init();
void dbg_shutdown();
void dbg_fin_cycle();

#endif

// vim: tabstop=4
