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

#include "debuger.h"
#include "debuger_ui.h"
#include "debuger_parser.h"

void yyerror(char *);
%}

oct		0[0-7]+
hex		0x[0-9a-fA-F]+
bin		0b[01]+
dec		[0-9]+
text	[./_a-zA-Z0-9]+

%%

{oct} {
	yylval.value = strtoul(yytext+1, NULL, 8);
	return VALUE;
}
{hex}	{
	yylval.value = strtoul(yytext+2, NULL, 16);
	return VALUE;
}
{bin} {
	yylval.value = strtoul(yytext+2, NULL, 2);
	return VALUE;
}
{dec}	{
	yylval.value = atoi(yytext);
	return VALUE;
}
{text}	{
	int tok = debuger_is_cmd(yytext);
	if (!tok) {
		yylval.text = strdup(yytext);
		return TEXT;
	} else {
		yylval.value = tok;
		return tok;
	}
}

[-:\n]	return *yytext;

[ \t]+	;
. {
	yylval.value = *yytext;
	return YERR;
}
%%

int yywrap(void) {
	return 1;
}

// vim: tabstop=4
