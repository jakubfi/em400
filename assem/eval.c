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
int program_ic;

struct dict_t **dict;
char assembly_error[1024];

// -----------------------------------------------------------------------
void ass_error(const char *str)
{
	strcpy(assembly_error, str);
}

// -----------------------------------------------------------------------
int program_append(struct node_t *n)
{
	int size = 0;

	while (n) {
		printf("apend: %p->%p (%i) %s %s \n", n, n->next, n->type, n->name, n->comment);
		if (!program_end) {
			program_start = program_end = n;
		} else {
			program_end->next = n;
			program_end = program_end->next;
		}
		if (n->type != N_DUMMY) {
			size++;
		}
		n = n->next;
	}

	program_ic += size;
	if (program_ic > MAX_PROG_SIZE) {
		return -1;
	}

	return size;
}

// -----------------------------------------------------------------------
struct node_t * expr_eval(struct node_t *n, char *refcheck)
{
	if (!n) return NULL;

	struct dict_t *d = dict_find(dict, n->name);
	struct node_t *n1 = expr_eval(n->n1, refcheck);
	struct node_t *n2 = expr_eval(n->n2, refcheck);

	struct node_t *nv = make_value(0, NULL);

	switch (n->type) {
		case N_VAL:
			nv->data = n->data;
			nv->was_addr = n->was_addr;
			break;
		case N_NAME:
			if (!d) return NULL;
			if (d->name == refcheck) return NULL;
			struct node_t *dn = expr_eval(d->n, d->name);
			if (!dn) return NULL;
			nv->data = dn->data;
			if (d->type == D_ADDR) nv->was_addr = 1;
			free(dn);
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

	free(n1);
	free(n2);

	return nv;
}

// -----------------------------------------------------------------------
int compose_data(struct node_t *n, uint16_t *dt)
{
	struct node_t *nv = expr_eval(n, NULL);
	if (!nv) {
		ass_error("cannot evaluate opcode's normal argument or data value");
		return 0;
	}
	*dt = ntohs(nv->data);
	free(nv);
	return 1;
}

// -----------------------------------------------------------------------
int prepare_t_arg(uint16_t *dt, uint16_t ic, struct node_t *n, int relative)
{
	int jsval;
	struct node_t *nv = expr_eval(n, NULL);
	if (!nv) {
		ass_error("cannot evaluate opcode's short argument");
		return 0;
	}

	// some instructions use relative addresses
	if (nv->was_addr && relative) {
		jsval = nv->data - ic - 1;
	} else {
		jsval = nv->data;
	}

	if ((jsval < -63) || (jsval > 63)) {
		ass_error("value exceeds short argument size");
		return 0;
	}

	// compose T argument
	if (jsval < 0) {
		*dt |= 0b0000001000000000;
		*dt |= (-jsval) & 0b0000000000111111;
	} else {
		*dt |= jsval & 0b0000000000111111;
	}
	free(nv);
	return 1;
}

// -----------------------------------------------------------------------
int compose_opcode(int ic, struct node_t *n, uint16_t *dt)
{
	int ret = 1;
	int opl;
	int relative = 0;
	struct node_t *nv = NULL;

	*dt = n->opcode;

	switch (n->type) {
		case N_KA1:
			opl = (n->opcode >> 10) & 0b111;
			// IRB, DRB, LWS, RWS use adresses relative to IC
			relative = ((opl == 0b010) || (opl == 0b011) || (opl == 0b110) || (opl == 0b111)) ? 1 : 0;
			ret = prepare_t_arg(dt, ic, n->n1, relative);
			break;
		case N_JS:
			// JS group jumps use adresses relative to IC
			ret = prepare_t_arg(dt, ic, n->n1, 1);
			break;
		case N_KA2:
			nv = expr_eval(n->n1, NULL);
			if (!nv) {
				ass_error("cannot evaluate opcode's byte argument");
				ret = 0;
				break;
			}
			if ((nv->data < 0) || (nv->data > 255)) {
				ass_error("value exceeds byte argument size");
				ret = 0;
				break;
			}
			*dt |= (nv->data & 255);
			break;
		case N_SHC:
			nv = expr_eval(n->n1, NULL);
			if (!nv) {
				ass_error("cannot evaluate SHC argument");
				ret = 0;
				break;
			}
			if ((nv->data < 0) || (nv->data > 15)) {
				ass_error("value exceeds SHC argument size");
				ret = 0;
				break;
			}
			*dt |= (nv->data & 0b111);
			*dt |= ((nv->data & 0b1000) << 6);
			break;
		case N_HLT:
			ret = prepare_t_arg(dt, ic, n->n1, 0);
			break;
		case N_2ARG:
		case N_FD:
		case N_J:
		case N_L:
		case N_G:
		case N_BN:
		case N_S:
		case N_C:
		default:
			break;
	}

	// convert opcode data
	*dt = ntohs(*dt);
	free(nv);
	return ret;
}

// -----------------------------------------------------------------------
int assembly(struct node_t *n, uint16_t *outdata)
{
	int ic = 0;
	int res;

	while (n) {
		if (n->type != N_DUMMY ) {
			// opcode
			if (n->type <= N_BN) {
				res = compose_opcode(ic, n, outdata);
			// data
			} else {
				res = compose_data(n, outdata);
			}

			if (res != 1) {
				return -(ic+1);
			}
			ic++;
			outdata++;
		}

		n = n->next;
	}

	return ic;
}


// vim: tabstop=4
