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

#include "elements.h"
#include "parser_modern.h"
#include "ops.h"

#define DICT_HASH_BITS 8

struct dict_bucket_t dict[1<<DICT_HASH_BITS];

// -----------------------------------------------------------------------
static inline unsigned int dict_hash(char *i)
{
	return (*i & 0b1111) | ((*(i+1) & 0b1111) << 4);
}

// -----------------------------------------------------------------------
struct dict_t * dict_add(int type, char *name, struct enode_t *e)
{
	if (!name || !e) {
		return NULL;
	}

	if (dict_find(name)) {
		return NULL;
	}

	struct dict_t *d = malloc(sizeof(struct dict_t));
	d->type = type;
	d->name = strdup(name);
	d->e = e;
	d->next = NULL;

	struct dict_bucket_t *b = dict + dict_hash(name);

	if (b->head) {
		b->tail->next = d;
		b->tail = d;
	} else {
		b->head = b->tail = d;
	}

	return d;
}

// -----------------------------------------------------------------------
struct dict_t * dict_find(char *name)
{
	if (!name) {
		return NULL;
	}

	struct dict_bucket_t *b = dict + dict_hash(name);
	struct dict_t *d = b->head;

	while (d) {
		if (!strcmp(name, d->name)) {
			return d;
		}
		d = d->next;
	}

	return NULL;
}

// -----------------------------------------------------------------------
void dict_drop(struct dict_t * dict)
{
	if (!dict) {
		return;
	}
	free(dict->name);
	dict_drop(dict->next);
	free(dict);
}

// -----------------------------------------------------------------------
void dicts_drop()
{
	for (int i=0 ; i<1<<DICT_HASH_BITS ; i++) {
		dict_drop(dict[i].head);
	}
}

// -----------------------------------------------------------------------
struct enode_t * make_enode(int type, int value, char *label, struct enode_t *e1, struct enode_t *e2)
{
	struct enode_t *e = malloc(sizeof(struct enode_t));
	e->type = type;
	e->value = value;
	e->was_addr = 0;
	if (label) {
		e->label = strdup(label);
	} else {
		e->label = NULL;
	}
	e->e1 = e1;
	e->e2 = e2;
	return e;
}

// -----------------------------------------------------------------------
void enode_drop(struct enode_t *e)
{
	if (!e) {
		return;
	}

	if (e->label) {
		free(e->label);
	}
	enode_drop(e->e1);
	enode_drop(e->e2);
	free(e);
}

// -----------------------------------------------------------------------
struct norm_t * make_norm(int rc, int rb, struct word_t *word)
{
	struct norm_t *norm = malloc(sizeof(struct norm_t));
	norm->rc = rc;
	norm->rb = rb;
	norm->word = word;
	norm->is_addr = 0;
	return norm;
}

// -----------------------------------------------------------------------
struct word_t * make_data(struct enode_t *e, int lineno)
{
	struct word_t *word = malloc(sizeof(struct word_t));
	word->type = DATA;
	word->e = e;

	word->opcode = 0;
	word->ra = 0;
	word->norm = NULL;
	word->lineno = lineno;
	word->next = NULL;
	return word;
}

// -----------------------------------------------------------------------
struct word_t * make_rep(int rep, int value, int lineno)
{
	struct word_t *whead = NULL;
	struct word_t *wtail = NULL;

	while (rep > 0) {
		struct enode_t *e = make_enode(VALUE, value, NULL, NULL, NULL);
		struct word_t *w = make_data(e, lineno);
		if (!whead) {
			whead = wtail = w;
		} else {
			wtail->next = w;
			wtail= w;
		}
		rep--;
	}

	return whead;
}

// -----------------------------------------------------------------------
struct word_t * make_string(char *str, int lineno)
{
	char *c = str;
	struct word_t *word = NULL;
	struct word_t *word_head = NULL;

	while (c && *c) {
		struct enode_t *e = make_enode(VALUE, (int)(*c << 8), NULL, NULL, NULL);
		struct word_t *w = make_data(e, lineno);

		c++;
		if (*c) {
			e->value |= (int)(*c);
			c++;
		}

		if (!word) {
			word = w;
			word_head = w;
		} else {
			word->next = w;
			word = w;
		}
	}

	return word_head;
}

// -----------------------------------------------------------------------
struct word_t * make_op(int type, uint16_t op, int ra, struct enode_t *e, struct norm_t *norm, int lineno)
{
	struct word_t *word = malloc(sizeof(struct word_t));
	word->type = type;
	word->opcode = op;
	word->ra = ra;
	word->e = e;
	word->norm = norm;
	word->lineno = lineno;
	if (norm && norm->word) {
		word->next = norm->word;
	} else {
		word->next = NULL;
	}
	return word;
}

// vim: tabstop=4
