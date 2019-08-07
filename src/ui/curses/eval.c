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

#define _XOPEN_SOURCE 500

#include <string.h>
#include <stdlib.h>

#include "ectl.h"

#include "ui/curses/debugger.h"
#include "ui/curses/keywords.h"
#include "ui/curses/eval.h"
#include "ui/curses/ui.h"
#include "debugger_parser.h"

struct node_t *node_stack = NULL;
struct var_t *var_stack = NULL;

// -----------------------------------------------------------------------
// --- VARIABLES ---------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void var_set(char *name, uint16_t value)
{
	struct var_t *v;

	v = var_get(name);

	if (v) {
		v->value = value;
	} else {
		v = (struct var_t *) malloc(sizeof(struct var_t));
		v->name = strdup(name);
		v->value = value;
		v->next = var_stack;
		var_stack = v;
	}
}

// -----------------------------------------------------------------------
struct var_t * var_get(char *name)
{
	struct var_t *v = var_stack;
	while (v) {
		if (!strcmp(name, v->name)) {
			return v;
		}
		v = v->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------
// --- NODES, HOUSEKEEPING -----------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
struct node_t * n_create()
{
	struct node_t *n = (struct node_t *) malloc(sizeof(struct node_t));
	n->type = N_NONE;
	n->base = HEX;
	n->val = 0;
	n->var = NULL;
	n->nb = 0;
	n->n1 = NULL;
	n->n2 = NULL;
	n->next = node_stack;
	node_stack = n;

	return n;
}

// -----------------------------------------------------------------------
void n_reset_stack()
{
	node_stack = NULL;
}

// -----------------------------------------------------------------------
void n_discard_stack()
{
	n_free_list(node_stack);
}

// -----------------------------------------------------------------------
void n_free_list(struct node_t *n)
{
	if (!n) return;
	n_free_list(n->next);
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
// --- NODES, CREATION ---------------------------------------------------
// -----------------------------------------------------------------------

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
struct node_t * n_ireg(int rtype, int bit)
{
	struct node_t *n = n_create();
	n->type = rtype;
	n->val = bit;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_var(char *name)
{
	struct node_t *n = n_create();
	n->type = N_VAR;
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
	n->n1 = n1;
	n->n2 = n2;
	return n;
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
	struct var_t *v = var_get(n->var);
	return v->value;
}

// -----------------------------------------------------------------------
int16_t n_eval_reg(struct node_t * n)
{
	switch (n->val) {
	case DBG_R_IC:
		return ectl_reg_get(ECTL_REG_IC);
	case DBG_R_KB:
		return ectl_reg_get(ECTL_REG_KB);
	case DBG_R_MOD:
		return ectl_reg_get(ECTL_REG_MOD);
	case DBG_R_MODc:
		return ectl_reg_get(ECTL_REG_MODc);
	case DBG_R_IR:
		return ectl_reg_get(ECTL_REG_IR);
	case DBG_R_SR:
		return ectl_reg_get(ECTL_REG_SR);
	case DBG_R_ALARM:
		return ectl_reg_get(ECTL_REG_ALARM);
	default:
		return ectl_reg_get(n->val);
	}
}

// -----------------------------------------------------------------------
int16_t n_eval_ireg(struct node_t * n)
{
	return (ectl_int_get32() >> (31 - n->val)) & 1;
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

	int nb;
	uint16_t addr;
	int res;

	switch (n->n1->type) {
		case N_VAR:
			var_set(n->n1->var, v);
			return v;
		case N_REG:
			switch (n->n1->val) {
			case DBG_R_IC:
				ectl_reg_set(ECTL_REG_IC, v);
				break;
			case DBG_R_KB:
				ectl_reg_set(ECTL_REG_KB, v);
				break;
			case DBG_R_MOD:
				ectl_reg_set(ECTL_REG_MOD, v);
				break;
			case DBG_R_MODc:
				ectl_reg_set(ECTL_REG_MODc, v);
				break;
			case DBG_R_IR:
				ectl_reg_set(ECTL_REG_IR, v);
				break;
			case DBG_R_SR:
				ectl_reg_set(ECTL_REG_SR, v);
				break;
			case DBG_R_ALARM:
				ectl_reg_set(ECTL_REG_ALARM, v);
				break;
			default:
				ectl_reg_set(n->n1->val, v);
				break;
			}
			return v;
		case N_MEM:
			nb = n_eval(n->n1->n1);
			addr = n_eval(n->n1->n2);
			res = ectl_mem_set(nb, addr, (uint16_t*)&v, 1);
			if (res) {
				return v;
			} else {
				return 0;
			}
		case N_RZ:
			if (v) {
				ectl_int_set(n->n1->val);
			} else {
				ectl_int_clear(n->n1->val);
			}
			return v;
		default:
			return v;
	}
}

// -----------------------------------------------------------------------
int16_t n_eval_mem(struct node_t *n)
{
	int nb = n_eval(n->n1);
	uint16_t addr = n_eval(n->n2);
	int res;
	uint16_t data;
	res = ectl_mem_get(nb, addr, &data, 1);
	if (res) {
		return data;
	} else {
		return 0;
	}
}

// -----------------------------------------------------------------------
int16_t n_eval_op2(struct node_t * n)
{
	int16_t v1, v2;

	uint16_t m;
	int s;

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
			if (v1 == v2) return 1;
			else return 0;
		case NEQ:
			if (v1 != v2) return 1;
			else return 0;
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
		case BF:
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
		case N_RZ:
			return n_eval_ireg(n);
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

// -----------------------------------------------------------------------
void print_node(struct node_t *n)
{
	uint16_t value = n_eval(n);
	switch (n->base) {
		case UINT:
			awtbprint(W_CMD, C_DATA, "%i ", (uint16_t) value);
			break;
		case INT:
			awtbprint(W_CMD, C_DATA, "%i ", (int16_t) value);
			break;
		case HEX:
			awtbprint(W_CMD, C_DATA, "0x%04x ", (uint16_t) value);
			break;
		case OCT:
			awtbprint(W_CMD, C_DATA, "0%0o ", (uint16_t) value);
			break;
		case BIN:
			awtbbinprint(W_CMD, C_DATA, "0b................ ", (uint16_t) value, 16);
			break;
		default:
			awtbprint(W_CMD, C_DATA, "0x%x ", (uint16_t) value);
			break;
	}
}


// vim: tabstop=4 shiftwidth=4 autoindent
