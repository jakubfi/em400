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

#include <stdlib.h>

#include "ectl.h"
#include "ectl/est.h"
#include "ectl_parser.h"

void yyerror(char *s, ...);

int ectl_yycolumn = 0;

#undef YY_USER_ACTION
#define YY_USER_ACTION \
	ectl_yylloc.first_column = ectl_yycolumn; \
	ectl_yylloc.last_column = ectl_yycolumn + ectl_yyleng-1; \
	ectl_yycolumn += ectl_yyleng;

%}

%top{
	#define _XOPEN_SOURCE 500
}

%option nounput
%option noinput
%option case-insensitive

%%

0x[0-9a-f]+	{ ectl_yylval.value = strtoul(yytext, NULL, 16); return VALUE; }
0b[01]+		{ ectl_yylval.value = strtoul(yytext+2, NULL, 2); return VALUE; }
0[0-7]+		{ ectl_yylval.value = strtoul(yytext, NULL, 8); return VALUE; }
[0-9]+		{ ectl_yylval.value = atoi(yytext); return VALUE; }
r[0-7]		{ ectl_yylval.value = atoi(yytext+1); return REG; }
ic			{ ectl_yylval.value = ECTL_REG_IC; return REG; }
sr			{ ectl_yylval.value = ECTL_REG_SR; return REG; }
ir			{ ectl_yylval.value = ECTL_REG_IR; return REG; }
kb			{ ectl_yylval.value = ECTL_REG_KB; return REG; }
mod			{ ectl_yylval.value = ECTL_REG_MOD; return REG; }
modc		{ ectl_yylval.value = ECTL_REG_MODc; return REG; }
alarm		{ ectl_yylval.value = ECTL_REG_ALARM; return REG; }
[zmvclegyx]	{ ectl_yylval.value = *yytext ; return FLAG; }
rz			{ return IRZ; }

">>"	return SHR;
"<<"	return SHL;
"=="	return EQ;
"!="	return NEQ;
"<>"	return NEQ;
">="	return GE;
"<="	return LE;
"||"	return OR;
"&&"	return AND;

[-+*/&|^><:()\]\[~!] return *yytext;

[ \t\n\r\v]+ ;

[a-z0-9A-Z]+ {
	ectl_yylval.str = strdup(yytext);
	return TOK_INVALID;
}

. {
	ectl_yylval.str = strdup(yytext);
	return TOK_INVALID;
}

%%

// -----------------------------------------------------------------------
int ectl_yywrap(void) {
	return 1;
}

// vim: tabstop=4 shiftwidth=4 autoindent
