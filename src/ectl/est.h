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

#ifndef ECTL_TREE_H
#define ECTL_TREE_H

#include <inttypes.h>

enum ectl_est_type {
	ECTL_AST_N_NONE,
	ECTL_AST_N_ERR,
	ECTL_AST_N_OP,
	ECTL_AST_N_VAL,
	ECTL_AST_N_REG,
	ECTL_AST_N_MEM,
	ECTL_AST_N_RZ,
	ECTL_AST_N_ALARM,
	ECTL_AST_N_MC,
	ECTL_AST_N_FLAG,
};

struct ectl_est {
	int type;
	int val;
	int nb;
	char *err;
	int c_beg, c_end;
	struct ectl_est *n1;
	struct ectl_est *n2;
};

// node creation
void ectl_est_delete(struct ectl_est *n);
struct ectl_est * ectl_est_val(int16_t v);
struct ectl_est * ectl_est_reg(int r);
struct ectl_est * ectl_est_flag(int f);
struct ectl_est * ectl_est_rz(int bit);
struct ectl_est * ectl_est_alarm();
struct ectl_est * ectl_est_mc();
struct ectl_est * ectl_est_op(int oper, struct ectl_est *n1, struct ectl_est *n2);
struct ectl_est * ectl_est_mem(struct ectl_est *n1, struct ectl_est *n2);
struct ectl_est * ectl_est_err(char *err);

// node evaluation
int ectl_est_eval_val(struct ectl_est *n);
int ectl_est_eval_reg(struct ectl_est *n);
int ectl_est_eval_flag(struct ectl_est *n);
int ectl_est_eval_rz(struct ectl_est * n);
int ectl_est_eval_alarm(struct ectl_est * n);
int ectl_est_eval_mc(struct ectl_est * n);
int ectl_est_eval_op(struct ectl_est *n);
int ectl_est_eval_mem(struct ectl_est *n);
int ectl_est_eval(struct ectl_est *n);
struct ectl_est * ectl_est_get_eval_err();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
