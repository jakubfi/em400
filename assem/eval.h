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

#ifndef EVAL_H
#define EVAL_H

#include <inttypes.h>

#include "elements.h"

struct retry_t {
	struct node_t *n;
	struct retry_t *next;
	int level;
};

extern int enable_debug;
extern char assembly_error[];

void DEBUG(char *format, ...);

struct node_t * eval_val(struct node_t *n);
struct node_t * eval_exlname(struct node_t *n);
struct node_t * eval_name(struct node_t *n);
struct node_t * eval_2op(int operator, struct node_t *n);
struct node_t * eval_1op(int operator, struct node_t *n);
struct node_t * eval_expr(struct node_t *n);

struct node_t * eval_t_arg(struct node_t *n, int ic, int rel_op);

struct node_t * eval_op(struct node_t *n);
struct node_t * eval_op_norm(struct node_t *n);
struct node_t * eval_op_ka1(struct node_t *n);
struct node_t * eval_op_js(struct node_t *n);
struct node_t * eval_op_ka2(struct node_t *n);
struct node_t * eval_op_brc(struct node_t *n);
struct node_t * eval_op_blc(struct node_t *n);
struct node_t * eval_op_shc(struct node_t *n);
struct node_t * eval_op_hlt(struct node_t *n);

struct node_t * eval_multi(struct node_t *n);
struct node_t * eval_string(struct node_t *n);
struct node_t * eval_res(struct node_t *n);

void exlize_names(struct node_t *n);
struct node_t * compose(struct node_t *n);

struct node_t * expr_eval(struct node_t *n);

int assembly(struct nodelist_t *program);
int flow_control(struct node_t **n);
int ass_retry();
int retry_push(struct node_t *n);


#endif

// vim: tabstop=4
