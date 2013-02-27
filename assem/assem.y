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

void yyerror(char *s, ...);
int yylex(void);
extern FILE *yyin;
int got_error = 0;
extern char assembly_error[];
int ic = 0;

%}

%locations
%union {
	char *str;
	int val;
	struct word_t *word;
	struct norm_t *norm;
	struct enode_t *enode;
};

%token '[' ']' ',' ':'
%token DATA EQU RES PROGRAM ENDPROG
%token <str> NAME
%token <val> VALUE REGISTER
%token <val> OP_2ARG OP_FD OP_KA1 OP_JS OP_KA2 OP_C OP_SHC OP_S OP_J OP_L OP_G OP_BN

%type <norm> normval norm
%type <word> data res instruction
%type <enode> expr

%left '+' '-'

%%

program:
	PROGRAM NAME sentences ENDPROG
	| sentences
	;

sentences:
	sentences sentence
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
	| EQU NAME VALUE { dict_add(D_VALUE, $2, $3); }
	| res {
		struct enode_t *e = enode_eval($1->data_rep);
		if (!e) {
			yyerror("cannot evaluate .res length");
		} else {
			if (e->value <= 0) {
				yyerror("resulting .res length is 0");
			} else {
				word_add($1);
				ic += e->value;
			}
		}
	}
	| NAME ':' { dict_add(D_ADDR, $1, ic); }
	;

data:
	DATA expr { $$ = make_data($2, NULL, yylloc.first_line); }
	;

res:
	RES expr ',' expr { $$ = make_data($4, $2, yylloc.first_line); }
	;

instruction:
	OP_2ARG REGISTER ',' norm { $$ = make_op(W_OP_2ARG, $1, $2, NULL, $4, yylloc.first_line); }
	| OP_FD norm { $$ = make_op(W_OP_FD, $1, 0, NULL, $2, yylloc.first_line); }
	| OP_KA1 REGISTER ',' expr { $$ = make_op(W_OP_KA1, $1, $2, $4, NULL, yylloc.first_line); }
	| OP_JS expr { $$ = make_op(W_OP_JS, $1, 0, $2, NULL, yylloc.first_line); }
	| OP_KA2 expr { $$ = make_op(W_OP_KA2, $1, 0, $2, NULL, yylloc.first_line); }
	| OP_C REGISTER { $$ = make_op(W_OP_C, $1, $2, NULL, NULL, yylloc.first_line); }
	| OP_SHC REGISTER ',' expr { $$ = make_op(W_OP_SHC, $1, $2, $4, NULL, yylloc.first_line); }
	| OP_S { $$ = make_op(W_OP_S, $1, 0, NULL, NULL, yylloc.first_line); }
	| OP_J norm { $$ = make_op(W_OP_J, $1, 0, NULL, $2, yylloc.first_line); }
	| OP_L norm { $$ = make_op(W_OP_L, $1, 0, NULL, $2, yylloc.first_line); }
	| OP_G norm { $$ = make_op(W_OP_G, $1, 0, NULL, $2, yylloc.first_line); }
	| OP_BN norm { $$ = make_op(W_OP_BN, $1, 0, NULL, $2, yylloc.first_line); }
	;

norm:
	normval { $$ = $1; }
	| '[' normval ']' { $2->is_addr = 1; $$ = $2; }
	;

normval:
	REGISTER { $$ = make_norm($1, 0, NULL); }
	| expr { $$ = make_norm(0, 0, $1); }
	| REGISTER '+' REGISTER { $$ = make_norm($1, $3, NULL); }
	| REGISTER '+' expr { $$ = make_norm(0, $1, $3); }
	| expr '+' REGISTER { $$ = make_norm(0, $3, $1); }
	;

expr:
	VALUE { $$ = make_enode(VALUE, $1, NULL, NULL, NULL); }
	| NAME { $$ = make_enode(NAME, 0, $1, NULL, NULL); }
	| expr '+' expr { $$ = make_enode('+', 0, NULL, $1, $3); }
	| expr '-' expr { $$ = make_enode('-', 0, NULL, $1, $3); }
	;

%%

// -----------------------------------------------------------------------
void yyerror(char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	printf("Error parsing source, line %d: ", yylloc.first_line);
	vprintf(s, ap);
	printf("\n");
	got_error = 1;
}

// -----------------------------------------------------------------------
int main(void) {
	FILE *asm_source = fopen("test.asm", "r");
	if (!asm_source) {
		printf("Cannot open input file\n");
		exit(1);
	}

	yyin = asm_source;
	int res;
	do {
		res = yyparse();
	} while (!feof(yyin));
	fclose(asm_source);

	if (got_error) {
		printf("Exiting.\n");
		exit(1);
	}

	uint16_t outdata[64*1024];
	struct word_t *word = program_start;
	int ic = 0;
	while (word) {
		res = make_bin(ic, word, outdata);
		if (res < 0) {
			printf("Error assembling binary image, line %i: %s\nExiting.\n", word->lineno-1, assembly_error);
			exit(1);
		}
		ic += res;
		word = word->next;
	}

	FILE *bin_out = fopen("test.bin", "w");
	res = fwrite(outdata, 2, ic, bin_out);
	fclose(bin_out);
	printf("Program size: %i, words written: %i\n", ic, res);

}

// vim: tabstop=4
