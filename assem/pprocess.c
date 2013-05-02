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
int indent_step = 1;
char indent_string[] = "\t\t\t\t\t\t\t\t\t\0";
char *indent = indent_string + 10;

// -----------------------------------------------------------------------
char * pp_get_mnemo(struct node_t *n)
{
	int mask = 0;

	switch (n->type) {
		case N_KA2:
		case N_BRC:
		case N_BLC:
		case N_EXL:
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
		case N_EXLNAME:
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
		case N_BRC:
		case N_BLC:
		case N_HLT:
		case N_EXL:
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
			pos += sprintf(buf+pos, " r%i", ra);
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
int pp_compose_flow(char *buf, struct node_t *n)
{
	if (!n) return 0;

	int pos = 0;

	if ((n->type != N_ALABEL) && (n->type != N_LABEL)) {
		pos += sprintf(buf+pos, "\t%s%s ", indent, pp_get_pragma(n));
	}

	switch (n->type) {
		case N_PROG:
		case N_SEG:
		case N_MACRO:
			indent -= indent_step;
			break;
		case N_FINPROG:
		case N_FINSEG:
		case N_FINMACRO:
			indent += indent_step;
			break;
		case N_FI:
			break;
		case N_LABEL:
		case N_ALABEL:
			pos += sprintf(buf+pos, "%s:", n->str);
			break;
		case N_VAR:
			pos += sprintf(buf+pos, "%s ", n->str);
			pos += pp_eval(buf+pos, n->n1);
			break;
		case N_AVAR:
			pos += sprintf(buf+pos, "*%s ", n->str);
			pos += pp_eval(buf+pos, n->n1);
			break;
		case N_SETIC:
		case N_OVL:
			pos += pp_eval(buf+pos, n->n1);
			break;
		case N_IFUNK:
		case N_IFUND:
		case N_IFDEF:
			pos += sprintf(buf+pos, "%s\n", n->str);
			break;
		case N_TEXT:
			pos += sprintf(buf+pos, ".text ");
			pos += pp_eval(buf+pos, n->n1);
			break;
		default:
			pos += sprintf(buf+pos, "?");
			break;
	}

	return pos;
}

// -----------------------------------------------------------------------
int pp_compose_multi(char *buf, struct node_t *n)
{
	if (!n) return 0;

	int pos = 0;

	switch (n->type) {
		case N_STRING:
			pos += sprintf(buf+pos, ".data \"%s\"", n->str);
			break;
		case N_RES:
			pos += sprintf(buf+pos, ".res ");
			pos += pp_eval(buf+pos, n->n1);
			pos += sprintf(buf+pos, ", ");
			pos += pp_eval(buf+pos, n->n2);
			break;
		default:
			pos += sprintf(buf+pos, "?");
			break;
	}

	return pos;
}

// -----------------------------------------------------------------------
int pp_compose_empty(char *buf, struct node_t *n)
{
	if (!n) return 0;

	int pos = 0;
	char *s;

	switch (n->type) {
		case N_COMMENT:
			s = n->str;
			while (*s == ' ') s++;
			pos += sprintf(buf+pos, "\t\t\t/* %s */", s);
			break;
		case N_NL:
			*(buf+pos) = '\0';
			break;
		case N_LEN:
			pos += sprintf(buf+pos, "\n%s.len ", indent);
			pos += pp_eval(buf+pos, n->n1);
			break;
		case N_FILE:
			pos += sprintf(buf+pos, "\n%s.file %s, ", indent, n->str);
			pos += pp_eval(buf+pos, n->n1);
			pos += sprintf(buf+pos, ", ");
			pos += pp_eval(buf+pos, n->n2);
			break;
		default:
			pos += sprintf(buf+pos, "?");
			break;
	}


	return pos;
}

// -----------------------------------------------------------------------
void preprocess(struct nodelist_t *nl, FILE *ppf)
{
	char buf[1024];
	struct node_t *n = nl->head;

	while (n) {
		//printf("TYPE: %i\n", n->type);
		if (n->type <= N_EMPTY) {
			pp_compose_empty(buf, n);
			fprintf(ppf, "%s", buf);

		} else if (n->type <= N_FLOWCTL) {
			pp_compose_flow(buf, n);
			fprintf(ppf, "\n\t%s", buf);

		} else if (n->type <= N_OPS) {
			fprintf(ppf, "\n0x%04x: ", n->ic);
			pp_compose_opcode(buf, n);
			fprintf(ppf, "\t%s%s", indent, buf);

		} else if (n->type <= N_WORD) {
			fprintf(ppf, "\n0x%04x: ", n->ic);
			pp_eval(buf, n);
			fprintf(ppf, "\t%s.data %s", indent, buf);

		} else if (n->type <= N_MWORD) {
			fprintf(ppf, "\n0x%04x: ", n->ic);
			pp_compose_multi(buf, n);
			fprintf(ppf, "\t%s%s", indent, buf);

		} else {
			fprintf(ppf, "\n?");
		}
		n = n->next;
	}
}

// vim: tabstop=4
