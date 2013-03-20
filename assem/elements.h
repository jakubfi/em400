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

#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <inttypes.h>

enum _dict_type_e {
	D_VALUE,
	D_ADDR
};

struct dict_t {
	char *name;
	int type;
	struct node_t *n;
	struct dict_t *next;
};

enum node_type_e {
	N_OP,
	N_VAL,
	N_NAME,
	N_PLUS,
	N_MINUS,
	N_UMINUS,
	N_SHL,
	N_SHR,
};

struct norm_t {
	int rb, rc, d;
	struct node_t *e;
};

struct node_t {
	int type;
	int optype;
	uint16_t opcode;
	int data;
	int was_addr;
	char *name;
	struct node_t *n1, *n2;
	struct node_t *next;
	int lineno;
};

struct node_t * make_value(int value);
struct node_t * make_name(char *name);
struct node_t * make_oper(int type, struct node_t *n1, struct node_t *n2);
void node_drop(struct node_t *n);

struct norm_t * make_norm(int rc, int rb, struct node_t *n);
struct node_t * make_rep(int rep, int value);
struct node_t * make_string(char *str);
struct node_t * make_op(int optype, uint16_t op, int ra, struct node_t *n, struct norm_t *norm);
struct dict_t ** dict_create();
struct dict_t * dict_add(struct dict_t **dict, int type, char *name, struct node_t *n);
struct dict_t * dict_find(struct dict_t **dict, char *name);
void dict_list_drop(struct dict_t * dict);
void dict_drop(struct dict_t **dict);

#endif

// vim: tabstop=4
