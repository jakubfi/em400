%{
//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include "cfg_parser.h"

void cyyerror(char *);

#define YY_USER_ACTION cyylloc.first_line = cyylineno;

%}

%option nounput
%option noinput
%option yylineno

number	[0-9]+
text	[^,: \n\t]+
chars	[\}\{=:,]
comment	#.*$

%%

cpu			return CPU;
memory		return MEMORY;
speed		return SPEED;
max			return MAX;
real		return REAL;
timer_step	return TIMER;
mod_17bit	return MOD_17;
mod_sint	return MOD_SINT;
module		return MODULE;
elwro		return ELWRO;
mega		return MEGA;
true|yes	{ cyylval.value.v = 1; cyylval.value.s = strdup(cyytext); return VALUE; }
false|no	{ cyylval.value.v = 0; cyylval.value.s = strdup(cyytext); return VALUE; }
channel		return CHANNEL;
unit		return UNIT;

{number} {
	cyylval.value.v = strtol(cyytext, NULL, 10);
	cyylval.value.s = strdup(cyytext);
	return VALUE;
}

{chars} {
	return *cyytext;
}

[ \t\n]+	;
{comment}	;

{text} {
	cyylval.value.s = strdup(cyytext);
	return TEXT;
}

%%

int cyywrap(void) {
	return 1;
}

// vim: tabstop=4
