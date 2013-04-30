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

int pp_mnemo_sel = MNEMO_MERA400;

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
char * pp_eval(struct node_t *n)
{
	if (!n) return NULL;

	char *buf = malloc(1024 * sizeof(char));
	char *s1 = pp_eval(n->n1);
	char *s2 = pp_eval(n->n2);

	switch (n->type) {
		case N_VAL:
			if (n->str) {
				sprintf(buf, "%s", n->str);
			} else {
				sprintf(buf, "%i", n->value);
			}
			break;
		case N_NAME:
			sprintf(buf, "%s", n->str);
			break;
		case N_PLUS:
			sprintf(buf, "%s+%s", s1, s2);
			break;
		case N_MINUS:
			sprintf(buf, "%s-%s", s1, s2);
			break;
		case N_UMINUS:
			sprintf(buf, "-%s", s1);
			break;
		case N_SHL:
			sprintf(buf, "%s<<%s", s1, s2);
			break;
		case N_SHR:
			sprintf(buf, "%s>>%s", s1, s2);
			break;
		case N_SCALE:
			sprintf(buf, "%s/%s", s1, s2);
			break;
		default:
			*buf = '\0';
			break;
	}

	free(s1);
	free(s2);

	return buf;
}

// -----------------------------------------------------------------------
char * pp_compose_opcode(int ic, struct node_t *n, int *norm_arg)
{
	char *s = NULL;
	char *buf = malloc(1024 * sizeof(char));
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
			s = pp_eval(n->n1);
			pos += sprintf(buf+pos, " %s", s);
			free(s);
			break;
		case N_HLT:
			s = pp_eval(n->n1);
			pos += sprintf(buf+pos, " %s", s);
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

	free(s);
	s = NULL;

	if (do_norm) {
		pos += sprintf(buf+pos, " ");
		if (d) {
			pos += sprintf(buf+pos, "[");
		}
		if (rc) {
			pos += sprintf(buf+pos, "r%i", rc);
		} else {
			*norm_arg = 1;
			s = pp_eval(n->next);
			pos += sprintf(buf+pos, "%s", s);
			free(s);
		}
		if (rb) {
			pos += sprintf(buf+pos, "+r%i", rb);
		}
		if (d) {
			pos += sprintf(buf+pos, "]");
		}
	}

	return buf;
}

// -----------------------------------------------------------------------
char * pp_compose_pragma(struct node_t *n)
{
	if (!n) return NULL;

	char *buf = malloc(1024 * sizeof(char));
	int pos = 0;

	struct kw_t *kw = pragmas;

	while (kw->mnemo[0]) {
		if (n->type == kw->opcode) {
			pos += sprintf(buf+pos, "%s", kw->mnemo[pp_mnemo_sel]);
		}
		kw++;
	}

	if (n->str) {
		pos += sprintf(buf+pos, " %s", n->str);
	}

	if (n->n1) {
		pos += sprintf(buf+pos, " %s", pp_eval(n->n1));
	}

	return buf;
}

// -----------------------------------------------------------------------
void preprocess_new(struct nodelist_t *nl, FILE *ppf)
{
	struct node_t *n = nl->head;

	while (n) {
		if (n->type <= N_EMPTY) {
			fprintf(ppf, "%s", n->str);
		} else if (n->type <= N_FLOWCTL) {
			char *s = pp_compose_pragma(n);
			fprintf(ppf, "%s\n", s);
			free(s);
		} else if (n->type <= N_OPS) {
			fprintf(ppf, "0x%04x: ", n->ic);
			int norm_arg = 0;
			char *s = pp_compose_opcode(-1, n, &norm_arg);
			fprintf(ppf, "%s\n", s);
			free(s);

			// if there was additional word in normal argument, we've processed it already in pp_compose_opcode()
			if (norm_arg) {
				n = n->next;
			}

		} else if (n->type <= N_WORD) {
			fprintf(ppf, "0x%04x: ", n->ic);
			char *s = pp_eval(n);
			fprintf(ppf, ".data %s\n", s);
			free(s);
		} else if (n->type <= N_MWORD) {
			fprintf(ppf, "?\n");
		} else {
			fprintf(ppf, "?\n");
		}
		n = n->next;
	}
}

// -----------------------------------------------------------------------
int preprocess(struct nodelist_t *nl, FILE *ppf)
{
	int ic = 0;
	int res;
	int linelen;

	struct node_t *n = nl->head;

	while (n) {
		linelen = 0;

		// empties and pragmas
		if (n->type <= N_EMPTY) {
			if (n->type == N_COMMENT) {
				fprintf(ppf, "        ; %s", n->str);
				linelen += strlen(n->str);
			} else {
				fprintf(ppf, "        %s", n->str);
				linelen += strlen(n->str);
			}
		} else if (n->type <= N_FLOWCTL) {
			char *p = pp_compose_pragma(n);
			fprintf(ppf, "        %s", p);
			linelen += strlen(n->str);
			if (n->n1) {
				char *s = pp_eval(n->n1);
				fprintf(ppf, "%s", s);
				linelen += strlen(s);
			}
			res = 0;
		} else {

			// address
			fprintf(ppf, "0x%04x: ", ic);
			linelen += 8;

			// opcode...
			if (n->type < N_OPS) {
				int norm_arg = 0;
				char *s = pp_compose_opcode(ic, n, &norm_arg);
				fprintf(ppf, "        %s", s);
				linelen += strlen(s);
				free(s);
				res = 1;

				// if there was additional word in normal argument, we've processed it already in pp_compose_opcode()
				if (norm_arg) {
					n = n->next;
					res++;
				}

			// ... or data
			} else {
				char *s = pp_eval(n);
				fprintf(ppf, "        .data %s", s);
				linelen += strlen(s);
				free(s);
				res = 1;
			}

		}

		fprintf(ppf, "\n");
		n = n->next;
		ic += res;
	}

	return ic;
}


// vim: tabstop=4
