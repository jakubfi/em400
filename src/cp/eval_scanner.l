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

#include "libem400.h"
#include "eval_parser.h"

void yyerror(char *s, ...);

int eval_yycolumn = 0;

#undef YY_USER_ACTION
#define YY_USER_ACTION \
    eval_yylloc.first_column = eval_yycolumn; \
	eval_yylloc.last_column = eval_yycolumn + eval_yyleng-1; \
	eval_yycolumn += eval_yyleng;

%}

%top{
	#define _XOPEN_SOURCE 500
}

%option nounput
%option noinput
%option case-insensitive

%%

0x[0-9a-f]+	{ eval_yylval.value = strtoul(yytext, NULL, 16); return VALUE; }
0b[01]+		{ eval_yylval.value = strtoul(yytext+2, NULL, 2); return VALUE; }
0[0-7]+		{ eval_yylval.value = strtoul(yytext, NULL, 8); return VALUE; }
[0-9]+		{ eval_yylval.value = atoi(yytext); return VALUE; }
r[0-7]		{ eval_yylval.value = atoi(yytext+1); return REG; }
ic			{ eval_yylval.value = EM400_REG_IC; return REG; }
sr			{ eval_yylval.value = EM400_REG_SR; return REG; }
ir			{ eval_yylval.value = EM400_REG_IR; return REG; }
ar			{ eval_yylval.value = EM400_REG_AR; return REG; }
ac			{ eval_yylval.value = EM400_REG_AC; return REG; }
kb			{ eval_yylval.value = EM400_REG_KB; return REG; }
alarm		{ return ALARM; }
mc			{ return MC; }
[zmvclegyx]	{ eval_yylval.value = *yytext ; return FLAG; }
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
    eval_yylval.str = strdup(yytext);
	return TOK_INVALID;
}

. {
    eval_yylval.str = strdup(yytext);
	return TOK_INVALID;
}

%%

// -----------------------------------------------------------------------
int eval_yywrap(void) {
	return 1;
}

// vim: tabstop=4 shiftwidth=4 autoindent
