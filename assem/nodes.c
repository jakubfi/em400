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
#include <stdarg.h>
#include <inttypes.h>

#include "nodes.h"
#include "parsers.h"

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
struct node_t * make_node(int type)
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->ic = -1;
	n->at = -1;
	n->type = type;
	n->value = 0;
	n->was_addr = 0;
	n->str = NULL;
	n->n1 = NULL;
	n->n2 = NULL;
	n->next = NULL;
	n->lineno = parser_lineno;
	return n;
}

// -----------------------------------------------------------------------
struct node_t * dup_node(struct node_t *sn)
{
	struct node_t *n = malloc(sizeof(struct node_t));
	n->ic = sn->ic;
	n->type = sn->type;
	n->value = sn->value;
	n->was_addr = sn->was_addr;
	n->str = NULL;
	n->n1 = NULL;
	n->n2 = NULL;
	n->next = NULL;
	n->lineno = sn->lineno;
	return n;
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
	if (nl) {
		nodes_drop(nl->head);
		free(nl);
	}
}

// -----------------------------------------------------------------------
void node_drop(struct node_t *n)
{
	if (!n) {
		return;
	}
	node_drop(n->n1);
	node_drop(n->n2);
	free(n->str);
	free(n);
}

// -----------------------------------------------------------------------
void nodes_drop(struct node_t *n)
{
	if (!n) {
		return;
	}
	nodes_drop(n->n1);
	nodes_drop(n->n2);
	nodes_drop(n->next);
	free(n->str);
	free(n);
}


// vim: tabstop=4
