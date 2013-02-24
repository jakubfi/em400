%{
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
#include <stdarg.h>
#include "assem_ops.h"

void yyerror(char *);
int yylex(void);
extern FILE *yyin;

int ic = 0;
%}

%union {
	char *str;
	int val;
	struct word_t *word;
	struct norm_t *norm;
	struct vval_t *vval;
};

%token '[' ']' ',' '+'
%token DATA
%token <str> DLABEL NAME
%token <val> VALUE REGISTER
%token <val> OP_2ARG OP_FD OP_KA1 OP_JS OP_KA2 OP_C OP_SHC OP_S OP_J OP_L OP_G OP_BN

%type <norm> normval norm
%type <word> data instruction
%type <vval> val

%%

sentences:
	sentence sentences
	|
	;

sentence:
	instruction {
		word_add($1);
		ic++;
		if (($1->norm) && ($1->norm->rc == 0)) {
			ic++;
		}
	}
	| data {
		word_add($1);
		ic++;
	}
	| DLABEL { label_add(ic, $1); }
	;

data:
	DATA VALUE { $$ = make_op(W_DATA, 0, 0, $2, NULL); }
	;

instruction:
	OP_2ARG REGISTER ',' norm { $$ = make_op(W_OP_2ARG, $1, $2, 0, $4); }
	| OP_FD norm { $$ = make_op(W_OP_FD, $1, 0, 0, $2); }
	| OP_KA1 REGISTER VALUE { $$ = make_op(W_OP_KA1, $1, $2, $3, NULL); }
	| OP_JS VALUE { $$ = make_op(W_OP_JS, $1, 0, $2, NULL); }
	| OP_KA2 VALUE { $$ = make_op(W_OP_KA2, $1, 0, $2, NULL); }
	| OP_C REGISTER { $$ = make_op(W_OP_C, $1, $2, 0, NULL); }
	| OP_SHC VALUE { $$ = make_op(W_OP_SHC, $1, 0, $2, NULL); }
	| OP_S { $$ = make_op(W_OP_S, $1, 0, 0, NULL); }
	| OP_J norm { $$ = make_op(W_OP_J, $1, 0, 0, $2); }
	| OP_L norm { $$ = make_op(W_OP_L, $1, 0, 0, $2); }
	| OP_G norm { $$ = make_op(W_OP_G, $1, 0, 0, $2); }
	| OP_BN norm { $$ = make_op(W_OP_BN, $1, 0, 0, $2); }
	;

norm:
	normval { $$ = $1; }
	| '[' normval ']' { $2->is_addr = 1; $$ = $2; }
	;

normval:
	REGISTER { $$ = make_norm($1, 0, NULL); }
	| val { $$ = make_norm(0, 0, $1); }
	| REGISTER '+' REGISTER { $$ = make_norm($1, $3, NULL); }
	| REGISTER '+' val { $$ = make_norm(0, $1, $3); }
	| val '+' REGISTER { $$ = make_norm(0, $3, $1); }
	;

val:
	VALUE { $$ = make_vval($1, NULL); }
	| NAME { $$ = make_vval(0, $1); }
	;

%%

void yyerror(char *s) {
	fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
	FILE *asm_source = fopen("test.asm", "r");
	if (!asm_source) {
		printf("Cannot open input file\n");
		exit(1);
	}
	yyin = asm_source;
	do {
		yyparse();
	} while (!feof(yyin));
	fclose(asm_source);

	FILE *bin_out = fopen("test.bin", "w");
	int res = program_write(word_first, bin_out);
	fclose(bin_out);
	printf("Words written: %i\n", res);

}

// vim: tabstop=4
