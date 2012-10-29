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

void yyerror(char *s);

// -----------------------------------------------------------------------
struct node_t * n_val(int16_t v)
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->type = N_VAL;
	n->val = v;
	n->n1 = NULL;
	n->n2 = NULL;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_var(char *name)
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->type = N_VAR;
	n->var = strdup(name);
	n->n1 = NULL;
	n->n2 = NULL;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_oper(int oper, int nops, struct node_t *n1, struct node_t *n2)
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->type = N_OPER;
	n->val = oper;
	n->n1 = n1;
	n->n2 = n2;
	n->nops = nops;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * n_reg(int r)
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->type = N_REG;
	n->val = r;
	n->n1 = NULL;
	n->n2 = NULL;
	return n;
}

// -----------------------------------------------------------------------
void n_free(struct node_t *n)
{
	if (!n) return;
	n_free(n->n1);
	n_free(n->n2);
	free(n);
}

// -----------------------------------------------------------------------
struct eval_res *n_eval(struct node_t * n)
{
	struct eval_res *e = malloc(sizeof(struct node_t));
	e->val = 0;
	e->res = EV_OK;

	struct eval_res *e1, *e2;

	if (!n) {
		e->res = EV_NODE_NULL;
	} else {
		switch (n->type) {
			case N_VAL:
				e->val = n->val;
				break;
			case N_VAR:
				if (!debuger_get_var(n->var)) {
					e->res = EV_UNKNOWN_VAR;
				} else {
					e->val = debuger_get_var(n->var)->value;
				}
				break;
			case N_REG:
				e->val = R(n->val);
				break;
			case N_OPER:
				e1 = n_eval(n->n1);
				e2 = n_eval(n->n2);
				if ((e1->res != EV_OK) || ((n->nops == 2) && (e2->res != EV_OK))) {
					e->res = e1->res | e2->res;
				} else {
					switch (n->val) {
						case '-':
							e->val = e1->val - e2->val;
							break;
						case '+':
							e->val = e1->val + e2->val;
							break;
						case '*':
							e->val = e1->val * e2->val;
							break;
						case '|':
							e->val = e1->val | e2->val;
							break;
						case '&':
							e->val = e1->val & e2->val;
							break;
						case '^':
							e->val = e1->val ^ e2->val;
							break;
						case SHR:
							e->val = e1->val >> e2->val;
							break;
						case SHL:
							e->val = e1->val << e2->val;
							break;
						case '~':
							e->val = ~e1->val;
							break;
						case '!':
							e->val = !e1->val;
							break;
						case EQ:
							e->val = e1->val == e2->val;
							break;
						case NEQ:
							e->val = e1->val != e2->val;
							break;
						case '>':
							e->val = e1->val > e2->val;
							break;
						case '<':
							e->val = e1->val < e2->val;
							break;
						case GE:
							e->val = e1->val >= e2->val;
							break;
						case LE:
								e->val = e1->val <= e2->val;
							break;
						case '[':
							if (n->n2) {
								e->val = MEMB((e1->val), (e2->val));
							} else {
								e->val = MEM((e1->val));
							}
							break;
						case UMINUS:
							e->val = -e1->val;
							break;
						case AND:
							if (e1->val && e2->val) e->val = 1;
							else e->val = 0;
							break;
						case OR:
							if (e1->val || e2->val) e->val = 1;
							else e->val = 0;
							break;
						default:
							e->res = EV_UNKNOWN_OP;
					}
				}
				free(e1);
				free(e2);
				break;
			default:
				e->res = EV_UNKNOWN_NODE;
		}
	}
	switch (e->res) {
		case EV_OK:
			break;
		case EV_NODE_NULL:
			break;
		case EV_UNKNOWN_VAR:
			waprintw(WCMD, attr[C_ERROR], "Unknown variable\n");
			break;
		case EV_UNKNOWN_NODE:
			waprintw(WCMD, attr[C_ERROR], "Unknown node type\n");
			break;
		case EV_UNKNOWN_OP:
			waprintw(WCMD, attr[C_ERROR], "Unknown operator\n");
			break;
	}
	return e;
}


// vim: tabstop=4
