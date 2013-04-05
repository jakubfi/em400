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
unsigned int dict_hash(char *i)
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
struct node_t * make_node()
{
	struct node_t *node = malloc(sizeof(struct node_t));
	node->type = N_DUMMY;
	node->opcode = 0;
	node->data = 0;
	node->was_addr = 0;
	node->name = NULL;
	node->n1 = NULL;
	node->n2 = NULL;
	node->next = NULL;
	node->lineno = m_yylloc.first_line;
	node->comment = NULL;
	return node;
}

// -----------------------------------------------------------------------
struct nodelist_t * make_nl(struct node_t *n)
{
	struct nodelist_t *nl = malloc(sizeof(struct nodelist_t));
	nl->head = nl->tail = NULL;
	nl_append_n(nl, n);
	return nl;
}

// -----------------------------------------------------------------------
struct nodelist_t * make_rep(int rep, int value, char *tvalue)
{
	struct nodelist_t *nl = make_nl(NULL);

	while (rep > 0) {
		struct node_t *n = make_value(value, tvalue);
		free(tvalue);
		nl_append_n(nl, n);
		rep--;
	}

	return nl;
}

// -----------------------------------------------------------------------
struct nodelist_t * make_string(char *str)
{
	char *c = str;
	struct nodelist_t *nl = make_nl(NULL);

	while (c && *c) {
		struct node_t *n = make_value((int)(*c << 8), NULL);

		c++;
		if (*c) {
			n->data |= (int)(*c);
			c++;
		}

		nl_append_n(nl, n);
	}

	free(str);

	return nl;
}

// -----------------------------------------------------------------------
struct node_t * make_op(int optype, uint16_t op, int ra, struct node_t *n, struct norm_t *norm)
{
	struct node_t *node = make_node();
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
	}

	return node;
}

// -----------------------------------------------------------------------
struct node_t * make_value(int value, char *tvalue)
{
	struct node_t *n = make_node();
	n->type = N_VAL;
	n->data = value;

	n->lineno = m_yylloc.first_line;
	if (tvalue) {
		n->name = strdup(tvalue);
		free(tvalue);
	}

	return n;
}

// -----------------------------------------------------------------------
struct node_t * make_name(char *name)
{
	struct node_t *n = make_node();
	n->type = N_NAME;
	n->name = strdup(name);
	free(name);

	return n;
}

// -----------------------------------------------------------------------
struct node_t * make_oper(int type, struct node_t *n1, struct node_t *n2)
{
	struct node_t *n = make_node();
	n->type = type;
	n->n1 = n1;
	n->n2 = n2;

	return n;
}

// -----------------------------------------------------------------------
struct node_t * make_comment(char *str)
{
	struct node_t *n = make_node();
	n->type = N_DUMMY;
	n->name = strdup(str);
	free(str);

	return n;
}

// -----------------------------------------------------------------------
struct nodelist_t * nl_append(struct nodelist_t *nl1, struct nodelist_t *nl2)
{
	if (!nl1 && !nl2) return NULL;
	if (!nl1) return nl2;
	if (!nl2) return nl1;

	if (!nl1->tail && !nl2->tail) {
		free(nl2);
		return nl1;
	}

	if (!nl1->tail) {
		free(nl1);
		return nl2;
	}

	if (!nl2->tail) {
		free(nl2);
		return nl1;
	}

	nl1->tail->next = nl2->head;
	nl1->tail = nl2->tail;
	free(nl2);

	return nl1;
}

// -----------------------------------------------------------------------
struct nodelist_t * nl_append_n(struct nodelist_t *nl, struct node_t *n)
{
	if (!nl && !n) {
		return NULL;
	}

	if (!n) {
		return nl;
	}

	if (!nl) {
		nl = make_nl(NULL);
	}

	// n may be a degenerated list, find the tail
	struct node_t *n_tail = n;
	while (n_tail->next) {
		n_tail = n->next;
	}

	if (nl->tail) {
		nl->tail->next = n;
		nl->tail = n_tail;
	} else {
		nl->head = n;
		nl->tail = n_tail;
	}

	return nl;
}

// -----------------------------------------------------------------------
void nodelist_drop(struct nodelist_t *nl)
{
	node_drop(nl->head);
	free(nl);
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
	free(n->comment);
	free(n);
}


// vim: tabstop=4
