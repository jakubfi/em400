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
#include <stdarg.h>
#include <ctype.h>

#include "cpu/cp.h"

#include "ectl.h"
#include "ectl/est.h"
#include "ectl_parser.h"

struct ectl_est *ectl_eval_err;

// -----------------------------------------------------------------------
// --- NODES, CREATION ---------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static struct ectl_est * ectl_est_create()
{
	struct ectl_est *n = malloc(sizeof(struct ectl_est));
	n->type = ECTL_AST_N_NONE;
	n->val = 0;
	n->nb = 0;
	n->n1 = NULL;
	n->n2 = NULL;
	n->err = NULL;
	n->c_beg = ectl_yylloc.first_column;
	n->c_end = ectl_yylloc.last_column;

	return n;
}

// -----------------------------------------------------------------------
void ectl_est_delete(struct ectl_est *n)
{
	if (!n) return;
	ectl_est_delete(n->n1);
	ectl_est_delete(n->n2);
	free(n->err);
	free(n);
}

// -----------------------------------------------------------------------
struct ectl_est * ectl_est_val(int16_t v)
{
	struct ectl_est *n = ectl_est_create();
	n->type = ECTL_AST_N_VAL;
	n->val = v;
	return n;
}

// -----------------------------------------------------------------------
struct ectl_est * ectl_est_reg(int r)
{
	struct ectl_est *n = ectl_est_create();
	n->type = ECTL_AST_N_REG;
	n->val = r;
	return n;
}

// -----------------------------------------------------------------------
struct ectl_est * ectl_est_flag(int f)
{
	struct ectl_est *n = ectl_est_create();
	n->type = ECTL_AST_N_FLAG;
	n->val = f;
	return n;
}

// -----------------------------------------------------------------------
struct ectl_est * ectl_est_rz(int bit)
{
	struct ectl_est *n = ectl_est_create();
	n->type = ECTL_AST_N_RZ;
	n->val = bit;
	return n;
}

// -----------------------------------------------------------------------
struct ectl_est * ectl_est_op(int oper, struct ectl_est *n1, struct ectl_est *n2)
{
	struct ectl_est *n = ectl_est_create();
	n->type = ECTL_AST_N_OP;
	n->val = oper;
	n->n1 = n1;
	n->n2 = n2;
	return n;
}

// -----------------------------------------------------------------------
struct ectl_est * ectl_est_mem(struct ectl_est *n1, struct ectl_est *n2)
{
	struct ectl_est *n = ectl_est_create();
	n->type = ECTL_AST_N_MEM;
	n->n1 = n1;
	n->n2 = n2;
	return n;
}

// -----------------------------------------------------------------------
struct ectl_est * ectl_est_err(char *err)
{
	struct ectl_est *n = ectl_est_create();
	n->type = ECTL_AST_N_ERR;
	n->err = strdup(err);
	ectl_eval_err = n;
	return n;
}

// -----------------------------------------------------------------------
static int __esterr(struct ectl_est * n, char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	free(n->err);
	n->err = strdup(buf);
	ectl_eval_err = n;

	return -1;
}

// -----------------------------------------------------------------------
// --- EVALUATION --------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
int ectl_est_eval_val(struct ectl_est * n)
{
	return (uint16_t) n->val;
}

// -----------------------------------------------------------------------
int ectl_est_eval_reg(struct ectl_est * n)
{
	int reg = cp_reg_get(n->val);
	if (reg == -1) {
		return __esterr(n, "No such register");
	} else {
		return reg;
	}
}

// -----------------------------------------------------------------------
int ectl_est_eval_flag(struct ectl_est * n)
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
	return (cp_reg_get(0) >> (15-pos)) & 1;
}

// -----------------------------------------------------------------------
int ectl_est_eval_rz(struct ectl_est * n)
{
	if ((n->val < 0) || (n->val > 31)) {
		return __esterr(n, "Wrong interrupt: %i", n->val);
	}

	return (cp_int_get() >> (31 - n->val)) & 1;
}

// -----------------------------------------------------------------------
int ectl_est_eval_mem(struct ectl_est *n)
{
	int nb = ectl_est_eval(n->n1);
	int addr = ectl_est_eval(n->n2);

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
	if (cp_mem_mget(nb, addr, &data, 1) == 0) {
		return __esterr(n, "Memory at %i:%i is not configured", nb, addr);
	}

	return data;
}

// -----------------------------------------------------------------------
static int ectl_est_eval_logical(int op, struct ectl_est * n)
{
	int v1, v2;
	v1 = ectl_est_eval(n->n1);
	if (v1 < 0) {
		return -1;
	}

	switch (op) {
		case AND:
			if (!v1) {
				return 0;
			} else {
				v2 = ectl_est_eval(n->n2);
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
				v2 = ectl_est_eval(n->n2);
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
int ectl_est_eval_op(struct ectl_est * n)
{
	if (!n->n1) {
		return __esterr(n, "Missing first operand");
	}
	if (!n->n2 && (n->val != '~') && (n->val != '!') && (n->val != UMINUS)) {
		return __esterr(n, "Missing second operand");
	}

	if ((n->val == AND) || (n->val == OR)) {
		return ectl_est_eval_logical(n->val, n);
	}

	int v1 = ectl_est_eval(n->n1);
	int v2 = ectl_est_eval(n->n2);

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
int ectl_est_eval(struct ectl_est *n)
{
	if (!n) return -1;

	switch (n->type) {
		case ECTL_AST_N_VAL: return ectl_est_eval_val(n);
		case ECTL_AST_N_REG: return ectl_est_eval_reg(n);
		case ECTL_AST_N_FLAG: return ectl_est_eval_flag(n);
		case ECTL_AST_N_MEM: return ectl_est_eval_mem(n);
		case ECTL_AST_N_RZ: return ectl_est_eval_rz(n);
		case ECTL_AST_N_OP: return ectl_est_eval_op(n);
		case ECTL_AST_N_ERR: return -1;
		default: return -1;
	}
}

// -----------------------------------------------------------------------
struct ectl_est * ectl_est_get_eval_err()
{
	return ectl_eval_err;
}

// vim: tabstop=4 shiftwidth=4 autoindent
