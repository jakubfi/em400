//  Copyright (c) 2016-2024 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdatomic.h>
#include <stdbool.h>

#include "cpu/cpu.h"
#include "cp/eval.h"

#include "libem400.h"


struct brk_point {
	unsigned id;
	char *expr;
	struct eval_est *tree;
	_Atomic (struct brk_point *) next;
	_Atomic int deleted;
};

static _Atomic (struct brk_point *) brk_list;
// UI-thread-only (insert/cleanup/del_all all UI) so no atomic needed
static int brk_id = 0;

// -----------------------------------------------------------------------
static int brk_insert(struct eval_est *tree, char *expr)
{
	if (brk_id < 0) {
		return -1;
	}

	struct brk_point *brkp = (struct brk_point *) malloc(sizeof(struct brk_point));
	if (!brkp) {
		return -1;
	}

	brkp->id = brk_id;
	brkp->tree = tree;
	brkp->expr = strdup(expr);
	// node not published yet, so relaxed stores are enough; the release on
	// brk_list below is what publishes a fully-built node to the reader
	atomic_store_explicit(&brkp->next, atomic_load_explicit(&brk_list, memory_order_acquire), memory_order_relaxed);
	atomic_store_explicit(&brkp->deleted, 0, memory_order_relaxed);
	atomic_store_explicit(&brk_list, brkp, memory_order_release);
	brk_id++;

	return brkp->id;
}

// -----------------------------------------------------------------------
static void brk_free(struct brk_point *brkp)
{
	if (!brkp) return;

	eval_est_delete(brkp->tree);
	free(brkp->expr);
	free(brkp);
}

// -----------------------------------------------------------------------
// Precondition: the CPU thread must be stopped (or joined). This frees nodes
// unconditionally and must not race with brk_check() on the CPU thread.
void brk_del_all()
{
	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);
	while (brkp) {
		struct brk_point *next = atomic_load_explicit(&brkp->next, memory_order_acquire);
		brk_free(brkp);
		brkp = next;
	}

	atomic_store_explicit(&brk_list, NULL, memory_order_release);
	brk_id = 0;
}

// -----------------------------------------------------------------------
static void brk_cleanup()
{
	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);
	struct brk_point *prev = NULL;

	while (brkp) {
		// grab next before any free, brkp may be gone afterwards
		struct brk_point *next = atomic_load_explicit(&brkp->next, memory_order_acquire);
		if (atomic_load_explicit(&brkp->deleted, memory_order_relaxed)) {
			if (prev) {
				atomic_store_explicit(&prev->next, next, memory_order_release);
			} else {
				atomic_store_explicit(&brk_list, next, memory_order_release);
			}
			brk_free(brkp);
			// prev unchanged, brkp was unlinked
		} else {
			prev = brkp;
		}
		brkp = next;
	}

	if (!atomic_load_explicit(&brk_list, memory_order_acquire)) {
		brk_id = 0;
	}
}

// -----------------------------------------------------------------------
int brk_delete(unsigned id)
{
	int ret = -1;

	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);

	while (brkp) {
		if (brkp->id == id) {
			atomic_store_explicit(&brkp->deleted, 1, memory_order_relaxed);
			ret = 0;
			break;
		}
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}

	if (cpu_state_get() == EM400_STATE_STOP) {
		brk_cleanup();
	}

	return ret;
}

// -----------------------------------------------------------------------
bool brk_check()
{
	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);

	while (brkp) {
		// skip deleted nodes, they may sit anywhere in the list
		if (!atomic_load_explicit(&brkp->deleted, memory_order_relaxed) && eval_est_eval(brkp->tree) > 0) {
			return true;
		}
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}

	return false;
}

// -----------------------------------------------------------------------
int brk_add(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	struct eval_est *tree = eval_str_parse(expression, err_msg, err_beg, err_end);
	if (!tree) {
		return -1;
	}

	int id = brk_insert(tree, expression);
	if (id < 0) {
		*err_msg = strdup("Cannot add new breakpoint");
		*err_beg = 0;
		*err_end = 0;
		eval_est_delete(tree);
		return -1;
	}

	return id;
}


// vim: tabstop=4 shiftwidth=4 autoindent
