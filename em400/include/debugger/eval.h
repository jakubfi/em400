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

#ifndef DEBUGGER_EVAL_H
#define DEBUGGER_EVAL_H

#include <inttypes.h>

enum _node_type {
	N_NONE,
	N_OP1,
	N_OP2,
	N_VAL,
	N_VAR,
	N_REG,
	N_MEM,
	N_BF,
	N_ASS,
	N_RZ,
	N_LAST
};

struct node_t {
	int type;
	int base;
	int16_t val;
	char *var;
	int nb;
	void *mptr;
	struct node_t *n1;
	struct node_t *n2;
	struct node_t *next;
};

struct var_t {
	char *name;
	uint16_t value;
	struct var_t *next;
};

// variables
void var_set(char *name, uint16_t value);
struct var_t * var_get(char *name);

// node housekeeping
struct node_t * n_create();
void n_reset_stack();
void n_discard_stack();
void n_free_list(struct node_t *n);
void n_free_tree(struct node_t *n);

// node creation
struct node_t * n_val(int16_t v);
struct node_t * n_var(char *name);
struct node_t * n_reg(int r);
struct node_t * n_ireg(int rtype, int bit);
struct node_t * n_bf(int beg, int end);
struct node_t * n_op1(int oper, struct node_t *n);
struct node_t * n_op2(int oper, struct node_t *n1, struct node_t *n2);
struct node_t * n_ass(struct node_t *n1, struct node_t *n2);
struct node_t * n_mem(struct node_t *n1, struct node_t *n2);

// node evaluation
int16_t n_eval_val(struct node_t *n);
int16_t n_eval_var(struct node_t *n);
int16_t n_eval_reg(struct node_t *n);
int16_t n_eval_ireg(struct node_t * n);
int16_t n_eval_op1(struct node_t *n);
int16_t n_eval_op2(struct node_t *n);
int16_t n_eval_mem(struct node_t *n);
int16_t n_eval(struct node_t *n);

void print_node(struct node_t *n);

#endif

// vim: tabstop=4
