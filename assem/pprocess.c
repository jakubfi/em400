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
#include <inttypes.h>
#include <arpa/inet.h>

#include "eval.h"
#include "pprocess.h"
#include "parser_modern.h"
#include "elements.h"
#include "ops.h"

// -----------------------------------------------------------------------
char * pp_get_labels(struct dict_t **dict, int addr)
{
	struct dict_t *d;
	char *labels = malloc(1024);
	*labels = '\0';
	int len = 0;

	for (int i=0 ; i<(1<<DICT_HASH_BITS) ; i++ ) {
		d = dict[i];
		while (d) {
			if (d->type == D_ADDR) {
				struct node_t *n = expr_eval(d->n, NULL);
				if (n && (n->data == addr)) {
					sprintf(labels+len, "%s: ", d->name);
					len += strlen(d->name) + 2;
				}
				free(n);
			}
			d = d->next;
		}
	}

	return labels;
}

// -----------------------------------------------------------------------
char * pp_get_mnemo(struct node_t *n)
{
	int mask = 0;

	switch (n->type) {
		case N_KA2:
			mask = 0b1111111100000000;
			break;
		case N_SHC:
			mask = 0b1111110000111000;
			break;
		case N_2ARG:
		case N_KA1:
			mask = 0b1111110000000000;
			break;
		case N_FD:
		case N_JS:
		case N_J:
		case N_L:
		case N_G:
		case N_BN:
			mask = 0b1111110111000000;
			break;
		case N_S:
		case N_HLT:
			mask = 0b1111111111000111;
			break;
		case N_C:
			mask = 0b1111111000111111;
			break;
	}

	int opcode = n->opcode & mask;

	struct op_t *op = ops;

	while (op->mnemo[0]) {
			if (opcode == op->opcode) {
					return op->mnemo[0];
			}
			op++;
	}

	return NULL;
}


// -----------------------------------------------------------------------
char * pp_expr_eval(struct node_t *n)
{
	if (!n) return NULL;

	struct dict_t *d;
	char *buf = malloc(1024);
	char *s1 = pp_expr_eval(n->n1);
	char *s2 = pp_expr_eval(n->n2);

	switch (n->type) {
		case N_VAL:
			if (n->name) {
				sprintf(buf, "%s", n->name);
			} else {
				sprintf(buf, "%i", n->data);
			}
			break;
		case N_NAME:
			d = dict_find(dict, n->name);
			sprintf(buf, "%s", d->name);
			break;
		case N_PLUS:
			sprintf(buf, "%s + %s", s1, s2);
			break;
		case N_MINUS:
			sprintf(buf, "%s - %s", s1, s2);
			break;
		case N_UMINUS:
			sprintf(buf, "-%s", s1);
			break;
		case N_SHL:
			sprintf(buf, "%s << %s", s1, s2);
			break;
		case N_SHR:
			sprintf(buf, "%s >> %s", s1, s2);
			break;
	}

	free(s1);
	free(s2);

	return buf;
}

// -----------------------------------------------------------------------
int pp_compose_opcode(int ic, struct node_t *n, FILE *ppf)
{
	int ret = 1;
	int do_norm = 0;
	char *s = NULL;

	int d  = (n->opcode & 0b0000001000000000) >> 6;
	int ra = (n->opcode & 0b0000000111000000) >> 6;
	int rb = (n->opcode & 0b0000000000111000) >> 3;
	int rc = (n->opcode & 0b0000000000000111) >> 0;

	fprintf(ppf, "%-4s", pp_get_mnemo(n));

	switch (n->type) {
		case N_KA1:
		case N_SHC:
			fprintf(ppf, " r%i,", ra);
		case N_JS:
		case N_KA2:
			s = pp_expr_eval(n->next);
			fprintf(ppf, " %s", s);
			free(s);
			break;
		case N_HLT:
			s = pp_expr_eval(n->n1);
			fprintf(ppf, " %s", s);
			break;
		case N_2ARG:
			fprintf(ppf, " r%i,", ra);
		case N_FD:
		case N_J:
		case N_L:
		case N_G:
		case N_BN:
			do_norm = 1;
			break;
		case N_S:
			break;
		case N_C:
			fprintf(ppf, " r%i,", ra);
			break;
	}

	free(s);
	s = NULL;

	if (do_norm) {
		fprintf(ppf, " ");
		if (d) {
			fprintf(ppf, "[");
		}
		if (rc) {
			fprintf(ppf, "r%i", rc);
		} else {
			s = pp_expr_eval(n->next);
			fprintf(ppf, "%s", s);
			free(s);
			ret++;
		}
		if (rb) {
			fprintf(ppf, " + r%i", rb);
		}
		if (d) {
			fprintf(ppf, "]");
		}
	}

	return ret;
}


// vim: tabstop=4
