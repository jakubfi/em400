//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <string.h>

#include "cp/eval.h"
#include "cp/watch.h"

struct watch {
	unsigned id;
	char *expr;
	struct watch *next;
};

static struct watch *watch_list;
static struct watch *watch_tail;
static int watch_id;

// -----------------------------------------------------------------------
static struct watch * watch_find(unsigned id)
{
	for (struct watch *w = watch_list; w; w = w->next) {
		if (w->id == id) {
			return w;
		}
	}
	return NULL;
}

// -----------------------------------------------------------------------
static int watch_validate(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	struct eval_est *tree = eval_str_parse(expression, err_msg, err_beg, err_end);
	if (!tree) {
		return -1;
	}
	eval_est_delete(tree);
	return 0;
}

// -----------------------------------------------------------------------
int watch_add(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	if (watch_validate(expression, err_msg, err_beg, err_end) < 0) {
		return -1;
	}

	if (watch_id < 0) {
		*err_msg = strdup("Cannot add new watch");
		*err_beg = 0;
		*err_end = 0;
		return -1;
	}

	struct watch *w = (struct watch *) malloc(sizeof(struct watch));
	if (!w) {
		*err_msg = strdup("Cannot add new watch");
		*err_beg = 0;
		*err_end = 0;
		return -1;
	}

	w->id = watch_id++;
	w->expr = strdup(expression);
	w->next = NULL;

	if (watch_tail) {
		watch_tail->next = w;
	} else {
		watch_list = w;
	}
	watch_tail = w;

	return w->id;
}

// -----------------------------------------------------------------------
int watch_delete(unsigned id)
{
	struct watch *prev = NULL;

	for (struct watch *w = watch_list; w; prev = w, w = w->next) {
		if (w->id != id) {
			continue;
		}
		if (prev) {
			prev->next = w->next;
		} else {
			watch_list = w->next;
		}
		if (w == watch_tail) {
			watch_tail = prev;
		}
		free(w->expr);
		free(w);
		if (!watch_list) {
			watch_id = 0;
		}
		return 0;
	}

	return -1;
}

// -----------------------------------------------------------------------
void watch_del_all()
{
	struct watch *w = watch_list;
	while (w) {
		struct watch *next = w->next;
		free(w->expr);
		free(w);
		w = next;
	}
	watch_list = watch_tail = NULL;
	watch_id = 0;
}

// -----------------------------------------------------------------------
int watch_edit(unsigned id, char *expression, char **err_msg, int *err_beg, int *err_end)
{
	struct watch *w = watch_find(id);
	if (!w) {
		return -1;
	}

	if (watch_validate(expression, err_msg, err_beg, err_end) < 0) {
		return -1;
	}

	char *expr = strdup(expression);
	free(w->expr);
	w->expr = expr;
	return 0;
}

// -----------------------------------------------------------------------
int watch_eval(unsigned id, int *result, char **err_msg, int *err_beg, int *err_end)
{
	struct watch *w = watch_find(id);
	if (!w) {
		return -1;
	}

	*result = eval_str_eval(w->expr, err_msg, err_beg, err_end);
	return 0;
}

// -----------------------------------------------------------------------
void watch_foreach(watch_iter_f cb, void *ctx)
{
	for (struct watch *w = watch_list; w; w = w->next) {
		cb(w->id, w->expr, ctx);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
