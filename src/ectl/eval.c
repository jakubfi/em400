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

#include "cpu/cp.h"
#include "cpu/cpext.h"
#include "cpu/cpu.h"

#include "libem400.h"
#include "ectl.h"
#include "ectl/eval.h"
#include "eval_parser.h"

struct eval_est *eval_eval_err;

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
struct eval_est * eval_est_val(int16_t v)
{
	struct eval_est *n = eval_est_create();
	n->type = EVAL_AST_N_VAL;
	n->val = v;
	return n;
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_reg(int r)
{
	struct eval_est *n = eval_est_create();
	n->type = EVAL_AST_N_REG;
	n->val = r;
	return n;
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_flag(int f)
{
	struct eval_est *n = eval_est_create();
	n->type = Eval_AST_N_FLAG;
	n->val = f;
	return n;
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_rz(int bit)
{
	struct eval_est *n = eval_est_create();
	n->type = Eval_AST_N_RZ;
	n->val = bit;
	return n;
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_alarm()
{
	struct eval_est *n = eval_est_create();
	n->type = Eval_AST_N_ALARM;
	return n;
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_mc()
{
	struct eval_est *n = eval_est_create();
	n->type = EVAL_AST_N_MC;
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
	if ((n->val < 0) || (n->val > 31)) {
		return __esterr(n, "Wrong interrupt: %i", n->val);
	}

	return (em400_rz32() >> (31 - n->val)) & 1;
}

// -----------------------------------------------------------------------
static int eval_est_eval_alarm(struct eval_est * n)
{
	return em400_cp_alarm_led();
}

// -----------------------------------------------------------------------
static int eval_est_eval_mc(struct eval_est * n)
{
	return em400_cp_mc_led();
}

// -----------------------------------------------------------------------
static int eval_est_eval_mem(struct eval_est *n)
{
	int nb = eval_est_eval(n->n1);
	int addr = eval_est_eval(n->n2);

	if (!n->n1) {
		return __esterr(n, "Missing memory segment");
	}

	if (!n->n2) {
		return __esterr(n, "Missing memory address");
	}

	if ((nb < 0) || (nb > 15)) {
		return __esterr(n->n1, "Wrong memory segment: %i", nb);
	}
	if ((addr < 0) || (addr > 0xffff)) {
		return __esterr(n->n2, "Wrong memory address: %i", addr);
	}

	uint16_t data;
	if (!cpext_mem_read_n(nb, addr, &data, 1)) {
		return __esterr(n, "Memory at %i:%i is not configured", nb, addr);
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
		case '/': return (uint16_t) (v1 / v2);
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
		case Eval_AST_N_FLAG: return eval_est_eval_flag(n);
		case EVAL_AST_N_MEM: return eval_est_eval_mem(n);
		case Eval_AST_N_RZ: return eval_est_eval_rz(n);
		case EVAL_AST_N_OP: return eval_est_eval_op(n);
		case Eval_AST_N_ALARM: return eval_est_eval_alarm(n);
		case EVAL_AST_N_MC: return eval_est_eval_mc(n);
		case EVAL_AST_N_ERR: return -1;
		default: return -1;
	}
}

// -----------------------------------------------------------------------
struct eval_est * eval_est_get_err()
{
	return eval_eval_err;
}

// vim: tabstop=4 shiftwidth=4 autoindent
