//  Copyright (c) 2012-2024 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdarg.h>
#include <ctype.h>

#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "mem/mem.h"

#include "libem400.h"

#include "cp/eval.h"
#include "eval_parser.h"

typedef struct eval_yy_buffer_state *YY_BUFFER_STATE;
int eval_yyparse(struct eval_est **tree);
YY_BUFFER_STATE eval_yy_scan_string(char *input);
void eval_yy_delete_buffer(YY_BUFFER_STATE b);

// Last node that failed during eval_est_eval(), used by eval_str_eval() to
// report the message. Thread-local: the evaluator runs on both the CPU thread
// (brk_check on the live breakpoint tree) and the UI thread (brk/watch/eval on
// fresh-parsed trees), and no tree is shared between threads, so a per-thread
// slot keeps the two from clobbering each other's error.
static thread_local struct eval_est *eval_eval_err;

// -----------------------------------------------------------------------
// --- NODES, CREATION ---------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static struct eval_est * eval_est_create()
{
	struct eval_est *n = (struct eval_est *) malloc(sizeof(struct eval_est));
	n->type = EVAL_AST_N_NONE;
	n->val = 0;
	n->nb = 0;
	n->n1 = NULL;
	n->n2 = NULL;
	n->err = NULL;
	n->c_beg = eval_yylloc.first_column;
	n->c_end = eval_yylloc.last_column;

	return n;
}

