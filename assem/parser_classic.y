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

%}
%error-verbose
%locations
%union {
	char *str;
	int val;
	struct word_t *word;
	struct node_t *norm;
	struct node_t *node;
};

%left '+' '-'
%left '/'
%nonassoc UMINUS
%token COMMENT
%token '.' '=' ':' ',' '(' ')' '&'
%token CP_S CP_RES CP_F CP_PROG CP_FINPROG CP_SEG CP_FINSEG CP_MACRO CP_FINMACRO CP_ALL CP_NAME CP_BA CP_INT CP_OUT CP_LAB CP_NLAB CP_MEM CP_OS CP_IFUNK CP_IFUND CP_IFDEF CP_FI CP_SS CP_HS CP_MAX CP_LEN CP_E CP_FILE CP_TEXT
%token <str> IDENTIFIER FLOAT STRING
%token <str> LABEL VAR
%token <val> VALUE
%token <val> COP_2ARG COP_FD COP_KA1 COP_JS COP_KA2 COP_C COP_SHC COP_S COP_HLT COP_J COP_L COP_G COP_BN
%type <norm> normarg norm1 norm2
%type <node> expr

%%

program:
	preblock CP_PROG '*' { printf("NEW VARSET: PROG\n"); } codeblock CP_FINPROG '*'
	;

segment:
	CP_SEG '*' { printf("NEW VARSET: SEG\n"); } codeblock CP_FINSEG '*'
	;

macro:
	CP_MACRO '*' { printf("NEW VARSET: MACRO\n"); } codeblock CP_FINMACRO '*'
	;

preblock:
	preblock pre
	|
	;

pre:
	COMMENT
	| pragma
	;

codeblock:
	codeblock code
	|
	;

code:
	vardef
	| label { printf("(label)\n");}
	| pragma
	| STRING
	| instruction { printf("(instr)\n"); }
	| COMMENT
	| expr '.' { printf("(expr)\n"); }
	| '.' { printf("(empty)\n"); }
	| macro
	| segment
	;

instruction:
	COP_2ARG ',' reg normarg {}
	| COP_FD normarg
	| COP_KA1 ',' reg ',' expr '.'
	| COP_JS ',' expr '.'
	| COP_KA2 ',' expr '.'
	| COP_C ',' reg '.'
	| COP_SHC ',' reg ',' expr '.'
	| COP_S ',' zero '.'
	| COP_HLT ',' expr '.'
	| COP_J normarg
	| COP_L normarg
	| COP_G normarg
	| COP_BN normarg
	;

normarg:
	',' norm1 '.' { $$=NULL; }
	| ',' norm1 '\'' '.' { $$=NULL; }
	| '(' norm2 ')' { $$=NULL; }
	| '(' norm2 '\'' ')' { $$=NULL; }
	;

norm1:
	reg { $$=NULL;}
	| reg '&' reg { $$=NULL; }
	;

norm2:
	expr { $$=NULL; }
	| expr '&' reg { $$=NULL; }
	;

expr:
	value { $$=NULL; }
	| expr '+' expr { $$=NULL; }
	| expr '-' expr { $$=NULL; }
	| value '/' value { $$=NULL; }
	| '-' expr %prec UMINUS { $$=NULL; }
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
/*	| IDENTIFIER { printf("mamto\n"); }*/
	;

vardef:
	VAR '=' expr '.' {
		if (!dict_add(dict, D_VALUE, $1, $3)) {
			//c_yyerror("name '%s' already defined", $1);
			//YYABORT;
		}
	}
	;

label:
	LABEL ':' {
		struct node_t *n = make_value(program_ic, NULL);
		if (!n) {
			c_yyerror("cannot make node for '%s'", $1);
			YYABORT;
		}
		if (!dict_add(dict, D_ADDR, $1, n)) {
			//c_yyerror("name '%s' already defined", $1);
			//YYABORT;
		}
	}
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
