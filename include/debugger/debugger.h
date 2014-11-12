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
#include <emdas.h>

#include "cfg.h"

#define INPUT_BUF_SIZE 200

struct evlb_t {
	int nr;
	char *label;
	int value;
	int disabled;
	struct node_t *n;
	struct evlb_t *next;
};

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

extern struct emdas *emd;

struct evlb_t * dbg_brk_check();
void dbg_step();
int dbg_init(struct cfg_em400 *cfg);
void dbg_shutdown();
void dbg_fin_cycle();
int dbg_parse(char *c);

int read_script(char *filename);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
