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

#ifndef DICT_H
#define DICT_H

#include "nodes.h"

enum _dict_type_e {
	D_NONE,
	D_VARIABLE,
	D_LABEL
};

struct dict_t {
	struct dict_t *parent;
	int level;
	int type;
	char *name;
	struct node_t *n;
};

extern struct dict_t *dict_bottom;
extern struct dict_t *dict_top;
extern struct dict_t *dict_branch;

struct dict_t * dict_add(int level, int type, char *name, struct node_t *n);
struct dict_t * dict_find(char *name);
void dict_drop_level(int level);
void dict_drop(struct dict_t *dict);

#endif

// vim: tabstop=4
