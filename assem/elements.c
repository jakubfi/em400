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

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "eval.h"
#include "elements.h"
#include "parser_modern.h"
#include "ops.h"

// -----------------------------------------------------------------------
static inline unsigned int dict_hash(char *i)
{
	return (*i & 0b1111) | ((*(i+1) & 0b1111) << 4);
}

// -----------------------------------------------------------------------
struct dict_t ** dict_create()
{
	struct dict_t **d = malloc(sizeof(struct dict_t*) * (1<<DICT_HASH_BITS));
	for (int i=0 ; i<(1<<DICT_HASH_BITS) ; i++) {
		d[i] = NULL;
	}
	return d;
}

// -----------------------------------------------------------------------
struct dict_t * dict_add(struct dict_t **dict, int type, char *name, struct node_t *n)
{
	if (!name || !n) {
		return NULL;
	}

	if (dict_find(dict, name)) {
		free(name);
		return NULL;
	}

	struct dict_t *d = malloc(sizeof(struct dict_t));
	d->type = type;
	d->name = strdup(name);
	free(name);
	d->n = n;

	d->next = dict[dict_hash(d->name)];
	dict[dict_hash(d->name)] = d;

	return d;
}

// -----------------------------------------------------------------------
struct dict_t * dict_find(struct dict_t **dict, char *name)
{
	if (!name) {
		return NULL;
	}

	struct dict_t *d = dict[dict_hash(name)];

	while (d) {
		if (!strcmp(name, d->name)) {
			return d;
		}
		d = d->next;
	}

	return NULL;
}

// -----------------------------------------------------------------------
void dict_list_drop(struct dict_t * dict)
{
	if (!dict) {
		return;
	}
	free(dict->name);
	node_drop(dict->n);
	dict_list_drop(dict->next);
	free(dict);
}

// -----------------------------------------------------------------------
void dict_drop(struct dict_t **dict)
{
	if (!dict) {
		return;
	}
	for (int i=0 ; i<(1<<DICT_HASH_BITS) ; i++) {
		dict_list_drop(dict[i]);
	}
	free(dict);
}

// -----------------------------------------------------------------------
struct norm_t * make_norm(int rc, int rb, struct node_t *n)
{
	struct norm_t *norm = malloc(sizeof(struct norm_t));
	norm->rc = rc;
	norm->rb = rb;
	norm->e = n;
	norm->d = 0;
	return norm;
}

// -----------------------------------------------------------------------
struct node_t * make_rep(int rep, int value, char *tvalue)
{
	struct node_t *nhead = NULL;
	struct node_t *ntail = NULL;

	while (rep > 0) {
		struct node_t *n = make_value(value, tvalue);
		free(tvalue);
		if (!nhead) {
			nhead = ntail = n;
		} else {
			ntail->next = n;
			ntail= n;
		}
		rep--;
	}

	return nhead;
}

// -----------------------------------------------------------------------
struct node_t * make_string(char *str)
{
	char *c = str;
	struct node_t *node = NULL;
	struct node_t *node_head = NULL;

	while (c && *c) {
		struct node_t *n = make_value((int)(*c << 8), NULL);

		c++;
		if (*c) {
			n->data |= (int)(*c);
			c++;
		}

		if (!node) {
			node = n;
			node_head = n;
		} else {
			node->next = n;
			node = n;
		}
	}

	free(str);

	return node_head;
}

// -----------------------------------------------------------------------
struct node_t * make_op(int optype, uint16_t op, int ra, struct node_t *n, struct norm_t *norm)
{
	struct node_t *node = malloc(sizeof(struct node_t));
	node->type = optype;
	node->opcode = op;
	node->opcode |= ra << 6;
	node->n1 = n;
	if (norm) {
		node->opcode |= (norm->d << 9);
		node->opcode |= norm->rc;
		node->opcode |= (norm->rb << 3);
		node->next = norm->e;
		free(norm);
	} else {
		node->next = NULL;
	}

	node->lineno = m_yylloc.first_line;
	node->name = NULL;
	node->n2 = NULL;
	return node;
}

// -----------------------------------------------------------------------
struct node_t * make_value(int value, char *tvalue)
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->type = N_VAL;
	n->data = value;

	n->lineno = m_yylloc.first_line;
	n->next = NULL;
	if (tvalue) {
		n->name = strdup(tvalue);
		free(tvalue);
	} else {
		n->name = NULL;
	}
	n->n1 = NULL;
	n->n2 = NULL;

	return n;
}

// -----------------------------------------------------------------------
struct node_t * make_name(char *name)
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->type = N_NAME;
	n->name = strdup(name);
	free(name);

	n->lineno = m_yylloc.first_line;
	n->next = NULL;
	n->n1 = NULL;
	n->n2 = NULL;

	return n;
}

// -----------------------------------------------------------------------
struct node_t * make_oper(int type, struct node_t *n1, struct node_t *n2)
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->type = type;
	n->n1 = n1;
	n->n2 = n2;

	n->lineno = m_yylloc.first_line;
	n->name = NULL;
	n->next = NULL;

	return n;
}

// -----------------------------------------------------------------------
void node_drop(struct node_t *n)
{
	if (!n) {
		return;
	}
	node_drop(n->n1);
	node_drop(n->n2);
	node_drop(n->next);
	free(n->name);
	free(n);
}



// vim: tabstop=4
