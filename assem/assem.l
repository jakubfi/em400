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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "assem_parse.h"
#include "assem_ops.h"

void yyerror(char *);

%}

ws		[ \t]+
comment	^#.*

oct		-?0[0-9]+
dec		-?[0-9]+
bin		0b[01]+
hex		-?0x[0-9a-fA-F]+

reg		r[0-7]
name	[a-zA-Z][a-zA-Z0-9_]+
dlabel	[a-zA-Z][a-zA-Z0-9_]+:
data	.data
chars	[\[\]\+\,]

%%

{oct}	{
	yylval.val = strtol(yytext, NULL, 8);
	return VALUE;
}
{dec} 	{
	yylval.val = strtol(yytext, NULL, 10);
	return VALUE;
}
{bin}	{
	yylval.val = strtol(yytext+2, NULL, 2);
	return VALUE;
}
{hex}	{
	yylval.val = strtol(yytext+2, NULL, 16);
	return VALUE;
}
{reg}	{
	yylval.val = strtol(yytext+1, NULL, 10);
	return REGISTER;
}
{dlabel}	{
	yylval.str = strdup(yytext);
	yylval.str[strlen(yytext)-1] = '\0';
	return DLABEL;
}
{data}	{
	return DATA;
}

{chars}	{
	return *yytext;
}

{name}	{
	struct op_t *op = get_op(yytext);
	if (op) {
		yylval.val = op->opcode;
		return op->type;
	} else {
		yylval.str = strdup(yytext);
		return NAME;
	}
}

{ws}		;
{comment}	;

.	yyerror("Unknown character");

%%

int yywrap(void) {
	return 1;
}

// vim: tabstop=4
