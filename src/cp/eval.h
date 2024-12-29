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

#ifndef __EVAL_H__
#define __EVAL_H__

#include <inttypes.h>

enum eval_est_type {
	EVAL_AST_N_NONE,
	EVAL_AST_N_ERR,
	EVAL_AST_N_OP,
	EVAL_AST_N_VAL,
	EVAL_AST_N_REG,
	EVAL_AST_N_MEM,
	Eval_AST_N_RZ,
	Eval_AST_N_ALARM,
	EVAL_AST_N_MC,
	Eval_AST_N_FLAG,
};

struct eval_est {
	int type;
	int val;
	int nb;
	char *err;
	int c_beg, c_end;
	struct eval_est *n1;
	struct eval_est *n2;
};

// node creation
void eval_est_delete(struct eval_est *n);
struct eval_est * eval_est_val(int16_t v);
struct eval_est * eval_est_reg(int r);
struct eval_est * eval_est_flag(int f);
struct eval_est * eval_est_rz(int bit);
struct eval_est * eval_est_alarm();
struct eval_est * eval_est_mc();
struct eval_est * eval_est_op(int oper, struct eval_est *n1, struct eval_est *n2);
struct eval_est * eval_est_mem(struct eval_est *n1, struct eval_est *n2);
struct eval_est * eval_est_err(char *err);

// node evaluation
int eval_est_eval(struct eval_est *n);
struct eval_est * eval_est_get_err();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
