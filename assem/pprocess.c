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

#include "dict.h"
#include "pprocess.h"
#include "keywords.h"

int pp_mnemo_sel = MERA400;

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

	int opcode = n->value & mask;

	struct kw_t *kw = ops;

	while (kw->mnemo[0]) {
			if (opcode == kw->opcode) {
					return kw->mnemo[pp_mnemo_sel];
			}
			kw++;
	}

	return NULL;
}

// -----------------------------------------------------------------------
int pp_eval_2op(char *buf, char *op, struct node_t *n1, struct node_t *n2)
{
	int pos = 0;
	pos += pp_eval(buf+pos, n1);
	pos += sprintf(buf+pos, "%s", op);
	pos += pp_eval(buf+pos, n2);
	return pos;
}

// -----------------------------------------------------------------------
int pp_eval(char *buf, struct node_t *n)
{
	if (!n) return 0;

	int pos = 0;

	switch (n->type) {
		case N_VAL:
			if (n->str) {
				pos += sprintf(buf+pos, "%s", n->str);
			} else {
				pos += sprintf(buf+pos, "%i", n->value);
			}
			break;
		case N_NAME:
			pos += sprintf(buf+pos, "%s", n->str);
			break;
		case N_PLUS:
			pos += pp_eval_2op(buf+pos, "+", n->n1, n->n2);
			break;
		case N_MINUS:
			pos += pp_eval_2op(buf+pos, "-", n->n1, n->n2);
			break;
		case N_MUL:
			pos += pp_eval_2op(buf+pos, "*", n->n1, n->n2);
			break;
		case N_DIV:
			pos += pp_eval_2op(buf+pos, "/", n->n1, n->n2);
			break;
		case N_PAR:
			pos += pp_eval_2op(buf+pos, "(", NULL, n->n1);
			pos += sprintf(buf+pos, ")");
			break;
		case N_UMINUS:
			pos += pp_eval_2op(buf+pos, "-", NULL, n->n1);
			break;
		case N_SHL:
			pos += pp_eval_2op(buf+pos, "<<", n->n1, n->n2);
			break;
		case N_SHR:
			pos += pp_eval_2op(buf+pos, ">>", n->n1, n->n2);
			break;
		case N_SCALE:
			pos += pp_eval_2op(buf+pos, "//", n->n1, n->n2);
			break;
		default:
			*(buf+pos) = '\0';
			break;
	}

	return pos;
}

// -----------------------------------------------------------------------
int pp_compose_opcode(char *buf, struct node_t *n)
{
	int pos = 0;
	int do_norm = 0;

	int d  = (n->value & 0b0000001000000000) >> 6;
	int ra = (n->value & 0b0000000111000000) >> 6;
	int rb = (n->value & 0b0000000000111000) >> 3;
	int rc = (n->value & 0b0000000000000111) >> 0;

	pos += sprintf(buf+pos, "%-4s", pp_get_mnemo(n));

	switch (n->type) {
		case N_KA1:
		case N_SHC:
			pos += sprintf(buf+pos, " r%i,", ra);
		case N_JS:
		case N_KA2:
			pos += sprintf(buf+pos, " ");
			pos += pp_eval(buf+pos, n->n1);
			break;
		case N_HLT:
			pos += sprintf(buf+pos, " ");
			pos += pp_eval(buf+pos, n->n1);
			break;
		case N_2ARG:
			pos += sprintf(buf+pos, " r%i,", ra);
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
			pos += sprintf(buf+pos, " r%i,", ra);
			break;
	}

	if (do_norm) {
		pos += sprintf(buf+pos, " ");
		if (d) {
			pos += sprintf(buf+pos, "[");
		}
		if (rc) {
			pos += sprintf(buf+pos, "r%i", rc);
		} else {
			pos += pp_eval(buf+pos, n->n1);
		}
		if (rb) {
			pos += sprintf(buf+pos, "+r%i", rb);
		}
		if (d) {
			pos += sprintf(buf+pos, "]");
		}
	}

	return pos;
}

// -----------------------------------------------------------------------
char * pp_get_pragma(struct node_t *n)
{
	struct kw_t *kw = pragmas;
	while (kw->mnemo[0]) {
		if (n->type == kw->opcode) {
			return kw->mnemo[pp_mnemo_sel];
		}
		kw++;
	}
	return NULL;
}

// -----------------------------------------------------------------------
int pp_compose_pragma(char *buf, struct node_t *n)
{
	if (!n) return 0;

	int pos = 0;

	pos += sprintf(buf+pos, "%s", pp_get_pragma(n));

	if (n->type == N_AVAR) {
		pos += sprintf(buf+pos, " *");
	} else {
		pos += sprintf(buf+pos, " ");
	}

	if (n->str) {
		pos += sprintf(buf+pos, "%s", n->str);
	}

	if (n->n1) {
		pos += sprintf(buf+pos, " ");
		pos += pp_eval(buf+pos, n->n1);
	}

	return pos;
}

#define fprintf(file, format, ...) printf(format, ##__VA_ARGS__)

// -----------------------------------------------------------------------
void preprocess_new(struct nodelist_t *nl, FILE *ppf)
{
	char buf[1024];
	struct node_t *n = nl->head;

	while (n) {
		//printf("TYPE: %i\n", n->type);
		if (n->type <= N_EMPTY) {
			fprintf(ppf, "%s", n->str);
		} else if (n->type <= N_FLOWCTL) {
			pp_compose_pragma(buf, n);
			fprintf(ppf, "%s\n", buf);
		} else if (n->type <= N_OPS) {
			fprintf(ppf, "0x%04x: ", n->ic);
			pp_compose_opcode(buf, n);
			fprintf(ppf, "%s\n", buf);

		} else if (n->type <= N_WORD) {
			fprintf(ppf, "0x%04x: ", n->ic);
			pp_eval(buf, n);
			fprintf(ppf, ".data %s\n", buf);
		} else if (n->type <= N_MWORD) {
			fprintf(ppf, "?\n");
		} else {
			fprintf(ppf, "?\n");
		}
		n = n->next;
	}
}

// vim: tabstop=4
