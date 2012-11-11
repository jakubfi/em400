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

struct break_t {
	int nr;
	char *label;
	unsigned int counter;
	int disabled;
	struct node_t *n;
	struct break_t *next;
};

extern int debuger_loop_fin;
extern volatile int debuger_enter;
extern char input_buf[];

extern struct break_t *brk_stack;
extern struct break_t *brk_last;

struct break_t * em400_debuger_brk_check();
void em400_debuger_step();
int em400_debuger_init();
void em400_debuger_shutdown();

#endif

// vim: tabstop=4
