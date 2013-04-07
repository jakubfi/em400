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

#include "ops.h"
#include "elements.h"
#include "eval.h"

void c_yyerror(char *s, ...);
int yylex(void);
extern int got_error;

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

%left '+' '-'
%left '/'
%nonassoc UMINUS
%token '.' '=' ':' ',' '(' ')' '&'
%token CP_S CP_RES CP_F CP_PROG CP_FINPROG CP_SEG CP_FINSEG CP_MACRO CP_FINMACRO CP_ALL CP_NAME CP_BA CP_INT CP_OUT CP_LAB CP_NLAB CP_MEM CP_OS CP_IFUNK CP_IFUND CP_IFDEF CP_FI CP_SS CP_HS CP_MAX CP_LEN CP_E CP_FILE CP_TEXT
%token <val.s> IDENTIFIER FLOAT STRING
%token <val.s> LABEL VAR COMMENT
%token <val> VALUE
%token <val.v> COP_2ARG COP_FD COP_KA1 COP_JS COP_KA2 COP_C COP_SHC COP_S COP_HLT COP_J COP_L COP_G COP_BN
%type <norm> normarg norm1 norm2
%type <node> instruction expr
%type <val.v> reg
%type <node> zero value
%type <nl> vardef label code segment macro codeblock pre preblock

%%

program:
	preblock CP_PROG '*' { printf("NEW VARSET: PROG\n"); } codeblock CP_FINPROG '*'	{ program = $1; }
	;

segment:
	CP_SEG '*' { printf("NEW VARSET: SEG\n"); } codeblock CP_FINSEG '*'				{ $$ = $4; }
	;

macro:
	CP_MACRO '*' { printf("NEW VARSET: MACRO\n"); } codeblock CP_FINMACRO '*'		{ $$ = $4; }
	;

preblock:
	preblock pre	{ $$ = nl_append($1, $2); }
	|				{ $$ = NULL; }
	;

pre:
	COMMENT			{ $$ = make_nl(make_comment($1)); }
	| pragma		{ $$ = NULL; }
	;

codeblock:
	codeblock code	{ $$ = nl_append($1, $2); }
	|				{ $$ = NULL; }
	;

code:
	vardef			{ $$ = $1; }
	| label			{ printf("(label)\n"); $$ = $1; }
	| pragma		{ $$ = NULL; }
	| STRING		{ $$ = make_string($1); program_ic += strlen($1); }
	| instruction	{ printf("(instr)\n"); $$ = make_nl($1); }
	| COMMENT		{ $$ = make_nl(make_comment($1)); }
	| expr '.'		{ printf("(expr)\n"); $$ = make_nl($1); }
	| '.'			{ printf("(empty)\n"); }
	| macro
	| segment		{ $$ = $1; }
	;

instruction:
	COP_2ARG ',' reg normarg		{ $$ = make_op(N_2ARG,  $1, $3, NULL, $4); }
	| COP_FD normarg				{ $$ = make_op(N_FD,    $1, 0,  NULL, $2); }
	| COP_KA1 ',' reg ',' expr '.'	{ $$ = make_op(N_KA1,   $1, $3, $5,   NULL); }
	| COP_JS ',' expr '.'			{ $$ = make_op(N_JS,    $1, 0,  $3,   NULL); }
	| COP_KA2 ',' expr '.'			{ $$ = make_op(N_KA2,   $1, 0,  $3,   NULL); }
	| COP_C ',' reg '.'				{ $$ = make_op(N_C,     $1, $3, NULL, NULL); }
	| COP_SHC ',' reg ',' expr '.'	{ $$ = make_op(N_SHC,   $1, $3, $5,   NULL); }
	| COP_S ',' zero '.'			{ $$ = make_op(N_S,     $1, 0,  NULL, NULL); }
	| COP_HLT ',' expr '.'			{ $$ = make_op(N_HLT,   $1, 0,  $3,   NULL); }
	| COP_J normarg					{ $$ = make_op(N_J,     $1, 0,  NULL, $2); }
	| COP_L normarg					{ $$ = make_op(N_L,     $1, 0,  NULL, $2); }
	| COP_G normarg					{ $$ = make_op(N_G,     $1, 0,  NULL, $2); }
	| COP_BN normarg				{ $$ = make_op(N_BN,    $1, 0,  NULL, $2); }
	;

normarg:
	',' norm1 '.'			{ $$ = $2; }
	| ',' norm1 '\'' '.'	{ $$ = $2; $$->d = 1; }
	| '(' norm2 ')'			{ $$ = $2; }
	| '(' norm2 '\'' ')'	{ $$ = $2; $$->d = 1; }
	;

norm1:
	reg				{ $$ = make_norm($1, 0, NULL); }
	| reg '&' reg	{ $$ = make_norm($1, $3, NULL); }
	;

norm2:
	expr			{ $$ = make_norm(0, 0, $1); program_ic++; }
	| expr '&' reg	{ $$ = make_norm(0, $3, $1); program_ic++; }
	;

expr:
	value					{ $$ = $1; }
	| expr '+' expr			{ $$ = make_oper(N_PLUS, $1, $3); }
	| expr '-' expr			{ $$ = make_oper(N_MINUS, $1, $3); }
	| value '/' value		{ $$=NULL; }
	| '-' expr %prec UMINUS { $$ = make_oper(N_UMINUS, $2, NULL); }
	;

zero:
	VALUE			{ $$ = make_value($1.v, $1.s); }
	;

value:
	VALUE			{ $$ = make_value($1.v, $1.s); }
	| IDENTIFIER	{ $$ = make_name($1); }
	;

reg:
	VALUE			{ $$ = $1.v; }
/*	| IDENTIFIER { printf("mamto\n"); }*/
	;

vardef:
	VAR '=' expr '.' { $$ = make_nl(make_equ($1, $3)); }
	;

label:
	LABEL ':' { $$ = make_nl(make_label($1)); }
	;

pragma:
	CP_S '*' expr '.'
	| CP_RES '*' expr '.'
	| CP_RES '*' expr ',' expr '.'
	| CP_F '*' '.'
	| CP_NAME '*' IDENTIFIER '.'
	| CP_BA '*' expr '.'
	| CP_INT '*' expr '.'
	| CP_OUT '*' expr '.'
	| CP_LAB '*' expr '.'
	| CP_NLAB '*'
	| CP_MEM '*' expr '.'
	| CP_OS '*'
	| CP_IFUNK '*' IDENTIFIER '.'
	| CP_IFUND '*' IDENTIFIER '.'
	| CP_IFDEF '*' IDENTIFIER '.'
	| CP_FI '*'
	| CP_SS '*' expr '.'
	| CP_HS '*'
	| CP_MAX '*' IDENTIFIER ',' maxlist '.'
	| CP_LEN '*' expr '.'
	| CP_E '*' expr '.'
	| CP_ALL '*' vardef
	| CP_ALL '*' label
	| CP_FILE '*' IDENTIFIER ',' IDENTIFIER ',' expr ',' expr '.'
	| CP_TEXT '*' expr '.'
	;

maxlist:
	expr
	| maxlist ',' expr
	;

%%

// -----------------------------------------------------------------------
void c_yyerror(char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	printf("Error parsing source, line %d: ", c_yylloc.first_line);
	vprintf(s, ap);
	printf("\n");
	va_end(ap);
	got_error = 1;
}

// vim: tabstop=4
