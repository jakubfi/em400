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

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "assem_parse.h"
#include "assem_ops.h"

struct label_t *label_first;
struct label_t *label_last;

struct word_t *word_first;
struct word_t *word_last;

struct op_t ops[] = {

{ "LW", OP_2ARG, 0b0100000000000000 },
{ "TW", OP_2ARG, 0b0100010000000000 },
{ "LS", OP_2ARG, 0b0100100000000000 },
{ "RI", OP_2ARG, 0b0100110000000000 },
{ "RW", OP_2ARG, 0b0101000000000000 },
{ "PW", OP_2ARG, 0b0101010000000000 },
{ "RJ", OP_2ARG, 0b0101100000000000 },
{ "IS", OP_2ARG, 0b0101110000000000 },
{ "BB", OP_2ARG, 0b0110000000000000 },
{ "BM", OP_2ARG, 0b0110010000000000 },
{ "BS", OP_2ARG, 0b0110100000000000 },
{ "BC", OP_2ARG, 0b0110110000000000 },
{ "BN", OP_2ARG, 0b0111000000000000 },
{ "OU", OP_2ARG, 0b0111010000000000 },
{ "IN", OP_2ARG, 0b0111100000000000 },

{ "AD", OP_FD, 0b0111110000000000 },
{ "SD", OP_FD, 0b0111110001000000 },
{ "MW", OP_FD, 0b0111110010000000 },
{ "DW", OP_FD, 0b0111110011000000 },
{ "AF", OP_FD, 0b0111110100000000 },
{ "SF", OP_FD, 0b0111110101000000 },
{ "MF", OP_FD, 0b0111110110000000 },
{ "DF", OP_FD, 0b0111110111000000 },

{ "AW", OP_2ARG, 0b1000000000000000 },
{ "AC", OP_2ARG, 0b1000010000000000 },
{ "SW", OP_2ARG, 0b1000100000000000 },
{ "CW", OP_2ARG, 0b1000110000000000 },
{ "OR", OP_2ARG, 0b1001000000000000 },
{ "OM", OP_2ARG, 0b1001010000000000 },
{ "NR", OP_2ARG, 0b1001100000000000 },
{ "NM", OP_2ARG, 0b1001110000000000 },
{ "ER", OP_2ARG, 0b1010000000000000 },
{ "EM", OP_2ARG, 0b1010010000000000 },
{ "XR", OP_2ARG, 0b1010100000000000 },
{ "XM", OP_2ARG, 0b1010110000000000 },
{ "CL", OP_2ARG, 0b1011000000000000 },
{ "LB", OP_2ARG, 0b1011010000000000 },
{ "RB", OP_2ARG, 0b1011100000000000 },
{ "CB", OP_2ARG, 0b1011110000000000 },

{ "AWT", OP_KA1, 0b1100000000000000 },
{ "TRB", OP_KA1, 0b1100010000000000 },
{ "IRB", OP_KA1, 0b1100100000000000 },
{ "DRB", OP_KA1, 0b1100110000000000 },
{ "CWT", OP_KA1, 0b1101000000000000 },
{ "LWT", OP_KA1, 0b1101010000000000 },
{ "LWS", OP_KA1, 0b1101100000000000 },
{ "RWS", OP_KA1, 0b1101110000000000 },

{ "UJS", OP_JS, 0b1110000000000000 },
{ "JLS", OP_JS, 0b1110000001000000 },
{ "JES", OP_JS, 0b1110000010000000 },
{ "JGS", OP_JS, 0b1110000011000000 },
{ "JVS", OP_JS, 0b1110000100000000 },
{ "JXS", OP_JS, 0b1110000101000000 },
{ "JYS", OP_JS, 0b1110000110000000 },
{ "JCS", OP_JS, 0b1110000111000000 },

{ "BLC", OP_KA2, 0b1110010000000000 },
{ "EXL", OP_KA2, 0b1110010100000000 },
{ "BRC", OP_KA2, 0b1110011000000000 },
{ "NRF", OP_KA2, 0b1110011100000000 },

{ "RIC", OP_C, 0b1110100000000000 },
{ "ZLB", OP_C, 0b1110100000000001 },
{ "SXU", OP_C, 0b1110100000000010 },
{ "NGA", OP_C, 0b1110100000000011 },
{ "SLZ", OP_C, 0b1110100000000100 },
{ "SLY", OP_C, 0b1110100000000101 },
{ "SLX", OP_C, 0b1110100000000110 },
{ "SRY", OP_C, 0b1110100000000111 },
{ "NGL", OP_C, 0b1110100000001000 },
{ "RPC", OP_C, 0b1110100000001001 },
{ "SHC", OP_SHC, 0b1110100000010000 },
{ "RKY", OP_C, 0b1110101000000000 },
{ "ZRB", OP_C, 0b1110101000000001 },
{ "SXL", OP_C, 0b1110101000000010 },
{ "NGC", OP_C, 0b1110101000000011 },
{ "SVZ", OP_C, 0b1110101000000100 },
{ "SVY", OP_C, 0b1110101000000101 },
{ "SVX", OP_C, 0b1110101000000110 },
{ "SRX", OP_C, 0b1110101000000111 },
{ "SRZ", OP_C, 0b1110101000001000 },
{ "LPC", OP_C, 0b1110101000001001 },

{ "HLT", OP_JS, 0b1110110000000000 },
{ "MCL", OP_S, 0b1110110001000000 },
{ "CIT", OP_S, 0b1110110010000000 },
{ "SIL", OP_S, 0b1110110010000001 },
{ "SIU", OP_S, 0b1110110010000010 },
{ "SIT", OP_S, 0b1110110010000011 },
{ "GIU", OP_S, 0b1110110011000000 },
{ "LIP", OP_S, 0b1110110100000000 },
{ "GIL", OP_S, 0b1110111011000000 },

{ "UJ", OP_J, 0b1111000000000000 },
{ "JL", OP_J, 0b1111000001000000 },
{ "JE", OP_J, 0b1111000010000000 },
{ "JG", OP_J, 0b1111000011000000 },
{ "JZ", OP_J, 0b1111000100000000 },
{ "JM", OP_J, 0b1111000101000000 },
{ "JN", OP_J, 0b1111000110000000 },
{ "LJ", OP_J, 0b1111000111000000 },

{ "LD", OP_L, 0b1111010000000000 },
{ "LF", OP_L, 0b1111010001000000 },
{ "LA", OP_L, 0b1111010010000000 },
{ "LL", OP_L, 0b1111010011000000 },
{ "TD", OP_L, 0b1111010100000000 },
{ "TF", OP_L, 0b1111010101000000 },
{ "TA", OP_L, 0b1111010110000000 },
{ "TL", OP_L, 0b1111010111000000 },

{ "RD", OP_G, 0b1111100000000000 },
{ "RF", OP_G, 0b1111100001000000 },
{ "RA", OP_G, 0b1111100010000000 },
{ "RL", OP_G, 0b1111100011000000 },
{ "PD", OP_G, 0b1111100100000000 },
{ "PF", OP_G, 0b1111100101000000 },
{ "PA", OP_G, 0b1111100110000000 },
{ "PL", OP_G, 0b1111100111000000 },

{ "MB", OP_BN, 0b1111110000000000 },
{ "IM", OP_BN, 0b1111110001000000 },
{ "KI", OP_BN, 0b1111110010000000 },
{ "FI", OP_BN, 0b1111110011000000 },
{ "SP", OP_BN, 0b1111110100000000 },
{ "MD", OP_BN, 0b1111110101000000 },
{ "RZ", OP_BN, 0b1111110110000000 },
{ "IB", OP_BN, 0b1111110111000000 },

{ "", 0, 0}

};

