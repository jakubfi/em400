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
#include "parser_modern.h"
#include "elements.h"
#include "ops.h"

struct node_t *program_start;
struct node_t *program_end;
struct dict_t **dict;
char assembly_error[1024];
int ic;

#define ASSEMBLY_ERROR(x) {strcpy(assembly_error, x); return -1;}

// -----------------------------------------------------------------------
int program_append(struct node_t *n)
{
	if (!n) {
		return -1;
	}

	// append given word list
	if (!program_end) {
		program_start = program_end = n;
		ic++;
	} else {
		program_end->next = n;
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
struct node_t * enode_eval(struct node_t *n, char *refcheck)
{
	if (!n) return NULL;

	struct dict_t *d = dict_find(dict, n->name);
	struct node_t *n1 = enode_eval(n->n1, refcheck);
	struct node_t *n2 = enode_eval(n->n2, refcheck);

	struct node_t *nv = make_value(0);

	switch (n->type) {
		case N_VAL:
			nv->data = n->data;
			nv->was_addr = n->was_addr;
			break;
		case N_NAME:
			if (!d) return NULL;
			if (d->name == refcheck) return NULL;
			struct node_t *dn = enode_eval(d->n, d->name);
			if (!dn) return NULL;
			nv->data = dn->data;
			if (d->type == D_ADDR) nv->was_addr = 1;
			break;
		case N_PLUS:
			if (!n1 || !n2) return NULL;
			nv->data = n1->data + n2->data;
			nv->was_addr = n1->was_addr | n2->was_addr;
			break;
		case N_MINUS:
			if (!n1 || !n2) return NULL;
			nv->data = n1->data - n2->data;
			nv->was_addr = n1->was_addr | n2->was_addr;
			break;
		case N_UMINUS:
			if (!n1) return NULL;
			nv->data= - n1->data;
			nv->was_addr = n1->was_addr;
			break;
		case N_SHL:
			if (!n1 || !n2) return NULL;
			nv->data = n1->data << n2->data;
			nv->was_addr = n1->was_addr | n2->was_addr;
			break;
		case N_SHR:
			if (!n1 || !n2) return NULL;
			nv->data = n1->data >> n2->data;
			nv->was_addr = n1->was_addr | n2->was_addr;
			break;
	}
	return nv;
}

// -----------------------------------------------------------------------
int compose_data(int ic, struct node_t *n, uint16_t *dt)
{
	// evaluate the argument -- data value
	struct node_t *nv = enode_eval(n, NULL);
	if (!nv) {
		ASSEMBLY_ERROR("cannot evaluate opcode's normal argument or data value");
	}
	*(dt+ic) = ntohs(nv->data);

	return 1;
}

// -----------------------------------------------------------------------
int compose_t_arg(uint16_t *dt, uint16_t ic, struct node_t *nv, int relative)
{
	int jsval;

	// some instructions use relative addres, calculate it, if so
	if (nv->was_addr && relative) {
		jsval = nv->data - ic - 1;
	} else {
		jsval = nv->data;
	}

	if ((jsval < -63) || (jsval > 63)) return -1;

	// compose T argument
	if (jsval < 0) {
		*(dt+ic) |= 0b0000001000000000;
		*(dt+ic) |= (-jsval) & 0b0000000000111111;
	} else {
		*(dt+ic) |= jsval & 0b0000000000111111;
	}

	return 0;
}

// -----------------------------------------------------------------------
int compose_opcode(int ic, struct node_t *n, uint16_t *dt)
{
	// compose opcode
	uint16_t *dtic = dt+ic;
	*dtic = n->opcode;

	int opl;
	int relative = 0;
	struct node_t *nv = NULL;

	switch (n->optype) {
		case O_KA1:
			nv = enode_eval(n->n1, NULL);
			opl = (n->opcode >> 10) & 0b111;
			// IRB, DRB, LWS, RWS use adresses relative to IC
			relative = ((opl == 0b010) || (opl == 0b011) || (opl == 0b110) || (opl == 0b111)) ? 1 : 0;
			if (compose_t_arg(dt, ic, nv, relative) < 0) {
				ASSEMBLY_ERROR("value excess short argument size");
			}
			break;
		case O_JS:
			nv = enode_eval(n->n1, NULL);
			// JS group jumps use adresses relative to IC
			if (compose_t_arg(dt, ic, nv, 1) < 0) {
				ASSEMBLY_ERROR("value excess short argument size");
			}
			break;
		case O_KA2:
			nv = enode_eval(n->n1, NULL);
			if ((nv->data < 0) || (nv->data > 255)) {
				ASSEMBLY_ERROR("value excess byte argument size");
			}
			*dtic |= (nv->data & 255);
			break;
		case O_SHC:
			nv = enode_eval(n->n1, NULL);
			if ((nv->data < 0) || (nv->data > 15)) {
				ASSEMBLY_ERROR("value excess SHC argument size");
			}
			*dtic |= (nv->data & 0b111);
			*dtic |= ((nv->data & 0b1000) << 6);
			break;
		case O_HLT:
			nv = enode_eval(n->n1, NULL);
			if (compose_t_arg(dt, ic, nv, 0) < 0) {
				ASSEMBLY_ERROR("value excess short argument size");
			}
			break;
		case O_2ARG:
		case O_FD:
		case O_J:
		case O_L:
		case O_G:
		case O_BN:
		case O_S:
		case O_C:
		default:
			break;
	}

	int ret = 0;

	// convert opcode data
	*dtic = ntohs(*dtic);

	return 1 + ret;
}


// vim: tabstop=4
