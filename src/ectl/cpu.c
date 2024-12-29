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

#include "log.h"
#include "utils/utils.h"
#include "cpu/cp.h"

#include "ectl.h"
#include "ectl/eval.h"
#include "ectl/brk.h"
#include "eval_parser.h"
#include "libem400.h"

typedef struct eval_yy_buffer_state *YY_BUFFER_STATE;
int eval_yyparse(struct eval_est **tree);
YY_BUFFER_STATE eval_yy_scan_string(char *input);
void eval_yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
static struct eval_est * __eval_parse(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	struct eval_est *tree;

	YY_BUFFER_STATE yb = eval_yy_scan_string(expression);
	eval_yyparse(&tree);
	eval_yy_delete_buffer(yb);

	if (!tree) {
		*err_msg = strdup("Fatal error, parser did not return anything");
		return NULL;
	}

	if (tree->type == EVAL_AST_N_ERR) {
		*err_msg = strdup(tree->err);
		*err_beg = tree->c_beg;
		*err_end = tree->c_end;
		eval_est_delete(tree);
		return NULL;
	}

	return tree;
}

// -----------------------------------------------------------------------
int eval_eval(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	LOG(L_ECTL, "ECTL eval: %s", expression);
	int res = -1;
	struct eval_est *tree = __eval_parse(expression, err_msg, err_beg, err_end);
	if (!tree) {
		goto fin;
	}

	res = eval_est_eval(tree);
	if (res < 0) {
		struct eval_est *err_node = eval_est_get_err();
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
	eval_est_delete(tree);
	LOG(L_ECTL, "ECTL eval: %s = %i %s", expression, res, err_msg);
	return res;
}

// -----------------------------------------------------------------------
int ectl_brk_add(char *expression, char **err_msg, int *err_beg, int *err_end)
{
	LOG(L_ECTL, "ECTL brk add: %s", expression);
	struct eval_est *tree = __eval_parse(expression, err_msg, err_beg, err_end);
	if (!tree) {
		return -1;
	}

	int id = ectl_brk_insert(tree, expression);
	if (id < 0) {
		*err_msg = strdup("Cannot add new breakpoint");
		eval_est_delete(tree);
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
