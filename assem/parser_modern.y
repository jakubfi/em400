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
#include <string.h>
#include <strings.h>

#include "parsers.h"
#include "keywords.h"
#include "dict.h"
#include "elements.h"

void m_yyerror(char *s, ...);
int yylex(void);

%}

%error-verbose
%locations

%union {
	struct val_t {
		int v;
		char *s;
	} val;
	struct norm_t *norm;
	struct node_t *node;
	struct nodelist_t *nl;
};

%token <val.v> OP_2ARG OP_FD OP_KA1 OP_JS OP_KA2 OP_BRC OP_BLC OP_EXL OP_C OP_SHC OP_S OP_HLT OP_J OP_L OP_G OP_BN
%token P_PROG P_FINPROG P_SEG P_FINSEG P_MACRO P_FINMACRO
%token P_DATA P_EQU P_RES
%token P_S P_F P_ALL P_NAME P_BA P_INT P_OUT P_LAB P_NLAB P_MEM P_OS P_IFUNK P_IFUND P_IFDEF P_FI P_SS P_HS P_MAX P_LEN P_E P_FILE P_TEXT

%token '[' ']' ',' ':' '*'
%token <val.s> STRING NAME CMT_CODE CMT_LINE
%token <val> VALUE
%token <val.v> REGISTER

%type <norm> normval norm
%type <node> instruction expr comment label prog finprog
%type <nl> data dataword comments sentence sentences pragma

%left '+' '-'
%left SHR SHL
%nonassoc UMINUS


%%

program:
	comments prog sentences finprog comments {
		if ($2->str) {
			printf("Assembling program: '%s'\n", $2->str);
		} else {
			printf("Assembling unnamed program\n");
		}
		program = nl_append(program, $1);
		program = nl_append_n(program, $2);
		program = nl_append(program, $3);
		program = nl_append_n(program, $4);
		program = nl_append(program, $5);
	}
	;

prog:
	P_PROG STRING		{ $$ = mknod_valstr(N_PROG, 0, $2); }
	| P_PROG			{ $$ = mknod_valstr(N_PROG, 0, NULL); }
	;

finprog:
	P_FINPROG			{ $$ = mknod_valstr(N_FINPROG, 0, NULL); }
	;

comments:
	comments comment	{ $$ = nl_append_n($1, $2); }
	|					{ $$ = NULL; }
	;

sentences:
	sentences sentence { $$ = nl_append($1, $2); }
	| { $$ = NULL; }
	;

sentence:
	instruction		{ $$ = make_nl($1); }
	| pragma		{ $$ = $1; }
	| label			{ $$ = make_nl($1); }
	| comment		{ $$ = make_nl($1); }
	;

label:
	NAME ':'		{ $$ = mknod_dentry(N_LABEL, $1, NULL); }
	| '*' NAME ':'	{ $$ = mknod_dentry(N_ALABEL, $2, NULL); }
	;

comment:
	CMT_LINE		{ $$ = mknod_valstr(N_COMMENT, 0, $1); }
	;

data:
	dataword			{ $$ = $1; }
	| dataword ',' data	{ $$ = nl_append($1, $3); }
	;

dataword:
	expr		{ $$ = make_nl($1); }
	| STRING	{ $$ = make_nl(mknod_valstr(N_STRING, 0, $1)); }
	;

pragma:
	P_RES expr				{ $$ = make_nl(mknod_nargs(N_RES, $2, NULL)); }
	| P_RES expr ',' expr	{ $$ = make_nl(mknod_nargs(N_RES, $2, $4)); }
	| P_DATA data			{ $$ = $2; }
	| P_EQU NAME expr		{ $$ = make_nl(mknod_dentry(N_VAR, $2, $3)); }
	| P_EQU '*' NAME expr	{ $$ = make_nl(mknod_dentry(N_AVAR, $3, $4)); }
	| P_MACRO sentences P_FINMACRO {
		$$ = NULL;
		$$ = nl_append_n($$, mknod_valstr(N_MACRO, 0, NULL));
		$$ = nl_append($$, $2);
		$$ = nl_append_n($$, mknod_valstr(N_FINMACRO, 0, NULL));
	}
	| P_SEG sentences P_FINSEG {
		$$ = NULL;
		$$ = nl_append_n($$, mknod_valstr(N_SEG, 0, NULL));
		$$ = nl_append($$, $2);
		$$ = nl_append_n($$, mknod_valstr(N_FINSEG, 0, NULL));
	}
	;

instruction:
	OP_2ARG REGISTER ',' norm	{ $$ = mknod_op(N_2ARG,	$1, $2, NULL, $4); }
	| OP_FD norm				{ $$ = mknod_op(N_FD,	$1, 0,  NULL, $2); }
	| OP_KA1 REGISTER ',' expr	{ $$ = mknod_op(N_KA1,	$1, $2, $4,   NULL); }
	| OP_JS expr				{ $$ = mknod_op(N_JS,	$1, 0,  $2,   NULL); }
	| OP_KA2 expr				{ $$ = mknod_op(N_KA2,	$1, 0,  $2,   NULL); }
	| OP_BRC expr				{ $$ = mknod_op(N_BRC,	$1, 0,  $2,   NULL); }
	| OP_BLC expr				{ $$ = mknod_op(N_BLC,	$1, 0,  $2,   NULL); }
	| OP_EXL expr				{ $$ = mknod_op(N_EXL,	$1, 0,  $2,   NULL); }
	| OP_C REGISTER				{ $$ = mknod_op(N_C,		$1, $2, NULL, NULL); }
	| OP_SHC REGISTER ',' expr	{ $$ = mknod_op(N_SHC,	$1, $2, $4,   NULL); }
	| OP_S						{ $$ = mknod_op(N_S,		$1, 0,  NULL, NULL); }
	| OP_HLT expr				{ $$ = mknod_op(N_HLT,	$1, 0,  $2,   NULL); }
	| OP_J norm					{ $$ = mknod_op(N_J,		$1, 0,  NULL, $2); }
	| OP_L norm					{ $$ = mknod_op(N_L,		$1, 0,  NULL, $2); }
	| OP_G norm					{ $$ = mknod_op(N_G,		$1, 0,  NULL, $2); }
	| OP_BN norm				{ $$ = mknod_op(N_BN,	$1, 0,  NULL, $2); }
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
	| REGISTER '-' expr		{ $$ = make_norm(0, $1, mknod_nargs(N_UMINUS, $3, NULL)); }
	| expr '+' REGISTER		{ $$ = make_norm(0, $3, $1); }
	;

expr:
	VALUE					{ $$ = mknod_valstr(N_VAL, $1.v, $1.s); }
	| NAME					{ $$ = mknod_valstr(N_NAME, 0, $1); }
	| expr '+' expr			{ $$ = mknod_nargs(N_PLUS, $1, $3); }
	| expr '-' expr			{ $$ = mknod_nargs(N_MINUS, $1, $3); }
	| '-' expr %prec UMINUS	{ $$ = mknod_nargs(N_UMINUS, $2, NULL); }
	| '(' expr ')'			{ $$ = $2; }
	| expr SHL expr			{ $$ = mknod_nargs(N_SHL, $1, $3); }
	| expr SHR expr			{ $$ = mknod_nargs(N_SHR, $1, $3); }
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
