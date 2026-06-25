//  Copyright (c) 2016-2026 Jakub Filipowicz <jakubf@gmail.com>
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
#include "cp/brk.h"

#include "libem400.h"

typedef _Atomic(struct brk_point *) atomic_brk_ptr;
typedef _Atomic(struct eval_est *) atomic_tree_ptr;

struct brk_point {
	unsigned id;
	char *expr;
	atomic_tree_ptr tree; // swapped under brk_check() on edit, hence atomic
	atomic_brk_ptr next;
	atomic_int deleted;
	atomic_int enabled;
};

// Trees retired by brk_edit() while the CPU runs: brk_check() may still hold the
// old pointer, so the free is deferred to the next stopped-CPU cleanup. UI thread
// only, like brk_id.
struct tree_garbage {
	struct eval_est *tree;
	struct tree_garbage *next;
};

static atomic_brk_ptr brk_list;
static int brk_id = 0; // UI thread only, so no atomic needed
static atomic_int brk_hit = -1;
static struct tree_garbage *brk_garbage; // UI thread only

// -----------------------------------------------------------------------
static int brk_insert(struct eval_est *tree, const char *expr)
{
	if (brk_id < 0) {
		return -1;
	}

	struct brk_point *brkp = (struct brk_point *) malloc(sizeof(struct brk_point));
	if (!brkp) {
		return -1;
	}

	brkp->id = brk_id;
	brkp->expr = strdup(expr);
	// node not published yet, so relaxed stores are enough
	atomic_store_explicit(&brkp->tree, tree, memory_order_relaxed);
	atomic_store_explicit(&brkp->next, atomic_load_explicit(&brk_list, memory_order_acquire), memory_order_relaxed);
	atomic_store_explicit(&brkp->deleted, 0, memory_order_relaxed);
	atomic_store_explicit(&brkp->enabled, 1, memory_order_relaxed);
	// release: publishes a fully-built node to the reader
	atomic_store_explicit(&brk_list, brkp, memory_order_release);
	brk_id++;

	return brkp->id;
}

// -----------------------------------------------------------------------
static void brk_free(struct brk_point *brkp)
{
	if (!brkp) return;

	eval_est_delete(atomic_load_explicit(&brkp->tree, memory_order_relaxed));
	free(brkp->expr);
	free(brkp);
}

// -----------------------------------------------------------------------
// Frees trees retired by brk_edit(). Callers must hold the same precondition as
// brk_free(): the CPU thread is stopped, so no brk_check() can hold a stale tree.
static void brk_garbage_free()
{
	while (brk_garbage) {
		struct tree_garbage *next = brk_garbage->next;
		eval_est_delete(brk_garbage->tree);
		free(brk_garbage);
		brk_garbage = next;
	}
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
	brk_garbage_free();
	brk_id = 0;
	atomic_store_explicit(&brk_hit, -1, memory_order_relaxed);
}

// -----------------------------------------------------------------------
static void brk_cleanup()
{
	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);
	struct brk_point *prev = NULL;

	while (brkp) {
		struct brk_point *next = atomic_load_explicit(&brkp->next, memory_order_acquire);
		if (atomic_load_explicit(&brkp->deleted, memory_order_relaxed)) {
			if (prev) {
				atomic_store_explicit(&prev->next, next, memory_order_release);
			} else {
				atomic_store_explicit(&brk_list, next, memory_order_release);
			}
			brk_free(brkp);
		} else {
			prev = brkp;
		}
		brkp = next;
	}

	brk_garbage_free();

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
int brk_enable(unsigned id, bool enabled)
{
	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);

	while (brkp) {
		if (brkp->id == id) {
			if (atomic_load_explicit(&brkp->deleted, memory_order_relaxed)) {
				return -1;
			}
			atomic_store_explicit(&brkp->enabled, enabled ? 1 : 0, memory_order_relaxed);
			return 0;
		}
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}

	return -1;
}

// -----------------------------------------------------------------------
// Runs on the UI thread. Evaluates a fresh parse of the stored expression
// rather than the live tree that brk_check() walks on the CPU thread, so the
// two threads never mutate the same eval_est node on a runtime eval error.
int brk_eval(unsigned id, int *result, char **err_msg, int *err_beg, int *err_end)
{
	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);

	while (brkp) {
		if (brkp->id == id && !atomic_load_explicit(&brkp->deleted, memory_order_relaxed)) {
			*result = eval_str_eval(brkp->expr, err_msg, err_beg, err_end);
			return 0;
		}
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}

	return -1;
}

// -----------------------------------------------------------------------
void brk_foreach(brk_iter_f cb, void *ctx)
{
	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);

	while (brkp) {
		if (!atomic_load_explicit(&brkp->deleted, memory_order_relaxed)) {
			cb(brkp->id, brkp->expr, atomic_load_explicit(&brkp->enabled, memory_order_relaxed), ctx);
		}
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}
}

// -----------------------------------------------------------------------
bool brk_check()
{
	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);

	while (brkp) {
		// skip deleted and disabled nodes, they may sit anywhere in the list
		if (!atomic_load_explicit(&brkp->deleted, memory_order_relaxed)
			&& atomic_load_explicit(&brkp->enabled, memory_order_relaxed)
			&& eval_est_eval(atomic_load_explicit(&brkp->tree, memory_order_acquire)) > 0) {
			atomic_store_explicit(&brk_hit, (int) brkp->id, memory_order_relaxed);
			return true;
		}
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}

	return false;
}

// -----------------------------------------------------------------------
int brk_hit_get()
{
	return atomic_load_explicit(&brk_hit, memory_order_relaxed);
}

// -----------------------------------------------------------------------
void brk_hit_clear()
{
	atomic_store_explicit(&brk_hit, -1, memory_order_relaxed);
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

// -----------------------------------------------------------------------
// Swaps a breakpoint's expression in place, keeping its id, enabled flag and
// list position. The new tree is parsed first, so a bad expression leaves the
// breakpoint untouched. The old tree may still be walked by brk_check() on a
// running CPU, so its free is deferred to the next stopped-CPU cleanup.
int brk_edit(unsigned id, char *expression, char **err_msg, int *err_beg, int *err_end)
{
	struct eval_est *tree = eval_str_parse(expression, err_msg, err_beg, err_end);
	if (!tree) {
		return -1;
	}

	struct brk_point *brkp = atomic_load_explicit(&brk_list, memory_order_acquire);
	while (brkp) {
		if (brkp->id == id && !atomic_load_explicit(&brkp->deleted, memory_order_relaxed)) {
			break;
		}
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}
	if (!brkp) {
		*err_msg = strdup("No such breakpoint");
		*err_beg = 0;
		*err_end = 0;
		eval_est_delete(tree);
		return -1;
	}

	char *expr = strdup(expression);
	struct eval_est *old_tree = atomic_load_explicit(&brkp->tree, memory_order_relaxed);

	// expr is read only on the UI thread, the tree also on the CPU thread
	free(brkp->expr);
	brkp->expr = expr;
	atomic_store_explicit(&brkp->tree, tree, memory_order_release);

	if (cpu_state_get() == EM400_STATE_STOP) {
		eval_est_delete(old_tree);
	} else {
		struct tree_garbage *g = (struct tree_garbage *) malloc(sizeof(struct tree_garbage));
		if (g) {
			g->tree = old_tree;
			g->next = brk_garbage;
			brk_garbage = g;
		}
	}

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
