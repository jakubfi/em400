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

#ifndef NODES_H
#define NODES_H

enum node_type_e {
	// no-data nodes
	N_DUMMY = -1000,
	N_COMMENT,
	N_NL,
	N_LEN,		// unhandled pragma
	N_FILE,		// unhandled pragma
	N_EMPTY = -500,

	// flow
	N_PROG,
	N_FINPROG,
	N_SEG,
	N_FINSEG,
	N_MACRO,
	N_FINMACRO,
	N_LABEL,
	N_ALABEL,
	N_VAR,
	N_AVAR,
	N_SETIC,
	N_OVL,
	N_IFUNK,
	N_IFUND,
	N_IFDEF,
	N_FI,
	N_TEXT,
	N_FLOWCTL = -1,

	// opcodes
	N_2ARG = 0,
	N_FD,
	N_KA1,
	N_JS,
	N_KA2,
	N_BRC,
	N_BLC,
	N_EXL,
	N_C,
	N_SHC,
	N_HLT,
	N_S,
	N_J,
	N_L,
	N_G,
	N_BN,
	N_OPS = 1000,

	// data nodes - single word
	N_VAL,
	N_NAME,
	N_EXLNAME,
	N_PLUS,
	N_MINUS,
	N_MUL,
	N_DIV,
	N_UMINUS,
	N_PAR,
	N_SHL,
	N_SHR,
	N_SCALE,
	N_WORD = 2000,

	// data nodes - multi word
	N_STRING,
	N_RES,
	N_MWORD = 3000,

	N_ERR,
};

struct norm_t {
	int rb, rc, d;
	struct node_t *e;
};

struct node_t {
	int ic;						// instruction counter for a node
	int at;						// position in output file (needed by retry())
	int type;					// node type
	int value;					// value (meaning differs between node types)
	int is_addr;				// node is an address (for ops with relative arguments)
	char *str;					// string (meaning differs)
	struct node_t *n1, *n2;		// child nodes in syntax structure (meaning differs)
	struct node_t *next;		// next node in syntax structure
	int lineno;					// line number, at which node was born
};

struct nodelist_t {
	struct node_t *head;
	struct node_t *tail;
};

struct node_t * make_node(int type);
struct node_t * dup_node(struct node_t *sn);
struct norm_t * make_norm(int rc, int rb, struct node_t *n);

void node_drop(struct node_t *n);
void nodes_drop(struct node_t *n);
void nodelist_drop(struct nodelist_t *nl);
struct nodelist_t * make_nl(struct node_t *n);
struct nodelist_t * nl_append_n(struct nodelist_t *nl, struct node_t *n);
struct nodelist_t * nl_append(struct nodelist_t *nl1, struct nodelist_t *nl2);


#endif

// vim: tabstop=4
