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
	i_2arg
	| i_fd
	| i_ka1
	| i_js
	| i_ka2
	| i_c
	| i_shc
	| i_s
	| i_j
	| i_l
	| i_g
	| i_bn
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

i_2arg:
	OP_2ARG REGISTER ',' norm
	;

i_fd:
	OP_FD norm
	;
i_ka1:
	OP_KA1 REGISTER VALUE
	;

i_js:
	OP_JS VALUE
	;

i_ka2:
	OP_KA2 VALUE
	;

i_c:
	OP_C REGISTER
	;

i_shc:
	OP_SHC VALUE
	;

i_s:
	OP_S
	;

i_j:
	OP_J norm
	;

i_l:
	OP_L norm
	;

i_g:
	OP_G norm
	;

i_bn:
	OP_BN norm
	;

%%

void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
}

int main(void) {
	yyparse();
}

// vim: tabstop=4
