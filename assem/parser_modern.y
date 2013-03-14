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
#include <stdarg.h>

#include "ops.h"
#include "elements.h"
#include "eval.h"

void m_yyerror(char *s, ...);
int yylex(void);
int got_error;
extern int ic;

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
%token <str> NAME STRING
%token <val> VALUE ADDR REGISTER
%token <val> OP_2ARG OP_FD OP_KA1 OP_JS OP_KA2 OP_C OP_SHC OP_S OP_HLT OP_J OP_L OP_G OP_BN

%type <norm> normval norm
%type <word> words
%type <word> instruction res data dataword
%type <enode> expr

%left '+' '-'
%left SHR SHL
%nonassoc UMINUS


%%

program:
	PROGRAM STRING sentences ENDPROG {
		printf("Assembling program '%s'\n", $2);
	}
	| sentences {
		printf("Assembling unnamed program\n");
	}
	;

sentences:
	sentences sentence
	| 
	;

sentence:
	words {
		if (program_append($1) < 0) {
			m_yyerror("program too big");
			YYABORT;
		}
	}
	| EQU NAME expr {
		if (!dict_add(VALUE, $2, $3)) {
			m_yyerror("name '%s' already defined", $2);
			YYABORT;
		}
	}
	| NAME ':' {
		struct enode_t *e = make_enode(VALUE, ic, NULL, NULL, NULL);
		if (!e) {
			m_yyerror("cannot make enode for '%s'", $1);
			YYABORT;
		}
		if (!dict_add(ADDR, $1, e)) {
			m_yyerror("name '%s' already defined", $1);
			YYABORT;
		}
	}
	;

words:
	instruction { $$ = $1; }
	| DATA data { $$ = $2; }
	| res { $$ = $1; }
	;

data:
	dataword { $$ = $1; }
	| dataword ',' data { $1->next = $3; $$ = $1; }
	;

dataword:
	expr { $$ = make_data($1, m_yylloc.first_line); }
	| STRING { $$ = make_string($1, m_yylloc.first_line); }
	;

res:
	RES VALUE {
		struct word_t *wlist = make_rep($2, 0, m_yylloc.first_line);
		if (!wlist) {
			m_yyerror("resulting .res length is 0");
			YYABORT;
		} else {
			$$ = wlist;
		}
	}
	| RES VALUE ',' VALUE{
		struct word_t *wlist = make_rep($2, $4, m_yylloc.first_line);
		if (!wlist) {
			m_yyerror("resulting .res length is 0");
			YYABORT;
		} else {
			$$ = wlist;
		}
	}
	;

instruction:
	OP_2ARG REGISTER ',' norm { $$ = make_op(OP_2ARG, $1, $2, NULL, $4, m_yylloc.first_line); }
	| OP_FD norm { $$ = make_op(OP_FD, $1, 0, NULL, $2, m_yylloc.first_line); }
	| OP_KA1 REGISTER ',' expr { $$ = make_op(OP_KA1, $1, $2, $4, NULL, m_yylloc.first_line); }
	| OP_JS expr { $$ = make_op(OP_JS, $1, 0, $2, NULL, m_yylloc.first_line); }
	| OP_KA2 expr { $$ = make_op(OP_KA2, $1, 0, $2, NULL, m_yylloc.first_line); }
	| OP_C REGISTER { $$ = make_op(OP_C, $1, $2, NULL, NULL, m_yylloc.first_line); }
	| OP_SHC REGISTER ',' expr { $$ = make_op(OP_SHC, $1, $2, $4, NULL, m_yylloc.first_line); }
	| OP_S { $$ = make_op(OP_S, $1, 0, NULL, NULL, m_yylloc.first_line); }
	| OP_HLT expr { $$ = make_op(OP_HLT, $1, 0, $2, NULL, m_yylloc.first_line); }
	| OP_J norm { $$ = make_op(OP_J, $1, 0, NULL, $2, m_yylloc.first_line); }
	| OP_L norm { $$ = make_op(OP_L, $1, 0, NULL, $2, m_yylloc.first_line); }
	| OP_G norm { $$ = make_op(OP_G, $1, 0, NULL, $2, m_yylloc.first_line); }
	| OP_BN norm { $$ = make_op(OP_BN, $1, 0, NULL, $2, m_yylloc.first_line); }
	;

norm:
	normval { $$ = $1; }
	| '[' normval ']' { $2->is_addr = 1; $$ = $2; }
	;

normval:
	REGISTER { $$ = make_norm($1, 0, NULL); }
	| expr { $$ = make_norm(0, 0, make_data($1, m_yylloc.first_line)); }
	| REGISTER '+' REGISTER { $$ = make_norm($1, $3, NULL); }
	| REGISTER '+' expr { $$ = make_norm(0, $1, make_data($3, m_yylloc.first_line)); }
	| REGISTER '-' expr { $$ = make_norm(0, $1, make_data(make_enode(UMINUS, 0, NULL, NULL, $3), m_yylloc.first_line)); }
	| expr '+' REGISTER { $$ = make_norm(0, $3, make_data($1, m_yylloc.first_line)); }
	;

expr:
	VALUE { $$ = make_enode(VALUE, $1, NULL, NULL, NULL); }
	| NAME { $$ = make_enode(NAME, 0, $1, NULL, NULL); }
	| expr '+' expr { $$ = make_enode('+', 0, NULL, $1, $3); }
	| expr '-' expr { $$ = make_enode('-', 0, NULL, $1, $3); }
	| '-' expr %prec UMINUS { $$ = make_enode(UMINUS, 0, NULL, NULL, $2); }
	| '(' expr ')' { $$ = $2; }
	| expr SHL expr { $$ = make_enode(SHL, 0, NULL, $1, $3); }
	| expr SHR expr { $$ = make_enode(SHR, 0, NULL, $1, $3); }
	;

%%

// -----------------------------------------------------------------------
void m_yyerror(char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	printf("Error parsing source, line %d: ", m_yylloc.first_line);
	vprintf(s, ap);
	printf("\n");
	va_end(ap);
	got_error = 1;
}

// vim: tabstop=4
