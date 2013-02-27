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

#include<stdio.h>
#include <inttypes.h>

struct op_t {
	char *mnemo;
	int type;
	uint16_t opcode;
};

enum _word_type {
	W_DATA,
	W_OP_2ARG,
	W_OP_FD,
	W_OP_KA1,
	W_OP_JS,
	W_OP_KA2,
	W_OP_C,
	W_OP_SHC,
	W_OP_S,
	W_OP_J,
	W_OP_L,
	W_OP_G,
	W_OP_BN
};

enum _dict_type {
	D_VALUE,
	D_ADDR
};

struct dict_t {
	char *name;
	int type;
	int value;
	struct dict_t *next;
};

struct enode_t {
	int type;
	int value;
	int was_addr;
	char *label;
	struct enode_t *e1, *e2;
};

struct norm_t {
	int is_addr;
	int rc, rb;
	struct enode_t *e;
};

struct word_t {
	int type;
	uint16_t opcode;
	int ra;
	struct enode_t *e;
	struct norm_t *norm;
	struct word_t *next;
	int lineno;
};

extern struct word_t *program_start;
extern struct word_t *program_end;

struct op_t * get_op(char * opname);

char assembly_error[100];

struct enode_t * make_enode(int type, int value, char *label, struct enode_t *e1, struct enode_t *e2);
struct norm_t * make_norm(int rc, int rb, struct enode_t *e);
struct word_t * make_data(struct enode_t *e, int lineno);
struct word_t * make_rep(int rep, struct enode_t *e, int lineno);
struct word_t * make_op(int type, uint16_t op, int ra, struct enode_t *e, struct norm_t *norm, int lineno);
struct dict_t * dict_add(int type, char *name, int value);
struct dict_t * dict_find(char *name);
int program_append(struct word_t *word);
int make_bin(int ic, struct word_t *word, uint16_t *dt);
struct enode_t * enode_eval(struct enode_t *e);


// vim: tabstop=4
