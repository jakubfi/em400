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

void c_yyerror(char *s, ...);
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

%token YERR;

%left '+' '-'
%left '/'
%nonassoc UMINUS
%token '.' '=' ':' ',' '(' ')' '&'

%token <val> VALUE
%token <val.s> FLOAT STRING
%token <val.s> LABEL IDENTIFIER VAR CMT NL
%type <val.v> reg
%type <norm> normarg norm1 norm2
%type <node> instruction expr
%type <node> zero value pragma condition prog finprog
%type <nl> vardef label code segment macro codeblock pre preblock cond maxlist

%%

program:
	preblock prog codeblock finprog NL {
		program = nl_append(program, $1);
		program = nl_append_n(program, $2);
		program = nl_append(program, $3);
		program = nl_append_n(program, $4);
		free($5);
	}
	;

prog:
	P_PROG			{ $$ = mknod_valstr(N_PROG, 0, NULL); }
	;

finprog:
	P_FINPROG		{ $$ = mknod_valstr(N_FINPROG, 0, NULL); }
	;

preblock:
	preblock pre	{ $$ = nl_append($1, $2); }
	|				{ $$ = NULL; }
	;

pre:
	CMT				{ $$ = make_nl(mknod_valstr(N_COMMENT, 0, $1)); }
	| NL			{ $$ = make_nl(mknod_valstr(N_NL, 0, $1)); }
	| pragma		{ $$ = make_nl($1); }
	;

codeblock:
	codeblock code 			{ $$ = nl_append($1, $2); if (!$2) { yyerror("Fatal. Sorry."); YYABORT;} }
	|						{ $$ = NULL; }
	;

code:
	vardef			{ $$ = $1; }
	| label			{ $$ = $1; }
	| pragma		{ $$ = make_nl($1); }
	| cond			{ $$ = $1; }
	| STRING		{ $$ = make_nl(mknod_valstr(N_STRING, 0, $1)); }
	| instruction	{ $$ = make_nl($1); }
	| CMT			{ $$ = make_nl(mknod_valstr(N_COMMENT, 0, $1)); }
	| expr '.'		{ $$ = make_nl($1); }
	| '.'			{ $$ = make_nl(mknod_valstr(N_VAL, 0, NULL)); }
	| macro			{ $$ = $1; }
	| segment		{ $$ = $1; }
	| NL			{ $$ = make_nl(mknod_valstr(N_NL, 0, $1)); }
	;

segment:
	P_SEG codeblock P_FINSEG {
		$$ = nl_append_n(NULL, mknod_valstr(N_SEG, 0, NULL));
		$$ = nl_append($$, $2);
		$$ = nl_append_n($$, mknod_valstr(N_FINSEG, 0, NULL));
	}
	;

macro:
	P_MACRO codeblock P_FINMACRO {
		$$ = nl_append_n(NULL, mknod_valstr(N_MACRO, 0, NULL));
		$$ = nl_append($$, $2);
		$$ = nl_append_n($$, mknod_valstr(N_FINMACRO, 0, NULL));
	}
	;

instruction:
	OP_2ARG ',' reg normarg			{ $$ = mknod_op(N_2ARG,  $1, $3, NULL, $4); }
	| OP_FD normarg					{ $$ = mknod_op(N_FD,    $1, 0,  NULL, $2); }
	| OP_KA1 ',' reg ',' expr '.'	{ $$ = mknod_op(N_KA1,   $1, $3, $5,   NULL); }
	| OP_JS ',' expr '.'			{ $$ = mknod_op(N_JS,    $1, 0,  $3,   NULL); }
	| OP_KA2 ',' expr '.'			{ $$ = mknod_op(N_KA2,   $1, 0,  $3,   NULL); }
	| OP_BRC ',' expr '.'			{ $$ = mknod_op(N_BRC,   $1, 0,  $3,   NULL); }
	| OP_BLC ',' expr '.'			{ $$ = mknod_op(N_BLC,   $1, 0,  $3,   NULL); }
	| OP_EXL ',' expr '.'			{ $$ = mknod_op(N_EXL,   $1, 0,  $3,   NULL); }
	| OP_C ',' reg '.'				{ $$ = mknod_op(N_C,     $1, $3, NULL, NULL); }
	| OP_SHC ',' reg ',' expr '.'	{ $$ = mknod_op(N_SHC,   $1, $3, $5,   NULL); }
	| OP_S ',' zero '.'				{ $$ = mknod_op(N_S,     $1, 0,  NULL, NULL); nodes_drop($3); }
	| OP_HLT ',' expr '.'			{ $$ = mknod_op(N_HLT,   $1, 0,  $3,   NULL); }
	| OP_J normarg					{ $$ = mknod_op(N_J,     $1, 0,  NULL, $2); }
	| OP_L normarg					{ $$ = mknod_op(N_L,     $1, 0,  NULL, $2); }
	| OP_G normarg					{ $$ = mknod_op(N_G,     $1, 0,  NULL, $2); }
	| OP_BN normarg					{ $$ = mknod_op(N_BN,    $1, 0,  NULL, $2); }
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
	expr			{ $$ = make_norm(0, 0, $1); }
	| expr '&' reg	{ $$ = make_norm(0, $3, $1); }
	;

