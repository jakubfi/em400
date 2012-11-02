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

#include <string.h>

#include "registers.h"
#include "memory.h"
#include "debuger.h"
#include "debuger_ui.h"
#include "debuger_eval.h"
#include "debuger_parser.h"

struct break_t *brkpoints = NULL;
struct break_t *last_brk = NULL;
int brkcnt = 0;

struct node_t *node_stack = NULL;
struct node_t *last_node = NULL;

// -----------------------------------------------------------------------
void brk_add(char *label, struct node_t *n)
{
	struct break_t *b = malloc(sizeof(struct break_t));
	b->nr= brkcnt;
	b->label = strdup(label);
	b->n = n;
	b->next = NULL;

	if (last_brk) {
		last_brk->next = b;
		last_brk = b;
	} else {
		last_brk = brkpoints = b;
	}

	brkcnt++;
}

// -----------------------------------------------------------------------
void brk_list()
{
	struct break_t *b = brkpoints;
	while (b) {
		waprintw(WCMD, attr[C_DATA], "%-2i : %s\n", b->nr, b->label);
		b = b->next;
	}
}

// -----------------------------------------------------------------------
int brk_del(int nr)
{
	struct break_t *b = brkpoints;
	struct break_t *prev = NULL;
	while (b) {
		if (b->nr == nr) {
			if (prev) {
				prev->next = b->next;
			} else {
				brkpoints = last_brk = b->next;
			}
			free(b->label);
			free(b);
			return 0;
		}
		prev = b;
		b = b->next;
	}
	return 1;
}

// -----------------------------------------------------------------------
// --- NODES -------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct node_t * n_create()
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->type = N_NONE;
	n->val = 0;
	n->var = NULL;
	n->nb = 0;
	n->mptr = NULL;
	n->n1 = NULL;
	n->n2 = NULL;
	n->next = NULL;

	if (!node_stack) {
		node_stack = last_node = n;
	} else {
		last_node->next = n;
		last_node = n;
	}

	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_val(int16_t v)
{
	struct node_t *n = n_create();
	n->type = N_VAL;
	n->val = v;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_reg(int r)
{
	struct node_t *n = n_create();
	n->type = N_REG;
	n->val = r;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_var(char *name)
{
	struct node_t *n = n_create();
	n->type = N_VAR;
	n->mptr = debuger_get_var(name);
	n->var = name;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_op1(int oper, struct node_t *n1)
{
	struct node_t *n = n_create();
	n->type = N_OP1;
	n->val = oper;
	n->n1 = n1;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_op2(int oper, struct node_t *n1, struct node_t *n2)
{
	struct node_t *n = n_create();
	n->type = N_OP2;
	n->val = oper;
	n->n1 = n1;
	n->n2 = n2;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_ass(struct node_t *n1, struct node_t *n2)
{
	struct node_t *n = n_create();
	n->type = N_ASS;
	n->n1 = n1;
	n->n2 = n2;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_bf(int beg, int end)
{
	struct node_t *n = n_create();
	n->type = N_BF;
	uint16_t bf = 0;
	for (int i=beg ; i<=end ; i++) {
		bf |= (0b1000000000000000 >> i);
	}
	n->val = bf;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_mem(struct node_t *n1, struct node_t *n2)
{
	struct node_t *n = n_create();
	n->type = N_MEM;
	// store memory pointer and address for error handling in parser
	n->nb = n_eval(n1);
	n->val = n_eval(n2);
	n->mptr = em400_mem_ptr(n->nb, n->val, 0);
	n->n1 = n1;
	n->n2 = n2;
	return n;
}

// -----------------------------------------------------------------------
void n_reset_stack()
{
	node_stack = last_node = NULL;
}

// -----------------------------------------------------------------------
void n_free_stack(struct node_t *n)
{
	if (!n) return;
	n_free_stack(n->next);
	free(n->var);
	free(n);
	n_reset_stack();
}

// -----------------------------------------------------------------------
void n_free_tree(struct node_t *n)
{
	if (!n) return;
	n_free_tree(n->n1);
	n_free_tree(n->n2);
	free(n->var);
	free(n);
	n_reset_stack();
}

// -----------------------------------------------------------------------
// --- EVALUATION --------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int16_t n_eval_val(struct node_t * n)
{
	return n->val;
}

// -----------------------------------------------------------------------
int16_t n_eval_var(struct node_t * n)
{
	struct debuger_var *v = debuger_get_var(n->var);
	return v->value;
}

// -----------------------------------------------------------------------
int16_t n_eval_reg(struct node_t * n)
{
	return R(n->val);
}

// -----------------------------------------------------------------------
int16_t n_eval_op1(struct node_t * n)
{
	int16_t v;

	v = n_eval(n->n1);

	switch (n->val) {
		case '~':
			return ~v;
		case '!':
			return !v;
		case UMINUS:
			return -v;
		default:
			return 0;
	}
}

// -----------------------------------------------------------------------
int16_t n_eval_ass(struct node_t * n)
{
	int16_t v = n_eval(n->n2);

	unsigned short int nb;
	uint16_t addr;

	switch (n->n1->type) {
		case N_VAR:
			debuger_set_var(n->n1->var, v);
			return v;
		case N_REG:
			Rw(n->n1->val, v);
			return v;
		case N_MEM:
			nb = n_eval(n->n1->n1);
			addr = n_eval(n->n1->n2);
			*em400_mem_ptr(nb, addr, 0) = v;
			return v;
		default:
			return v;
	}
}

// -----------------------------------------------------------------------
int16_t n_eval_mem(struct node_t *n)
{
	unsigned short int nb = n_eval(n->n1);
	uint16_t addr = n_eval(n->n2);
	return *em400_mem_ptr(nb, addr, 0);
}

// -----------------------------------------------------------------------
int16_t n_eval_op2(struct node_t * n)
{
	int16_t v1, v2;

	uint16_t m;
	unsigned short int s;

	v1 = n_eval(n->n1);
	v2 = n_eval(n->n2);

	switch (n->val) {
		case '-':
			return v1 - v2;
		case '+':
			return v1 + v2;
		case '*':
			return v1 * v2;
		case '|':
			return v1 | v2;
		case '&':
			return v1 & v2;
		case '^':
			return v1 ^ v2;
		case SHR:
			return v1 >> v2;
		case SHL:
			return v1 << v2;
		case EQ:
			return v1 == v2;
		case NEQ:
			return v1 != v2;
		case '>':
			return v1 > v2;
		case '<':
			return v1 < v2;
		case GE:
			return v1 >= v2;
		case LE:
			return v1 <= v2;
		case AND:
			if (v1 && v2) return 1;
			else return 0;
		case OR:
			if (v1 || v2) return 1;
			else return 0;
		case '.':
			m = v2;
			s = 0;
			while ((m & 1) == 0) {
				m >>= 1;
				s++;
			}
			return (uint16_t) (v1 & v2) >> s;
		default:
			return 0;
	}
}

// -----------------------------------------------------------------------
int16_t n_eval(struct node_t *n)
{
	switch (n->type) {
		case N_VAL:
			return n_eval_val(n);
		case N_VAR:
			return n_eval_var(n);
		case N_REG:
			return n_eval_reg(n);
		case N_BF:
			return n_eval_val(n);
		case N_OP1:
			return n_eval_op1(n);
		case N_OP2:
			return n_eval_op2(n);
		case N_ASS:
			return n_eval_ass(n);
		case N_MEM:
			return n_eval_mem(n);
		default:
			return 0;
	}
}


// vim: tabstop=4
