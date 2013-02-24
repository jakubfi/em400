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

struct label_t {
	int addr;
	char *name;
	struct label_t *next;
};

struct vval_t {
	int value;
	char *label;
};

struct op_t {
	char *mnemo;
	int type;
	uint16_t opcode;
};

struct norm_t {
	int is_addr;
	int rc, rb;
	struct vval_t *vval;
};

struct word_t {
	int type;
	uint16_t opcode;
	int ra;
	int value;
	char *label;
	struct norm_t *norm;
	struct word_t *next;
};

extern struct word_t *word_first;

struct op_t * get_op(char * opname);
struct vval_t * make_vval(int value, char *label);
struct norm_t * make_norm(int rc, int rb, struct vval_t *vval);
struct word_t * make_op(int type, uint16_t op, int ra, int value, struct norm_t *norm);
void label_add(int addr, char *name);
struct label_t * label_find(char *name);
void word_add(struct word_t *word);
int program_write(struct word_t *word, FILE *out);

// vim: tabstop=4
