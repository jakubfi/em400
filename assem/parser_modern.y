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
	struct val_t {
		int v;
		char *s;
	} val;
	struct norm_t *norm;
	struct node_t *node;
};

%token '[' ']' ',' ':'
%token MP_DATA MP_EQU MP_RES MP_PROG MP_FINPROG MP_SEG MP_FINSEG MP_MACRO MP_FINMACRO
%token <val.s> NAME STRING COMMENT
%token <val> VALUE ADDR
%token <val.v> REGISTER
%token <val.v> MOP_2ARG MOP_FD MOP_KA1 MOP_JS MOP_KA2 MOP_C MOP_SHC MOP_S MOP_HLT MOP_J MOP_L MOP_G MOP_BN

%type <norm> normval norm
%type <node> instruction res data dataword words expr comment

%left '+' '-'
%left SHR SHL
%nonassoc UMINUS


%%

program:
	comments MP_PROG STRING sentences MP_FINPROG comments	{ printf("Assembling program '%s'\n", $3); free($3); }
	| comments MP_PROG sentences MP_FINPROG	comments		{ printf("Assembling unnamed program\n"); }
	;

sentences:
	sentences sentence
	| 
	;

sentence:
	words {
		if (program_append($1) < 0) {
			m_yyerror("cannot append word (program too big?)");
			YYABORT;
		}
	}
	| MP_EQU NAME expr {
		if (!dict_add(dict, D_VALUE, $2, $3)) {
			m_yyerror("name '%s' already defined", $2);
			YYABORT;
		}
	}
	| NAME ':' {
		struct node_t *n = make_value(ic, NULL);
		if (!dict_add(dict, D_ADDR, $1, n)) {
			m_yyerror("name '%s' already defined", $1);
			YYABORT;
		}
	}
	| comment { program_append($1); }
	;

comments:
	comment comments { program_append($1); }
	|
	;

comment:
	COMMENT { $$ = make_comment($1); }
	;

words:
	instruction		{ $$ = $1; }
	| MP_DATA data	{ $$ = $2; }
	| res			{ $$ = $1; }
	;

data:
	dataword			{ $$ = $1; }
	| dataword ',' data	{ $1->next = $3; $$ = $1; }
	;

dataword:
	expr		{ $$ = $1; }
	| STRING	{ $$ = make_string($1); }
	;

res:
	MP_RES VALUE				{ $$ = make_rep($2.v, 0, NULL); }
	| MP_RES VALUE ',' VALUE	{ $$ = make_rep($2.v, $4.v, $4.s); }
	;

instruction:
	MOP_2ARG REGISTER ',' norm	{ $$ = make_op(N_2ARG,	$1, $2, NULL, $4); }
	| MOP_FD norm				{ $$ = make_op(N_FD,	$1, 0,  NULL, $2); }
	| MOP_KA1 REGISTER ',' expr	{ $$ = make_op(N_KA1,	$1, $2, $4,   NULL); }
	| MOP_JS expr				{ $$ = make_op(N_JS,	$1, 0,  $2,   NULL); }
	| MOP_KA2 expr				{ $$ = make_op(N_KA2,	$1, 0,  $2,   NULL); }
	| MOP_C REGISTER			{ $$ = make_op(N_C,		$1, $2, NULL, NULL); }
	| MOP_SHC REGISTER ',' expr	{ $$ = make_op(N_SHC,	$1, $2, $4,   NULL); }
	| MOP_S						{ $$ = make_op(N_S,		$1, 0,  NULL, NULL); }
	| MOP_HLT expr				{ $$ = make_op(N_HLT,	$1, 0,  $2,   NULL); }
	| MOP_J norm				{ $$ = make_op(N_J,		$1, 0,  NULL, $2); }
	| MOP_L norm				{ $$ = make_op(N_L,		$1, 0,  NULL, $2); }
	| MOP_G norm				{ $$ = make_op(N_G,		$1, 0,  NULL, $2); }
	| MOP_BN norm				{ $$ = make_op(N_BN,	$1, 0,  NULL, $2); }
	;

norm:
	normval					{ $$ = $1; }
	| '[' normval ']'		{ $$ = $2; $$->d = 1; }
	;

normval:
	REGISTER				{ $$ = make_norm($1, 0, NULL); }
	| expr					{ $$ = make_norm(0, 0, $1); }
	| REGISTER '+' REGISTER	{ $$ = make_norm($1, $3, NULL); }
	| REGISTER '+' expr		{ $$ = make_norm(0, $1, $3); }
	| REGISTER '-' expr		{ $$ = make_norm(0, $1, make_oper(N_UMINUS, $3, NULL)); }
	| expr '+' REGISTER		{ $$ = make_norm(0, $3, $1); }
	;

expr:
	VALUE					{ $$ = make_value($1.v, $1.s); }
	| NAME					{ $$ = make_name($1); }
	| expr '+' expr			{ $$ = make_oper(N_PLUS, $1, $3); }
	| expr '-' expr			{ $$ = make_oper(N_MINUS, $1, $3); }
	| '-' expr %prec UMINUS	{ $$ = make_oper(N_UMINUS, $2, NULL); }
	| '(' expr ')'			{ $$ = $2; }
	| expr SHL expr			{ $$ = make_oper(N_SHL, $1, $3); }
	| expr SHR expr			{ $$ = make_oper(N_SHR, $1, $3); }
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
