//  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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

#include "atomic.h"
#include "ectl/est.h"

struct ectl_brkpoint {
	unsigned id;
	char *expr;
	struct ectl_est *tree;
	struct ectl_brkpoint *next;
};

static struct ectl_brkpoint *ectl_brk_list;
static int ectl_brk_id = 0;

// -----------------------------------------------------------------------
int ectl_brk_insert(struct ectl_est *tree, char *expr)
{
	if (ectl_brk_id < 0) {
		return -1;
	}

	struct ectl_brkpoint *brkp = malloc(sizeof(struct ectl_brkpoint));
	if (!brkp) {
		return -1;
	}

	brkp->id = ectl_brk_id;
	brkp->tree = tree;
	brkp->expr = strdup(expr);
	brkp->next = ectl_brk_list;
	atom_store_release(&ectl_brk_list, brkp);
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
	struct ectl_brkpoint *brkp = ectl_brk_list;
	while (brkp) {
		struct ectl_brkpoint *next = brkp->next;
		ectl_brk_free(brkp);
		brkp = next;
	}

	ectl_brk_list = NULL;
	ectl_brk_id = 0;
}

// -----------------------------------------------------------------------
int ectl_brk_delete(unsigned id)
{
	struct ectl_brkpoint *brkp = ectl_brk_list;
	struct ectl_brkpoint *prev = NULL;

	while (brkp) {
		if (brkp->id == id) {
			if (prev) {
				atom_store_release(&(prev->next), brkp->next);
			} else {
				atom_store_release(&ectl_brk_list, brkp->next);
			}
			if (!ectl_brk_list) {
				ectl_brk_id = 0;
			}
			// TODO: this needs to be freed when CPU is in STOP state
			// or ectl_brk_check() will eventually fail
			ectl_brk_free(brkp);
			return 0;
		}
		prev = brkp;
		brkp = brkp->next;
	}

	return -1;
}

// -----------------------------------------------------------------------
int ectl_brk_check()
{
	struct ectl_brkpoint *brkp = ectl_brk_list;

	while (brkp) {
		if (ectl_est_eval(brkp->tree) > 0) {
			return 1;
		}
		brkp = brkp->next;
	}

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
