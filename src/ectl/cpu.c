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
#include <inttypes.h>
#include <string.h>
#include <strings.h>
#include <stdatomic.h>

#include "log.h"
#include "utils/utils.h"
#include "cpu/cp.h"
#include "cpu/cpext.h"

#include "ectl.h"
#include "ectl/est.h"
#include "ectl/brk.h"
#include "ectl_parser.h"
#include "libem400.h"

const char *state_names[] = {
	"RUN",
	"STOP",
	"WAIT",
	"CLO",
	"OFF",
	"CYCLE",
	"BIN",
	"LOAD",
	"STORE",
	"FETCH",
	"ANY",
	"???"
};

typedef struct ectl_yy_buffer_state *YY_BUFFER_STATE;
int ectl_yyparse(struct ectl_est **tree);
YY_BUFFER_STATE ectl_yy_scan_string(char *input);
void ectl_yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
int ectl_init()
{
	LOG(L_ECTL, "ECTL init");
	return 0;
}

// -----------------------------------------------------------------------
void ectl_shutdown()
{
	LOG(L_ECTL, "ECTL shutdown");
	ectl_brk_del_all();
}

// -----------------------------------------------------------------------
const char * ectl_cpu_state_name(unsigned state)
{
	if (state > ECTL_STATE_UNKNOWN) {
		return state_names[ECTL_STATE_UNKNOWN];
	} else {
		return state_names[state];
	}
}

// -----------------------------------------------------------------------
unsigned ectl_cpu_state_get()
{
	LOG(L_ECTL, "ECTL state get");
	unsigned state = cpext_state();
	LOG(L_ECTL, "ECTL state get: %s", state_names[state]);
	return state;
}

// -----------------------------------------------------------------------
void ectl_cpu_off()
{
	LOG(L_ECTL, "ECTL cpu OFF");
	cp_off();
}

// -----------------------------------------------------------------------
static struct ectl_est * __ectl_parse(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	struct ectl_est *tree;

	YY_BUFFER_STATE yb = ectl_yy_scan_string(expression);
	ectl_yyparse(&tree);
	ectl_yy_delete_buffer(yb);

	if (!tree) {
		*err_msg = strdup("Fatal error, parser did not return anything");
		return NULL;
	}

	if (tree->type == ECTL_AST_N_ERR) {
		*err_msg = strdup(tree->err);
		*err_beg = tree->c_beg;
		*err_end = tree->c_end;
		ectl_est_delete(tree);
		return NULL;
	}

	return tree;
}

// -----------------------------------------------------------------------
int ectl_eval(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	LOG(L_ECTL, "ECTL eval: %s", expression);
	int res = -1;
	struct ectl_est *tree = __ectl_parse(expression, err_msg, err_beg, err_end);
	if (!tree) {
		goto fin;
	}

	res = ectl_est_eval(tree);
	if (res < 0) {
		struct ectl_est *err_node = ectl_est_get_eval_err();
		if (err_node) {
			*err_msg = strdup(err_node->err);
			*err_beg = err_node->c_beg;
			*err_end = err_node->c_end;
		} else {
			*err_msg = strdup("Evaluation failed");
		}
		goto fin;
	}

fin:
	ectl_est_delete(tree);
	LOG(L_ECTL, "ECTL eval: %s = %i %s", expression, res, err_msg);
	return res;
}

// -----------------------------------------------------------------------
int ectl_brk_add(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	LOG(L_ECTL, "ECTL brk add: %s", expression);
	struct ectl_est *tree = __ectl_parse(expression, err_msg, err_beg, err_end);
	if (!tree) {
		return -1;
	}

	int id = ectl_brk_insert(tree, expression);
	if (id < 0) {
		*err_msg = strdup("Cannot add new breakpoint");
		ectl_est_delete(tree);
		return -1;
	}

	return id;
}

// -----------------------------------------------------------------------
int ectl_brk_del(unsigned id)
{
	LOG(L_ECTL, "ECTL brk del: %i", id);
	return ectl_brk_delete(id);
}


// vim: tabstop=4 shiftwidth=4 autoindent
