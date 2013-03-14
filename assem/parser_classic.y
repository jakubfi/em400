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

void c_yyerror(char *s, ...);
int yylex(void);
extern int got_error;
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

%left  '+' '-' '/'
%token '=' ':' ',' '.' '(' ')' '&'
%token S RES F PROG FINPROG SEG FINSEG MACRO FINMACRO ALL NAME BA INT OUT LAB NLAB MEM OS IFUNK IFUND IFDEF FI SS HS MAX LEN E
%token <str> IDENTIFIER FLOAT STRING
%token <val> VALUE
%token <val> OP_2ARG OP_FD OP_KA1 OP_JS OP_KA2 OP_C OP_SHC OP_S OP_HLT OP_J OP_L OP_G OP_BN


%%

program:
	PROG progblock FINPROG
	;

progblock:
	segment progblock
	| macro progblock
	| code progblock
	|
	;

segment:
	SEG segmentblock FINSEG
	;

segmentblock:
	macro segmentblock
	| code segmentblock
	|
	;

macro:
	MACRO macroblock FINMACRO
	;

macroblock:
	code macroblock
	|
	;

code:
	vardef
	| label
	| internal
	| STRING
	| instruction
	;

instruction:
	OP_2ARG ',' reg normarg
	| OP_FD normarg
	| OP_KA1 ',' reg ',' expr '.'
	| OP_JS ',' expr '.'
	| OP_KA2 ',' expr '.'
	| OP_C ',' reg '.'
	| OP_SHC '.' reg ',' expr '.'
	| OP_S ',' zero '.'
	| OP_HLT ',' expr '.'
	| OP_J normarg
	| OP_L normarg
	| OP_G normarg
	| OP_BN normarg
	;

normarg:
	',' norm_oneword '.'
	',' norm_oneword '\'' '.'
	| '(' norm_twowords ')'
	| '(' norm_twowords '\'' ')'
	;

norm_oneword:
	reg
	| reg '&' reg
	;

norm_twowords:
	expr
	expr '&' reg
	;

expr:
	value
	| expr '+' expr
	| expr '-' expr
	| expr '/' value
	;

zero:
	VALUE
	;

value:
	VALUE
	| IDENTIFIER
	;

reg:
	VALUE
	| IDENTIFIER
	;

vardef:
	IDENTIFIER '=' expr '.'
	;

label:
	IDENTIFIER ':'
	;

internal:
	S expr '.'
	| RES expr '.'
	| RES expr ',' expr '.'
	| F expr '.' floats
	| NAME IDENTIFIER '.'
	| BA expr '.'
	| INT expr '.'
	| OUT expr '.'
	| LAB expr '.'
	| NLAB
	| MEM expr '.'
	| OS
	| IFUNK IDENTIFIER '.'
	| IFUND IDENTIFIER '.'
	| IFDEF IDENTIFIER '.'
	| FI
	| SS expr '.'
	| HS
	| MAX IDENTIFIER ',' maxlist '.'
	| LEN expr '.'
	| E expr '.'
	;

maxlist:
	expr
	| maxlist ',' expr
	;

floats:
	float
	| float ',' floats
	| float ';' floats
	| float floats
	;

float:
	VALUE
	| FLOAT
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
