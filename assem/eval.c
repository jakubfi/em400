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
#include <arpa/inet.h>

#include "eval.h"
#include "assem_parse.h"
#include "elements.h"
#include "ops.h"

struct word_t *program_start;
struct word_t *program_end;

char assembly_error[1024];
int ic;

#define ASSEMBLY_ERROR(x) {strcpy(assembly_error, x); return -1;}

// -----------------------------------------------------------------------
int program_append(struct word_t *word)
{
	if (!word) {
		return -1;
	}

	// append given word list
	if (!program_end) {
		program_start = program_end = word;
		ic++;
	} else {
		program_end->next = word;
	}

	// move program end and count words
	while (program_end->next) {
		program_end = program_end->next;
		ic++;
	}

	if (ic > MAX_PROG_SIZE) {
		return -1;
	}
	return 0;
}

// -----------------------------------------------------------------------
void program_drop(struct word_t *word)
{
	if (!word) {
		return;
	}

	enode_drop(word->e);
	if (word->norm) {
		free(word->norm);
	}

	program_drop(word->next);
	free(word);
}

// -----------------------------------------------------------------------
struct enode_t * enode_eval(struct enode_t *e)
{
	if (!e) return NULL;

	struct dict_t *d = dict_find(e->label);
	struct enode_t *e1 = enode_eval(e->e1);
	struct enode_t *e2 = enode_eval(e->e2);

	struct enode_t *ev = make_enode(VALUE, 0, NULL, NULL, NULL);

	switch (e->type) {
		case VALUE:
			ev->value = e->value;
			break;
		case NAME:
			if (!d) return NULL;
			ev->value = d->value;
			if (d->type == D_ADDR) ev->was_addr = 1;
			break;
		case '+':
			if (!e1 || !e2) return NULL;
			ev->value = e1->value + e2->value;
			if (e1->was_addr || e2->was_addr) ev->was_addr = 1;
			break;
		case '-':
			if (!e1 || !e2) return NULL;
			ev->value = e1->value - e2->value;
			if (e1->was_addr || e2->was_addr) ev->was_addr = 1;
			break;
	}
	return ev;
}

// -----------------------------------------------------------------------
int compose_data(struct word_t *word, uint16_t *dt)
{
	int res = 0;

	// evaluate the argument -- data value
	struct enode_t *ev = enode_eval(word->e);
	if (!ev) {
		ASSEMBLY_ERROR("cannot evaluate data value");
	}

	*dt = ntohs(ev->value);
	res++;

	return res;
}

// -----------------------------------------------------------------------
int make_bin(int ic, struct word_t *word, uint16_t *dt)
{
	int res = 0;
	struct enode_t *ev;

	// compose data
	if (word->type == W_DATA) {
		res = compose_data(word, dt+ic);
		if (res < 0) ASSEMBLY_ERROR("cannot compose data word");
		return res;
	}

	// evaluate the argument, if exists (meaning opcode uses it)
	ev = enode_eval(word->e);
	if (word->e && !ev) {
		ASSEMBLY_ERROR("cannot evaluate opcode's argument");
	}

	// compose opcode
	uint16_t *dtic = dt+ic;
	*dtic = 0;
	*dtic |= word->opcode;
	*dtic |= word->ra << 6;

	int jsval;

	switch (word->type) {
		case OP_2ARG:
			break;
		case OP_FD:
			break;
		case W_OP_KA1:
			if ((ev->value < -63) || (ev->value > 63)) ASSEMBLY_ERROR("value excess short argument size");
			if (ev->value < 0) {
				*dtic |= 0b0000001000000000;
				*dtic |= (-ev->value) & 0b0000000000111111;
			} else {
				*dtic |= ev->value & 0b0000000000111111;
			}
			break;
		case W_OP_JS:
			if (ev->was_addr) {
				jsval = ev->value - ic - 1;
			} else {
				jsval = ev->value;
			}
			if ((jsval < -63) || (jsval > 63)) ASSEMBLY_ERROR("value excess short argument size");
			if (jsval < 0) {
				*dtic |= 0b0000001000000000;
				*dtic |= (-jsval) & 0b0000000000111111;
			} else {
				*dtic |= jsval & 0b0000000000111111;
			}
			break;
		case W_OP_KA2:
			if ((ev->value < 0) || (ev->value > 255)) ASSEMBLY_ERROR("value excess byte argument size");
			*dtic |= (ev->value & 256);
			break;
		case OP_C:
			break;
		case W_OP_SHC:
			if ((ev->value < 0) || (ev->value > 15)) ASSEMBLY_ERROR("value excess SHC argument size");
			*dtic |= (ev->value & 0b111);
			*dtic |= ((ev->value & 0b1000) << 6);
			break;
		case OP_S:
			break;
		case OP_J:
			break;
		case OP_L:
			break;
		case OP_G:
			break;
		case OP_BN:
			break;
	}

	// compose normal argument (opcode part) if exists
	if (word->norm) {
		*dtic |= (word->norm->is_addr << 9);
		*dtic |= word->norm->rc;
		*dtic |= (word->norm->rb << 3);
	}

	// write out opcode data
	*dtic = ntohs(*dtic);
	res++;

	return res;
}


// vim: tabstop=4