// -----------------------------------------------------------------------
struct op_t * get_op(char * opname)
{
	struct op_t *op = ops;

	char *opname_u = strdup(opname);
	char *c = opname_u;
	while (*c) {
		*c = toupper(*c);
		c++;
	}

	while (op->mnemo) {
		if (!strcmp(op->mnemo, opname_u)) {
			free(opname_u);
			return op;
		}
		op++;
	}
	free(opname_u);
	return NULL;
}

// -----------------------------------------------------------------------
struct vval_t * make_vval(int value, char *label)
{
	struct vval_t *vval = malloc(sizeof(struct vval_t));
	vval->value = value;
	if (label) {
		vval->label = strdup(label);
	} else {
		vval->label = NULL;
	}
	return vval;
}

// -----------------------------------------------------------------------
struct norm_t * make_norm(int rc, int rb, struct vval_t *vval)
{
	struct norm_t *norm = malloc(sizeof(struct norm_t));
	norm->rc = rc;
	norm->rb = rb;
	norm->vval = vval;
	norm->is_addr = 0;
	return norm;
}

// -----------------------------------------------------------------------
struct word_t * make_op(int type, uint16_t op, int ra, int value, struct norm_t *norm)
{
	struct word_t *word = malloc(sizeof(struct word_t));
	word->type = type;
	word->opcode = op;
	word->ra = ra;
	word->value = value;
	word->norm = norm;
	return word;
}

// -----------------------------------------------------------------------
void label_add(int addr, char *name)
{
	struct label_t *label = malloc(sizeof(struct label_t));
	label->addr = addr;
	label->name = strdup(name);

	if (!label_first) {
		label_first = label_last = label;
	} else {
		label_last->next = label;
		label_last = label;
	}
}

// -----------------------------------------------------------------------
struct label_t * label_find(char *name)
{
	struct label_t *l = label_first;
	while (l) {
		if (!strcmp(name, l->name)) {
			return l;
		}
		l = l->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------
void word_add(struct word_t *word)
{
	if (!word_first) {
		word_first = word_last = word;
	} else {
		word_last->next = word;
		word_last = word;
	}
}

// -----------------------------------------------------------------------
int program_write(struct word_t *word, FILE *out)
{
	int res = 0;

	while (word) {
		uint16_t code = 0;

		if (word->type == W_DATA) {
			code = word->value;

		} else {
			code |= word->opcode;
			code |= word->ra << 6;
			if (word->norm) {
				code |= (word->norm->is_addr << 9);
				code |= word->norm->rc;
				code |= (word->norm->rb << 3);
			}
			if ((word->type == W_OP_KA1) || (word->type == W_OP_JS)) {
				if (word->value < 0) {
					code |= 0b0000001000000000;
				}
				code |= word->value & 0b0000000000111111;
			}
			if (word->type == W_OP_KA2) {
				code |= (word->value & 256);
			}
			if (word->type == W_OP_SHC) {
				code |= (word->value & 0b111);
				code |= ((word->value & 0b1000) << 6);
			}
		}

		code = ntohs(code);
		res += fwrite(&code, 2, 1, out);

		if ((word->norm) && (word->norm->rc == 0)) {
			uint16_t data;
			if (!word->norm->vval->label) {
				data = ntohs(word->norm->vval->value);
			} else {
				struct label_t * l = label_find(word->norm->vval->label);
				if (!l) {
					printf("Can't find label: %s\n", word->norm->vval->label);
				} else {
					data = ntohs(l->addr);
				}
			}
			res += fwrite(&data, 2, 1, out);
		}

		word = word->next;
	}
	return res;
}


// vim: tabstop=4
