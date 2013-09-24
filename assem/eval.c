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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <inttypes.h>

#include "dict.h"
#include "eval.h"
#include "parser_modern.h"
#include "elements.h"
#include "keywords.h"
#include "image.h"
#include "errors.h"

int enable_debug = 0;

int syntax_level = -1;
struct retry_t *retry;
int retrying = 0;
char text_b = 0;
char text_e = 128;

struct label_t *labels;
struct label_t *labels_top;

char assembly_error[1024];

// -----------------------------------------------------------------------
void DEBUG(char *format, ...)
{
	if (enable_debug) {
		va_list ap;
		va_start(ap, format);
		printf("DEBUG: ");
		for (int i=syntax_level ; i>0 ; i--) printf(".  ");
		if (retrying) {
			printf("+ ");
		}
		vprintf(format, ap);
		va_end(ap);
	}
}

// -----------------------------------------------------------------------
int ass_error(int line, char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int pos = sprintf(assembly_error, "line %i: ", line);
	vsprintf(assembly_error+pos, format, ap);
	va_end(ap);
	return E_ASS;
}

// -----------------------------------------------------------------------
struct label_t * label_add(char *name, int value)
{
	struct label_t *l = malloc(sizeof(struct label_t));
	if (l) {
		l->name = strdup(name);
		l->value = value;
		l->next = NULL;
		if (labels) {
			labels_top->next = l;
			labels_top = l;
		} else {
			labels = labels_top = l;
		}
	}
	return l;
}

// -----------------------------------------------------------------------
int write_labels(FILE *labf)
{
	struct label_t *l = labels;
	while (l) {
		fprintf(labf, "%s = %i\n", l->name, l->value);
		l = l->next;
	}
	return 1;
}

// -----------------------------------------------------------------------
void labels_drop()
{
	struct label_t *l = labels;
	while (l) {
		struct label_t *next = l->next;
		free(l->name);
		free(l);
		l = next;
	}
}

// -----------------------------------------------------------------------
struct node_t * eval_val(struct node_t *n)
{
	struct node_t *nn = dup_node(n);
	if (!nn) {
		ass_error(n->lineno, "Memory allocation error");
		return NULL;
	}