expr:
	value					{ $$ = $1; }
	| expr '+' expr			{ $$ = mknod_nargs(N_PLUS, $1, $3); }
	| expr '-' expr			{ $$ = mknod_nargs(N_MINUS, $1, $3); }
	| value '/' value		{ $$ = mknod_nargs(N_SCALE, $1, $3); }
	| '-' expr %prec UMINUS { $$ = mknod_nargs(N_UMINUS, $2, NULL); }
	;

zero:
	VALUE {
		if ($1.v != 0) {
			c_yyerror("Invalid argument, should be '0'");
			YYABORT;
		}
		$$ = mknod_valstr(N_VAL, $1.v, $1.s);
	}
	;

value:
	VALUE			{ $$ = mknod_valstr(N_VAL, $1.v, $1.s); }
	| IDENTIFIER	{ $$ = mknod_valstr(N_NAME, 0, $1); }
	;

reg:
	VALUE			{ $$ = $1.v; free($1.s); }
	;

vardef:
	VAR '=' expr '.'			{ $$ = make_nl(mknod_dentry(N_VAR, $1, $3)); }
	| P_ALL VAR '=' expr '.'	{ $$ = make_nl(mknod_dentry(N_AVAR, $2, $4)); }
	;

label:
	LABEL ':'					{ $$ = make_nl(mknod_dentry(N_LABEL, $1, NULL)); }
	| P_ALL LABEL ':'			{ $$ = make_nl(mknod_dentry(N_ALABEL, $2, NULL)); }
	;

pragma:
	P_S expr '.'				{ $$ = mknod_nargs(N_SETIC, $2, NULL); }
	| P_RES expr '.'			{ $$ = mknod_nargs(N_RES, $2, NULL); }
	| P_RES expr ',' expr '.'	{ $$ = mknod_nargs(N_RES, $2, $4); }
	| P_F '.'					{ $$ = mknod_valstr(N_COMMENT, 0, NULL); } // unhandled
	| P_NAME IDENTIFIER '.'		{ $$ = mknod_valstr(N_COMMENT, 0, $2); } // unhandled
	| P_BA expr '.'				{ $$ = mknod_nargs(N_COMMENT, $2, NULL); } // unhandled
	| P_INT expr '.'			{ $$ = mknod_nargs(N_COMMENT, $2, NULL); } // unhandled
	| P_OUT expr '.'			{ $$ = mknod_nargs(N_COMMENT, $2, NULL); } // unhandled
	| P_LAB expr '.'			{ $$ = mknod_nargs(N_COMMENT, $2, NULL); } // unhandled
	| P_NLAB					{ $$ = mknod_valstr(N_COMMENT, 0, NULL); } // unhandled
	| P_MEM expr '.'			{ $$ = mknod_nargs(N_COMMENT, $2, NULL); } // unhandled
	| P_OS						{ $$ = mknod_valstr(N_COMMENT, 0, NULL); } // unhandled
	| P_SS expr '.'				{ $$ = mknod_nargs(N_OVL, $2, NULL); }
	| P_HS						{ $$ = mknod_valstr(N_COMMENT, 0, NULL); } // unhandled
	| P_MAX IDENTIFIER ',' maxlist '.' { $$ = mknod_valstr(N_COMMENT, 0, $2); } // unhandled
	| P_LEN expr '.'			{ $$ = mknod_nargs(N_LEN, $2, NULL); } // unhandled
	| P_E expr '.'				{ $$ = mknod_nargs(N_COMMENT, $2, NULL); } // unhandled
	| P_FILE IDENTIFIER ',' VALUE ',' expr ',' expr '.' { $$ = mknod_file($2, $4.s, $6, $8); } // unhandled
	| P_TEXT expr '.'			{ $$ = mknod_nargs(N_TEXT, $2, NULL); }
	;

cond:
	condition '.' codeblock P_FI {
		$$ = nl_append_n(NULL, $1);
		$$ = nl_append($$, $3);
		$$ = nl_append_n($$, mknod_valstr(N_FI, 0, NULL));
	}
	;

condition:
	P_IFUNK IDENTIFIER		{ $$ = mknod_valstr(N_IFUNK, 0, $2); }
/*	| P_IFUND IDENTIFIER	{ $$ = mknod_valstr(N_IFUND, 0, $2); } */
	| P_IFDEF IDENTIFIER	{ $$ = mknod_valstr(N_IFDEF, 0, $2); }
	;

maxlist:
	expr { $$ = NULL; }
	| maxlist ',' expr { $$ = NULL; }
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
}

// vim: tabstop=4
