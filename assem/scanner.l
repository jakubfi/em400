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
#include <string.h>

#include "parser_modern.h"
#include "ops.h"

void yyerror(char *, ...);

char strbuf[1024];
char *strbufpos;

#define YY_USER_ACTION yylloc.first_line = yylineno;
#define YYABORT     goto yyabortlab


%}

%option nounput
%option noinput
%option yylineno

%x string

ws		[ \t\n]+
comment	[#;].*\n

oct		-?0[0-9]+
dec		-?[0-9]+
bin		0b[01]+
hex		-?0x[0-9a-fA-F]+

reg		r[0-7]
name	[a-zA-Z][a-zA-Z0-9_]*
data	\.data
equ		\.equ
res		\.res
program	\.program
endprog	\.endprog
chars	[-+:,\[\]<>]

%%

{ws}		;
{comment}	;

\"						{ BEGIN string; strbufpos = strbuf; }
<string>\"				{ yylval.str = strdup(strbuf); BEGIN 0; return STRING; }
<string>[^\\\"\n\r\t]*	{ strcpy(strbufpos, yytext); strbufpos += strlen(yytext); }
<string>\\\\			{ *strbufpos++ = '\\'; }
<string>\\\"			{ *strbufpos++ = '\"'; }
<string>\\n				{ *strbufpos++ = '\n'; }
<string>\\r				{ *strbufpos++ = '\r'; }
<string>\\t				{ *strbufpos++ = '\t'; }
<string>\n				{ BEGIN 0; yyerror("newline in string"); }
<string>\r				{ BEGIN 0; yyerror("carriage return in string"); }
<string>\\.				{ BEGIN 0; yyerror("unknown escape sequence: %s", yytext); }

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
{data}		return DATA;
{equ}		return EQU;
{res}		return RES;
{program}	return PROGRAM;
{endprog}	return ENDPROG;

{chars}	{
	return *yytext;
}
"<<"	return SHL;
">>"	return SHR;

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

.	{ yyerror("unknown character"); }

%%

int yywrap(void) {
	return 1;
}

// vim: tabstop=4