	DEBUG("eval_val(): %i\n", nn->value);

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_exlname(struct node_t *n)
{
	struct node_t *nn;

	struct var_t *ev = get_pvar(extracodes, n->str);
	if (ev) { // this is extracode name
		nn = mknod_valstr(N_VAL, ev->value, NULL);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
		} else {
			DEBUG("eval_exlname() got exl: %s = %i\n", n->str, nn->value);
		}
	} else { // variable or label
		nn = eval_name(n);
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_name(struct node_t *n)
{
	struct node_t *nn;

	// predefined variable - S
	if (!strcasecmp(n->str, "S")) {
		nn = mknod_valstr(N_VAL, img_get_ic(), NULL);
		if (nn) {
			nn->is_addr = 1;
			DEBUG("eval_name() predef: %s = %i\n", n->str, nn->value);
		} else {
			ass_error(n->lineno, "Memory allocation error");
		}
	// predefined variable - #S
	} else if (!strcasecmp(n->str, "#S")) {
		nn = mknod_valstr(N_VAL, img_get_sector(), NULL);
		if (nn) {
			DEBUG("eval_name() predef: %s = %i\n", n->str, nn->value);
		} else {
			ass_error(n->lineno, "Memory allocation error");
		}
	} else { // regular variable or label
		struct dict_t *d = dict_find(n->str);
		if (d) { // exists in dictionary
			nn = eval_expr(d->n);
			if (nn) {
				DEBUG("eval_name() dict: %s = %i\n", n->str, nn->value);
				if (d->type == D_LABEL) {
					nn->is_addr = 1;
				}
			}
		} else { // does not exist (yet!)
			nn = mknod_valstr(N_NAME, 0, strdup(n->str));
			if (nn) {
				ass_error(n->lineno, "Name '%s' unknown", n->str);
				DEBUG("eval_name() undefined: %s\n", n->str);
			} else {
				ass_error(n->lineno, "Memory allocation error");
			}
		}
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_1op(int operator, struct node_t *n)
{
	struct node_t *nn;
	struct node_t *n1 = eval_expr(n->n1);

	if (!n1) {
		return NULL;
	}

	if (n1->type == N_VAL) {
		nn = mknod_valstr(N_VAL, 0, NULL);
		if (nn) {
			nn->is_addr = n1->is_addr;
			switch (operator) {
				case N_UMINUS:
					nn->value = - n1->value;
					nodes_drop(n1);
					break;
				case N_PAR:
					nn->value = n1->value;
					nodes_drop(n1);
					break;
				default:
					ass_error(n->lineno, "Fatal, unknown operator of type %i", n1->type);
					free(nn);
					nn = NULL;
					break;
			}
			DEBUG("eval_1op() result: %i\n", nn->value);
		} else {
			ass_error(n->lineno, "Memory allocation error");
		}
	} else {
		DEBUG("eval_1op() not yet evaluated\n");
		nn = mknod_nargs(operator, n1, NULL);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
		}
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_2op(int operator, struct node_t *n)
{
	struct node_t *nn = NULL;
	struct node_t *n1 = eval_expr(n->n1);
	struct node_t *n2 = eval_expr(n->n2);

	if (!n1 || !n2) {
		return NULL;
	}

	if ((n1->type == N_VAL) && (n2->type == N_VAL)) {
		nn = mknod_valstr(N_VAL, 0, NULL);
		if (nn) {
			nn->is_addr = n1->is_addr | n2->is_addr;
			switch (operator) {
				case N_PLUS:
					nn->value = n1->value + n2->value;
					break;
				case N_MINUS:
					nn->value = n1->value - n2->value;
					break;
				case N_MUL:
					nn->value = n1->value * n2->value;
					break;
				case N_DIV:
					nn->value = n1->value / n2->value;
					break;
				case N_SHR:
					nn->value = n1->value >> n2->value;
					break;
				case N_SHL:
					nn->value = n1->value << n2->value;
					break;
				case N_SCALE:
					nn->value = n1->value << (15 - n2->value);
					break;
				default:
					free(nn);
					ass_error(n->lineno, "Unknown operator (type %i)", n1->type);
					nn = NULL;
					break;
			}
			nodes_drop(n1);
			nodes_drop(n2);
			DEBUG("eval_2op() evaluated to: %i\n", nn->value);
		} else {
			ass_error(n->lineno, "Memory allocation error");
		}
	} else {
		DEBUG("eval_2op() not yet evaluated\n");
		nn = mknod_nargs(operator, n1, n2);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
		}
	}
	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_res(struct node_t *n)
{
	int repetitions;
	struct node_t *n1 = eval_expr(n->n1);
	if (!n1) {
		return NULL;
	}

	if (n1->type == N_VAL) {
		if (n1->value < 0) {
			ass_error(n->n1->lineno, "Repetition count is negative");
			return NULL;
		} else {
			repetitions = n1->value;
		}
	} else {
		return NULL;
	}
	nodes_drop(n1);

	int value;
	struct node_t *n2 = eval_expr(n->n2);
	if (n2 && (n2->type == N_VAL)) {
		value = n2->value;
	} else {
		value = 0;
	}
	nodes_drop(n2);

	DEBUG("eval_res(): rep:%i, val:%i\n", repetitions, value);

	struct node_t *nn = NULL;

	// prepend nodes, order doesn't matter here
	while (repetitions > 0) {
		struct node_t *nnn = mknod_valstr(N_VAL, value, NULL);
		if (!nnn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
		nnn->next = nn;
		nn = nnn;
		repetitions--;
		DEBUG("eval_res(): made node, %i left\n", repetitions);
	}
	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_string(struct node_t *n)
{
	struct node_t *nn = NULL;
	struct node_t *ncur = NULL;

	DEBUG("eval_str(): %s\n", n->str);

	// honor TEXT* string additions
	char *string = malloc(strlen(n->str)+5);
	if (text_b) {
		sprintf(string, "%c%s%c", text_b, n->str, text_e);
	} else {
		sprintf(string, "%s%c", n->str, text_e);
	}

	int pos = 1; // start with left byte
	char *s = string;
	struct node_t *nnn = NULL;

	// append each double-character as value
	while (s && *s) {
		if (pos == 1) { // left byte
			nnn = mknod_valstr(N_VAL, *s << 8, NULL);
			if (!nnn) {
				ass_error(n->lineno, "Memory allocation error");
				free(s);
				return NULL;
			}
			if (ncur) {
				ncur->next = nnn;
				ncur = nnn;
			} else {
				ncur = nnn;
				nn = ncur;
			}
			pos = 0;
		} else { // right byte
			nnn->value |= *s;
			pos = 1;
		}
		s++;
	}

	free(string);
	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_multi(struct node_t *n)
{
	struct node_t *nn = NULL;

	switch(n->type) {
		case N_RES:
			nn = eval_res(n);
			break;
		case N_STRING:
			nn = eval_string(n);
			break;
		default:
			ass_error(n->lineno, "Unknown multi node (type %i)", n->type);
			return NULL;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_expr(struct node_t *n)
{
	if (!n) {
		ass_error(-1, "Evaluating NULL expression");
		return NULL;
	}

	struct node_t *nn = NULL;

	switch (n->type) {
		case N_VAL:
			nn = eval_val(n);
			break;
		case N_EXLNAME:
			nn = eval_exlname(n);
			break;
		case N_NAME:
			nn = eval_name(n);
			break;
		case N_PLUS:
		case N_MINUS:
		case N_MUL:
		case N_DIV:
		case N_SHR:
		case N_SHL:
		case N_SCALE:
			nn = eval_2op(n->type, n);
			break;
		case N_UMINUS:
		case N_PAR:
			nn = eval_1op(n->type, n);
			break;
		default:
			ass_error(n->lineno, "Unknown expression node (type %i)", n->type);
			break;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_op_norm(struct node_t *n)
{
	// opcode
	struct node_t *nn = mknod_valstr(N_VAL, n->value, NULL);
	if (!nn) {
		ass_error(n->lineno, "Memory allocation error");
	}
	// norm arg.
	if (n->n1) {
		struct node_t *nnn = eval_expr(n->n1);
		if (!nnn) {
			return NULL;
		}
		nn->next = nnn;
	}
	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_t_arg(struct node_t *n, int ic, int rel_op)
{
	struct node_t *arg = eval_expr(n);
	if (!arg) {
		return NULL;
	}

	struct node_t *nn = NULL;

	if (arg->type == N_VAL) {
		int jsval = arg->value;

		// arg is relative
		if (arg->is_addr && rel_op) {
			DEBUG("Got relative T-arg\n");
			jsval -= ic + 1;
		}

		// check range
		if ((jsval < -63) || (jsval > 63)) {
			ass_error(n->lineno, "Value outside short argument range");
			return NULL;
		}

		nn = mknod_valstr(N_VAL, 0, NULL);

		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}

		// set sign
		if (jsval < 0) {
			nn->value |= 0b0000001000000000;
			nn->value |= -jsval;
		} else {
			nn->value |= jsval;
		}
		nodes_drop(arg);
	} else {
		nn = arg;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_op_ka1(struct node_t *n)
{
	struct node_t *nn = NULL;

	// relative argument with instruction that use relative addresses?
	// IRB, DRB, LWS, RWS use adresses relative to IC
	int opl = (n->value >> 10) & 0b111;
	int rel_op = ((opl == 0b010) || (opl == 0b011) || (opl == 0b110) || (opl == 0b111)) ? 1 : 0;

	struct node_t *arg = eval_t_arg(n->n1, n->ic, rel_op);
	if (!arg) {
		return NULL;
	}

	if (arg->type == N_VAL) {
		nn = mknod_valstr(N_VAL, (n->value | arg->value), NULL);
		nodes_drop(arg);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
	} else {
		nn = dup_node(n);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
		nn->n1 = arg;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_op_js(struct node_t *n)
{
	struct node_t *nn = NULL;

	struct node_t *arg = eval_t_arg(n->n1, n->ic, 1);
	if (!arg) {
		return NULL;
	}

	if (arg->type == N_VAL) {
		nn = mknod_valstr(N_VAL, (n->value | arg->value), NULL);
		nodes_drop(arg);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
	} else {
		nn = dup_node(n);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
		nn->n1 = arg;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_op_ka2(struct node_t *n)
{
	struct node_t *arg = eval_expr(n->n1);

	if (!arg) {
		return NULL;
	}

	struct node_t *nn = NULL;

	if (arg->type == N_VAL) {
		if ((arg->value < 0) || (arg->value > 255)) {
			ass_error(n->lineno, "Value outside byte argument range");
			return NULL;
		}
		nn = mknod_valstr(N_VAL, (n->value | arg->value), NULL);
		nodes_drop(arg);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
	} else {
		nn = dup_node(n);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
		nn->n1 = arg;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_op_brc(struct node_t *n)
{
	struct node_t *arg = eval_expr(n->n1);

	if (!arg) {
		return NULL;
	}

	struct node_t *nn = NULL;

	if (arg->type == N_VAL) {
		if ((arg->value < 0) || (arg->value > 255)) {
			ass_error(n->lineno, "Bits set in left byte of BRC argument");
			return NULL;
		}
		nn = mknod_valstr(N_VAL, (n->value | arg->value), NULL);
		nodes_drop(arg);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
	} else {
		nn = dup_node(n);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
		nn->n1 = arg;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_op_blc(struct node_t *n)
{
	struct node_t *arg = eval_expr(n->n1);

	if (!arg) {
		return NULL;
	}

	struct node_t *nn = NULL;

	if (arg->type == N_VAL) {
		if (arg->value & 255) {
			ass_error(n->lineno, "Bits set in right byte of BLC argument (0x%04x)", arg->value);
			return NULL;
		}
		int blcval = (arg->value >> 8) & 255;
		nn = mknod_valstr(N_VAL, (n->value | blcval), NULL);
		nodes_drop(arg);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
	} else {
		nn = dup_node(n);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
		nn->n1 = arg;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_op_shc(struct node_t *n)
{
	struct node_t *arg = eval_expr(n->n1);

	if (!arg) {
		return NULL;
	}

	struct node_t *nn = NULL;

	if (arg->type == N_VAL) {
		int shcval = arg->value % 16;
		nodes_drop(arg);
		shcval = (shcval & 0b111) | ((shcval & 0b1000) << 6);
		nn = mknod_valstr(N_VAL, (n->value | shcval), NULL);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
	} else {
		nn = dup_node(n);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
		nn->n1 = arg;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_op_hlt(struct node_t *n)
{
	struct node_t *nn = NULL;

	struct node_t *arg = eval_t_arg(n->n1, n->ic, 0);
	if (!arg) {
		return NULL;
	}

	if (arg->type == N_VAL) {
		nn = mknod_valstr(N_VAL, (n->value | arg->value), NULL);
		nodes_drop(arg);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
	} else {
		nn = dup_node(n);
		if (!nn) {
			ass_error(n->lineno, "Memory allocation error");
			return NULL;
		}
		nn->n1 = arg;
	}

	return nn;
}

// -----------------------------------------------------------------------
struct node_t * eval_op(struct node_t *n)
{
	struct node_t *nn = NULL;

	DEBUG("eval_op()\n");

	switch (n->type) {
		case N_2ARG:
		case N_FD:
		case N_J:
		case N_L:
		case N_G:
		case N_BN:
		case N_S:
		case N_C:
			nn = eval_op_norm(n);
			break;
		case N_KA1:
			nn = eval_op_ka1(n);
			break;
		case N_JS:
			nn = eval_op_js(n);
			break;
		case N_EXL:
			exlize_names(n->n1);
		case N_KA2:
			nn = eval_op_ka2(n);
			break;
		case N_BRC:
			nn = eval_op_brc(n);
			break;
		case N_BLC:
			nn = eval_op_blc(n);
			break;
		case N_SHC:
			nn = eval_op_shc(n);
			break;
		case N_HLT:
			nn = eval_op_hlt(n);
			break;
		default:
			ass_error(n->lineno, "Unknown opcode node (type %i)", n->type);
			break;
	}

	return nn;
}

// -----------------------------------------------------------------------
void exlize_names(struct node_t *n)
{
	if (!n) return;
	if (n->type == N_NAME) n->type = N_EXLNAME;
	exlize_names(n->n1);
	exlize_names(n->n2);
}

// -----------------------------------------------------------------------
int ass_retry()
{
	struct node_t *nn = NULL;
	int res = E_OK;

	struct retry_t *r = retry;
	struct retry_t *parent = NULL;
	struct retry_t *child = NULL;

	retrying = 1;
	DEBUG("Retrying on level %i\n", syntax_level);

	// retry only nodes from current code segment
	while (r && (r->level == syntax_level)) {
		DEBUG("--- Retrying '%s' on level %i ------\n", r->n->str, syntax_level);
		parent = r->next;

		nn = compose(r->n);

		if (!nn) {
			res = E_ASS;
			break;
		}

		if (nn->type == N_VAL) { // if retry is OK, remove node from stack
			DEBUG("retry ok\n");
			res = img_put_at(r->n->at, r->n->ic, nn->value);
			if (res != E_OK) {
				ass_error(r->n->lineno, "Cannot update image at IC=%i, filepos=%i", r->n->ic, r->n->at);
			}
			nodes_drop(r->n);
			free(r);
			nodes_drop(nn);
			if (child) {
				child->next = parent;
			} else {
				retry = parent;
			}
		} else { // if retry failed...
			if (r->level <= 0) { // ...fail completly, if node's segment is the top one
				DEBUG("retry failed completly\n");
				res = E_ASS;
				break;
			} else { // ...move node to retry once again when parent segment folds
				DEBUG("Moving %s to level %i\n", r->n->str, r->level-1);
				(r->level)--;
				nn->ic = r->n->ic;
				nn->at = r->n->at;
				nodes_drop(r->n);
				r->n = nn;
				res = E_OK;
			}
			child = r;
		}
		r = parent;
	}
	DEBUG("Done retrying on level %i\n", syntax_level);
	retrying = 0;

	return res;
}

// -----------------------------------------------------------------------
int add_def(int type, int level, struct node_t *n)
{
	struct node_t *nn = NULL;

	if (type == D_LABEL) {
		nn = mknod_valstr(N_VAL, n->ic, NULL);
		nn->is_addr = 1;
	} else {
		nn = eval_expr(n->n1);
	}

	if (!nn) {
		return E_ASS;
	}

	struct dict_t *d = dict_find(n->str);

	if (d) { // name already defined
		if (type == D_LABEL) { // labels cannot be redefined
			nodes_drop(nn);
			return ass_error(n->lineno, "Label '%s' already defined", n->str);
		} else { // but variables can
			DEBUG("%i redefine variable: %s\n", level, n->str);
			nodes_drop(d->n);
			d->n = nn;
			char *varname = malloc(strlen(n->str)+2);
			sprintf(varname, "_%s", n->str);
			label_add(varname, nn->value);
			free(varname);
			return E_OK;
		}
	} else { // add new variable/label
		DEBUG("%i add %s: %s\n", level, (type==D_LABEL)?"label":"variable", n->str);
		dict_add(level, type, n->str, nn);
		if (type == D_LABEL) {
			label_add(n->str, n->ic);
		} else {
			char *varname = malloc(strlen(n->str)+2);
			sprintf(varname, "_%s", n->str);
			label_add(varname, nn->value);
			free(varname);
		}
		return E_OK;
	}
}

// -----------------------------------------------------------------------
int set_ic(struct node_t *n)
{
	int res;

	struct node_t *nn = eval_expr(n->n1);

	if (!nn || (nn->type != N_VAL)) {
		res = E_ASS;
	} else {
		res = img_set_ic(nn->value);
		if (res != E_OK) {
			ass_error(n->lineno, "Cannot set IC=%i", nn->value);
		}
	}
	nodes_drop(nn);
	return res;
}

// -----------------------------------------------------------------------
int set_ovl(struct node_t *n)
{
	int res;
	struct node_t *nn = eval_expr(n->n1);

	if (!nn || (nn->type != N_VAL)) {
		res = E_ASS;
	} else {
		res = img_next_sector(nn->value);
		if (res != E_OK) {
			ass_error(n->lineno, "Cannot set next overlay at IC=%i", nn->value);
		}
	}
	nodes_drop(nn);

	return res;
}

// -----------------------------------------------------------------------
int set_text(struct node_t *n)
{
	int res;

	struct node_t *nn = eval_expr(n->n1);

	if (!nn || (nn->type != N_VAL)) {
		res = E_ASS;
	} else {
		text_b = (nn->value >> 8) & 255;
		text_e = nn->value & 255;
		res = E_OK;
	}

	DEBUG("TEXT: %i / %i\n", text_b, text_e);

	nodes_drop(nn);
	return res;
}

// -----------------------------------------------------------------------
int conditional(int condition, struct node_t **n)
{
	int startline = (*n)->lineno;
	// skip nodes until FI*
	while (condition && n && ((*n)->type != N_FI)) {
		(*n)->next->ic = 0xffff;
		*n = (*n)->next;
	}
	if (n) {
		return E_OK;
	} else {
		return ass_error(startline, "Missing closing FI");
	}
}

// -----------------------------------------------------------------------
int flow_control(struct node_t **n)
{
	int res = E_ASS;
	struct dict_t *d = dict_find((*n)->str);

	switch ((*n)->type) {
		case N_LABEL:
			return add_def(D_LABEL, syntax_level, *n);
		case N_ALABEL:
			return add_def(D_LABEL, 0, *n);
		case N_VAR:
			return add_def(D_VARIABLE, syntax_level, *n);
		case N_AVAR:
			return add_def(D_VARIABLE, 0, *n);
		case N_SETIC:
			return set_ic(*n);
		case N_OVL:
			return set_ovl(*n);
		case N_PROG:
		case N_SEG:
		case N_MACRO:
			syntax_level++;
			DEBUG("BEG LEVEL: %i\n", syntax_level);
			return E_OK;
		case N_FINPROG:
		case N_FINSEG:
		case N_FINMACRO:
			res = ass_retry();
			dict_drop_level(syntax_level);
			DEBUG("FIN LEVEL: %i\n", syntax_level);
			syntax_level--;
			return res;
		case N_IFUNK:
			return conditional(d?1:0, n);
		case N_IFDEF:
			return conditional(!d?1:0, n);
		case N_FI:
			return E_OK; // alone FI* is OK, may happen when IF* was true
		case N_TEXT:
			return set_text(*n);
		default:
			return ass_error((*n)->lineno, "Unknown flow control node (type %i)", (*n)->type);
	}
}

// -----------------------------------------------------------------------
int retry_push(struct node_t *n)
{
	DEBUG("will retry later: %s (at IC=%i, pos=%i)\n", n->str, n->ic, n->at);
	struct retry_t *r = malloc(sizeof(struct retry_t));
	if (!r) {
		return ass_error(n->lineno, "Memory allocation error");
	}
	r->n = n;
	r->next = retry;
	r->level = syntax_level;
	retry = r;
	return E_OK;
}

// -----------------------------------------------------------------------
struct node_t * compose(struct node_t *n)
{
	struct node_t *nn;

	if (n->type <= N_OPS) {
		nn = eval_op(n);
	} else if (n->type <= N_WORD) {
		nn = eval_expr(n);
	} else if (n->type <= N_MWORD) {
		nn = eval_multi(n);
	} else {
		ass_error(n->lineno, "Trying to compose unknown node (type %i)", n->type);
		nn = NULL;
	}

	return nn;
}

// -----------------------------------------------------------------------
int write_out(struct node_t *n)
{
	int res;
	struct node_t *nn = compose(n);

	if (!n || !nn) {
		return E_ASS;
	}

	// write out all composed words
	struct node_t *next = NULL;
	while (nn) {
		if (nn->type == N_VAL) {
			// write word to image
			res = img_put(nn->value);
			next = nn->next;
			node_drop(nn);
			if (res != E_OK) {
				return ass_error(n->lineno, "Cannot update image at IC=%i", n->ic);
			}
		} else {
			nn->lineno = n->lineno;
			nn->at = img_get_filepos();
			res = retry_push(nn);
			if (res != E_OK) {
				return ass_error(n->lineno, "Cannot push node to retry");
			}
			res = img_inc_ic();
			if (res != E_OK) {
				return ass_error(n->lineno, "Cannot advance IC=%i", img_get_ic());
			}
			next = nn->next;
		}
		nn = next;
	}
	return E_OK;
}


// -----------------------------------------------------------------------
int assembly(struct node_t *n)
{
	int res = E_OK;

	while (n) {
		// set ic for a node
		n->ic = img_get_ic();

		DEBUG("---- IC: %i ---- LINE: %i -------------\n", n->ic, n->lineno);

		// node is empty, nothing to compose
		if (n->type <= N_EMPTY) {
			res = E_OK;
		// node is assembler flow control command
		} else if (n->type <= N_FLOWCTL) {
			res =  flow_control(&n);
		// node is something that produces output
		} else if (n->type <= N_MWORD) {
			res =  write_out(n);
		// shouldn't happen
		} else {
			res = ass_error(n->lineno, "Trying to assembly unknow node (type %i)", n->type);
		}

		// check result
		if (res != E_OK) {
			break;
		} else {
			n = n->next;
		}
	}

	return res;
}


// vim: tabstop=4
