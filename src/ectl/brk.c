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

#include "ectl.h"
#include "cpu/cp.h"
#include "cpu/cpext.h"
#include "ectl/est.h"

struct ectl_brkpoint {
	unsigned id;
	char *expr;
	struct ectl_est *tree;
	struct ectl_brkpoint *next;
	int deleted;
};

static struct ectl_brkpoint *ectl_brk_list;
static int ectl_brk_id = 0;

// -----------------------------------------------------------------------
int ectl_brk_insert(struct ectl_est *tree, char *expr)
{
	if (ectl_brk_id < 0) {
		return -1;
	}

	struct ectl_brkpoint *brkp = (struct ectl_brkpoint *) malloc(sizeof(struct ectl_brkpoint));
	if (!brkp) {
		return -1;
	}

	brkp->id = ectl_brk_id;
	brkp->tree = tree;
	brkp->expr = strdup(expr);
	brkp->next = atomic_load_explicit(&ectl_brk_list, memory_order_acquire);
	brkp->deleted = 0;
	atomic_store_explicit(&ectl_brk_list, brkp, memory_order_release);
	ectl_brk_id++;

	return brkp->id;
}

// -----------------------------------------------------------------------
static void ectl_brk_free(struct ectl_brkpoint *brkp)
{
	if (!brkp) return;

	ectl_est_delete(brkp->tree);
	free(brkp->expr);
	free(brkp);
}

// -----------------------------------------------------------------------
void ectl_brk_del_all()
{
	struct ectl_brkpoint *brkp = atomic_load_explicit(&ectl_brk_list, memory_order_acquire);
	while (brkp) {
		struct ectl_brkpoint *next = atomic_load_explicit(&brkp->next, memory_order_acquire);
		ectl_brk_free(brkp);
		brkp = next;
	}

	ectl_brk_list = NULL;
	ectl_brk_id = 0;
}

// -----------------------------------------------------------------------
static void ectl_brk_cleanup()
{
	struct ectl_brkpoint *brkp = atomic_load_explicit(&ectl_brk_list, memory_order_acquire);
	struct ectl_brkpoint *prev = NULL;

	while (brkp) {
		if (brkp->deleted) {
			if (prev) {
				atomic_store_explicit(&(prev->next), brkp->next, memory_order_release);
			} else {
				atomic_store_explicit(&ectl_brk_list, brkp->next, memory_order_release);
			}
			if (!ectl_brk_list) {
				ectl_brk_id = 0;
			}
			ectl_brk_free(brkp);
		}
		prev = brkp;
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}
}

// -----------------------------------------------------------------------
int ectl_brk_delete(unsigned id)
{
	int ret = -1;

	struct ectl_brkpoint *brkp = atomic_load_explicit(&ectl_brk_list, memory_order_acquire);

	while (brkp) {
		if (brkp->id == id) {
			brkp->deleted = 1;
			ret = 0;
			break;
		}
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}

	if (cpext_state() == ECTL_STATE_STOP) {
		ectl_brk_cleanup();
	}

	return ret;
}


// -----------------------------------------------------------------------
int ectl_brk_check()
{
	struct ectl_brkpoint *brkp = atomic_load_explicit(&ectl_brk_list, memory_order_acquire);

	while (brkp) {
		if ((!brkp->deleted) && (ectl_est_eval(brkp->tree) > 0)) {
			return 1;
		}
		brkp = atomic_load_explicit(&brkp->next, memory_order_acquire);
	}

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
