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
%token CP_S CP_RES CP_F CP_PROG CP_FINPROG CP_SEG CP_FINSEG CP_MACRO CP_FINMACRO CP_ALL CP_NAME CP_BA CP_INT CP_OUT CP_LAB CP_NLAB CP_MEM CP_OS CP_IFUNK CP_IFUND CP_IFDEF CP_FI CP_SS CP_HS CP_MAX CP_LEN CP_E
%token <str> IDENTIFIER FLOAT STRING
%token <val> VALUE
%token <val> COP_2ARG COP_FD COP_KA1 COP_JS COP_KA2 COP_C COP_SHC COP_S COP_HLT COP_J COP_L COP_G COP_BN


%%

program:
	CP_PROG progblock CP_FINPROG
	;

progblock:
	segment progblock
	| macro progblock
	| code progblock
	|
	;

segment:
	CP_SEG segmentblock CP_FINSEG
	;

segmentblock:
	macro segmentblock
	| code segmentblock
	|
	;

macro:
	CP_MACRO macroblock CP_FINMACRO
	;

macroblock:
	code macroblock
	|
	;

code:
	vardef
	| label
	| pragma
	| STRING
	| instruction
	;

instruction:
	COP_2ARG ',' reg normarg
	| COP_FD normarg
	| COP_KA1 ',' reg ',' expr '.'
	| COP_JS ',' expr '.'
	| COP_KA2 ',' expr '.'
	| COP_C ',' reg '.'
	| COP_SHC '.' reg ',' expr '.'
	| COP_S ',' zero '.'
	| COP_HLT ',' expr '.'
	| COP_J normarg
	| COP_L normarg
	| COP_G normarg
	| COP_BN normarg
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

pragma:
	CP_S expr '.'
	| CP_RES expr '.'
	| CP_RES expr ',' expr '.'
	| CP_F expr '.' floats
	| CP_NAME IDENTIFIER '.'
	| CP_BA expr '.'
	| CP_INT expr '.'
	| CP_OUT expr '.'
	| CP_LAB expr '.'
	| CP_NLAB
	| CP_MEM expr '.'
	| CP_OS
	| CP_IFUNK IDENTIFIER '.'
	| CP_IFUND IDENTIFIER '.'
	| CP_IFDEF IDENTIFIER '.'
	| CP_FI
	| CP_SS expr '.'
	| CP_HS
	| CP_MAX IDENTIFIER ',' maxlist '.'
	| CP_LEN expr '.'
	| CP_E expr '.'
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
