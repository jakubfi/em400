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

#include "dict.h"
#include "nodes.h"

struct dict_t *dict = NULL;
struct symbol_t *symbols = NULL;

// -----------------------------------------------------------------------
struct dict_t * dict_add(int level, int type, char *name, struct node_t *n)
{
	if (!name || !n) {
		return NULL;
	}

	struct dict_t *d = malloc(sizeof(struct dict_t));
	d->name = strdup(name);
	d->level = level;
	d->type = type;
	d->n = n;
	d->parent = dict;
	dict = d;

	return d;
}

// -----------------------------------------------------------------------
struct dict_t * dict_find(char *name)
{
	if (!name) {
		return NULL;
	}

	struct dict_t *d = dict;

	while (d) {
		if (!strcmp(name, d->name)) {
			return d;
		}
		d = d->parent;
	}

	return NULL;
}

// -----------------------------------------------------------------------
void dict_drop_level(int level)
{
	struct dict_t *d = dict;
	struct dict_t *parent = NULL;
	struct dict_t *child = NULL;

	while (d) {
		parent = d->parent;

		if (d->level >= level) {
			dict_drop(d);
			if (child) {
				child->parent = parent;
			} else {
				dict = parent;
			}
		} else {
			child = d;
		}

		d = parent;
	}
}

// -----------------------------------------------------------------------
void dict_drop(struct dict_t * dict)
{
	if (!dict) {
		return;
	}
	free(dict->name);
	nodes_drop(dict->n);
	free(dict);
}

// vim: tabstop=4
