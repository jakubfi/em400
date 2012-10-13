%{
//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

void yyerror(char *);
int yylex(void);
%}

%union {
	char *str;
	int val;
};

%token '[' ']' ',' '+'
%token DATA
%token <str> DLABEL NAME
%token <val> VALUE REGISTER
%token <val> OP_2ARG OP_FD OP_KA1 OP_JS OP_KA2 OP_C OP_SHC OP_S OP_J OP_L OP_G OP_BN

%%

sentences:
	sentences sentence
	|
	;

sentence:
	instruction
	| data
	| DLABEL instruction
	| DLABEL data
	;

data:
	DATA VALUE
	;

instruction:
	OP_2ARG REGISTER ',' norm
	| OP_FD norm
	| OP_KA1 REGISTER VALUE
	| OP_JS VALUE
	| OP_KA2 VALUE
	| OP_C REGISTER
	| OP_SHC VALUE
	| OP_S
	| OP_J norm
	| OP_L norm
	| OP_G norm
	| OP_BN norm
	;

norm:
	normval
	| '[' normval ']'
	;

normval:
	REGISTER
	| addrval
	| REGISTER '+' REGISTER
	| REGISTER '+' addrval
	;

addrval:
	VALUE
	| NAME
	;

%%

void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
}

int main(void) {
	yyparse();
}

// vim: tabstop=4
