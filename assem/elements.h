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

enum _dict_type {
	D_VALUE,
	D_ADDR
};

struct dict_t {
	char *name;
	int type;
	struct enode_t *e;
	struct dict_t *next;
};

struct dict_bucket_t {
	struct dict_t *head;
	struct dict_t *tail;
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
	struct word_t *word;
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

struct enode_t * make_enode(int type, int value, char *label, struct enode_t *e1, struct enode_t *e2);
void enode_drop(struct enode_t *e);
struct norm_t * make_norm(int rc, int rb, struct word_t *w);
struct word_t * make_data(struct enode_t *e, int lineno);
struct word_t * make_rep(int rep, int value, int lineno);
struct word_t * make_string(char *str, int lineno);
struct word_t * make_op(int type, uint16_t op, int ra, struct enode_t *e, struct norm_t *norm, int lineno);
struct dict_t * dict_add(int type, char *name, struct enode_t *e);
struct dict_t * dict_find(char *name);
void dict_drop(struct dict_t * dict);
void dicts_drop();

#endif

// vim: tabstop=4
