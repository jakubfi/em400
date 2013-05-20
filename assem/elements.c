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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"
#include "nodes.h"
#include "eval.h"
#include "elements.h"
#include "parser_modern.h"
#include "keywords.h"

struct nodelist_t *program;

// -----------------------------------------------------------------------
struct node_t * mknod_valstr(int type, int value, char *str)
{
	struct node_t *n = make_node(type);
	n->value = value;
	if (str) {
		n->str = strdup(str);
		free(str);
	}
	return n;
}

// -----------------------------------------------------------------------
struct node_t * mknod_nargs(int type, struct node_t *n1, struct node_t *n2)
{
	struct node_t *n = make_node(type);
	n->n1 = n1;
	n->n2 = n2;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * mknod_op(int optype, int op, int ra, struct node_t *arg, struct norm_t *norm)
{
	struct node_t *n = make_node(optype);
	n->value = op;
	n->value |= ra << 6;
	n->n1 = arg;

	// handle normal argument
	if (norm) {
		n->value |= (norm->d << 9);
		n->value |= norm->rc;
		n->value |= (norm->rb << 3);
		n->n1 = norm->e;
		free(norm);
		norm = NULL;
	}
	return n;
}

// -----------------------------------------------------------------------
struct node_t * mknod_dentry(int type, char *name, struct node_t *value)
{
	struct node_t *n = make_node(type);
	n->type = type;
	n->n1 = value;
	n->str = strdup(name);
	free(name);
	return n;
}

// -----------------------------------------------------------------------
struct node_t * mknod_file(char *name, char *type, struct node_t *addr, struct node_t *attr)
{
	struct node_t *n = make_node(N_FILE);
	n->str = malloc(strlen(name) + strlen(type) + 2);
	sprintf(n->str, "%s.%s", name, type);
	free(name);
	free(type);
	n->n1 = addr;
	n->n2 = attr;
	return n;
}

// vim: tabstop=4