// -----------------------------------------------------------------------
void eval_est_delete(struct eval_est *n)
{
	if (!n) return;
	eval_est_delete(n->n1);
	eval_est_delete(n->n2);
	free(n->err);
	free(n);
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_leaf(int type, int16_t val)
{
	struct eval_est *n = eval_est_create();
	n->type = type;
	n->val = val;
	return n;
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_op(int oper, struct eval_est *n1, struct eval_est *n2)
{
	struct eval_est *n = eval_est_create();
	n->type = EVAL_AST_N_OP;
	n->val = oper;
	n->n1 = n1;
	n->n2 = n2;
	return n;
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_mem(struct eval_est *n1, struct eval_est *n2)
{
	struct eval_est *n = eval_est_create();
	n->type = EVAL_AST_N_MEM;
	n->n1 = n1;
	n->n2 = n2;
	return n;
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_err(char *err)
{
	struct eval_est *n = eval_est_create();
	n->type = EVAL_AST_N_ERR;
	n->err = strdup(err);
	eval_eval_err = n;
	return n;
}

// -----------------------------------------------------------------------
static int __esterr(struct eval_est * n, const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	free(n->err);
	n->err = strdup(buf);
	eval_eval_err = n;

	return -1;
}

// -----------------------------------------------------------------------
// --- EVALUATION --------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static int eval_est_eval_val(struct eval_est * n)
{
	return (uint16_t) n->val;
}

// -----------------------------------------------------------------------
static int eval_est_eval_reg(struct eval_est * n)
{
	if ((n->val < 0) || (n->val >= EM400_REG_COUNT)) {
		return __esterr(n, "No such register");
	}
	return cpu_reg_fetch(n->val);
}

// -----------------------------------------------------------------------
static int eval_est_eval_flag(struct eval_est * n)
{
	int pos = -1;

	switch (toupper(n->val)) {
		case 'Z': pos = 0; break;
		case 'M': pos = 1; break;
		case 'V': pos = 2; break;
		case 'C': pos = 3; break;
		case 'L': pos = 4; break;
		case 'E': pos = 5; break;
		case 'G': pos = 6; break;
		case 'Y': pos = 7; break;
		case 'X': pos = 8; break;
		default: return __esterr(n, "No such flag: %c", n->val);
	}
	return (cpu_reg_fetch(0) >> (15-pos)) & 1;
}

// -----------------------------------------------------------------------
static int eval_est_eval_rz(struct eval_est * n)
{
	return int_get_nchan();
}

// -----------------------------------------------------------------------
static int eval_est_eval_rz_bit(struct eval_est * n)
{
	if ((n->val < 0) || (n->val > 31)) {
		return __esterr(n, "Wrong interrupt: %i", n->val);
	}

	return (rz >> (31 - n->val)) & 1;
}

// -----------------------------------------------------------------------
static int eval_est_eval_alarm(struct eval_est * n)
{
	return rALARM;
}

// -----------------------------------------------------------------------
static int eval_est_eval_mc(struct eval_est * n)
{
	return mc;
}

// -----------------------------------------------------------------------
static int eval_est_eval_nb(struct eval_est * n)
{
	return nb;
}

// -----------------------------------------------------------------------
static int eval_est_eval_q(struct eval_est * n)
{
	return q;
}

// -----------------------------------------------------------------------
static int eval_est_eval_bs(struct eval_est * n)
{
	return bs;
}

// -----------------------------------------------------------------------
static int eval_est_eval_rm(struct eval_est * n)
{
	return rm;
}

// -----------------------------------------------------------------------
static int eval_est_eval_mem(struct eval_est *n)
{
	int seg = (int16_t) eval_est_eval(n->n1);
	int addr = eval_est_eval(n->n2);

	if (!n->n1) {
		return __esterr(n, "Missing memory segment");
	}

	if (!n->n2) {
		return __esterr(n, "Missing memory address");
	}

	if (seg > 15) {
		return __esterr(n->n1, "Wrong memory segment: %i", seg);
	}
	if ((addr < 0) || (addr > 0xffff)) {
		return __esterr(n->n2, "Wrong memory address: %i", addr);
	}

	uint16_t data;
	if (seg < 0) {
		if (!mem_read_n(q*nb, addr, &data, 1)) {
			return __esterr(n, "Memory at %i:%i is not configured", q*nb, addr);
		}
	} else {
		if (!mem_read_n(seg, addr, &data, 1)) {
			return __esterr(n, "Memory at %i:%i is not configured", seg, addr);
		}
	}

	return data;
}

// -----------------------------------------------------------------------
static int eval_est_eval_logical(int op, struct eval_est * n)
{
	int v1, v2;
	v1 = eval_est_eval(n->n1);
	if (v1 < 0) {
		return -1;
	}

	switch (op) {
		case AND:
			if (!v1) {
				return 0;
			} else {
				v2 = eval_est_eval(n->n2);
				if (v2 < 0) {
					return -1;
				} else {
					return (v1 && v2);
				}
			}
		case OR:
			if (v1) {
				return 1;
			} else {
				v2 = eval_est_eval(n->n2);
				if (v2 < 0) {
					return -1;
				} else {
					return (v1 || v2);
				}
			}
		default: return __esterr(n, "Unknown operator");
	}
}

// -----------------------------------------------------------------------
static int eval_est_eval_op(struct eval_est * n)
{
	if (!n->n1) {
		return __esterr(n, "Missing first operand");
	}
	if (!n->n2 && (n->val != '~') && (n->val != '!') && (n->val != UMINUS)) {
		return __esterr(n, "Missing second operand");
	}

	if ((n->val == AND) || (n->val == OR)) {
		return eval_est_eval_logical(n->val, n);
	}

	int v1 = eval_est_eval(n->n1);
	int v2 = eval_est_eval(n->n2);

	if ((v1 < 0) || ((v2 < 0) && (n->val != '~') && (n->val != '!') && (n->val != UMINUS))) {
		return -1;
	}

	switch (n->val) {
		case '~': return (uint16_t) ~v1;
		case '!': return (uint16_t) !v1;
		case UMINUS: return (uint16_t) -v1;
		case '-': return (uint16_t) (v1 - v2);
		case '+': return (uint16_t) (v1 + v2);
		case '*': return (uint16_t) (v1 * v2);
		case '/':
			// guard the divide: v1/v2 with v2==0 raises SIGFPE and would take
			// the whole emulator down (this runs on the CPU thread for every
			// breakpoint eval, and on the UI thread for eval/watches)
			if (v2 == 0) {
				return __esterr(n, "Division by zero");
			}
			return (uint16_t) (v1 / v2);
		case '|': return (uint16_t) (v1 | v2);
		case '&': return (uint16_t) (v1 & v2);
		case '^': return (uint16_t) (v1 ^ v2);
		case SHR: return (uint16_t) (v1 >> v2);
		case SHL: return (uint16_t) (v1 << v2);
		case EQ: return (v1 == v2);
		case NEQ: return (v1 != v2);
		case '>': return (v1 > v2);
		case '<': return (v1 < v2);
		case GE: return (v1 >= v2);
		case LE: return (v1 <= v2);
		default: return __esterr(n, "Unknown operator");
	}
}

// -----------------------------------------------------------------------
int eval_est_eval(struct eval_est *n)
{
	if (!n) return -1;

	switch (n->type) {
		case EVAL_AST_N_VAL: return eval_est_eval_val(n);
		case EVAL_AST_N_REG: return eval_est_eval_reg(n);
		case EVAL_AST_N_FLAG: return eval_est_eval_flag(n);
		case EVAL_AST_N_MEM: return eval_est_eval_mem(n);
		case EVAL_AST_N_RZ: return eval_est_eval_rz(n);
		case EVAL_AST_N_RZ_BIT: return eval_est_eval_rz_bit(n);
		case EVAL_AST_N_OP: return eval_est_eval_op(n);
		case EVAL_AST_N_ALARM: return eval_est_eval_alarm(n);
		case EVAL_AST_N_MC: return eval_est_eval_mc(n);
		case EVAL_AST_N_NB: return eval_est_eval_nb(n);
		case EVAL_AST_N_Q: return eval_est_eval_q(n);
		case EVAL_AST_N_BS: return eval_est_eval_bs(n);
		case EVAL_AST_N_RM: return eval_est_eval_rm(n);
		case EVAL_AST_N_ERR: return -1;
		default: return -1;
	}
}

// -----------------------------------------------------------------------
struct eval_est * eval_str_parse(char *str, char **err_msg, int *err_beg, int *err_end)
{
	struct eval_est *tree;

	YY_BUFFER_STATE yb = eval_yy_scan_string(str);
	eval_yyparse(&tree);
	eval_yy_delete_buffer(yb);

	if (!tree) {
		*err_msg = strdup("Fatal error, parser did not return anything");
		*err_beg = 0;
		*err_end = 0;
		return NULL;
	}

	if (tree->type == EVAL_AST_N_ERR) {
		*err_msg = strdup(tree->err);
		*err_beg = tree->c_beg;
		*err_end = tree->c_end;
		eval_est_delete(tree);
		return NULL;
	}

	return tree;
}

// -----------------------------------------------------------------------
int eval_str_eval(char *str, char **err_msg, int *err_beg, int *err_end)
{
	int res = -1;
	struct eval_est *tree = eval_str_parse(str, err_msg, err_beg, err_end);
	if (!tree) {
		goto fin;
	}

	// clear any stale pointer from a previous eval on this thread: that node
	// belongs to an already-freed tree, so it must not be read if this eval
	// returns -1 without setting it
	eval_eval_err = NULL;
	res = eval_est_eval(tree);
	if (res < 0) {
		if (eval_eval_err) {
			*err_msg = strdup(eval_eval_err->err);
			*err_beg = eval_eval_err->c_beg;
			*err_end = eval_eval_err->c_end;
		} else {
			*err_msg = strdup("Evaluation failed");
			*err_beg = 0;
			*err_end = 0;
		}
		goto fin;
	}

fin:
	eval_est_delete(tree);
	return res;
}

// vim: tabstop=4 shiftwidth=4 autoindent
