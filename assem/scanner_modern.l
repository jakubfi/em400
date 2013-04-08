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

#include "ops.h"
#include "parser_modern.h"
#include "pragmas_modern.h"

void m_yyerror(char *, ...);

char strbuf[1024];
char *strbufpos;

int uop_to_modern[] = { MOP_2ARG, MOP_FD, MOP_KA1, MOP_JS, MOP_KA2, MOP_C, MOP_SHC, MOP_HLT, MOP_S, MOP_J, MOP_L, MOP_G, MOP_BN };

#define YY_USER_ACTION	m_yylloc.first_line = m_yylineno;

%}

%option nounput
%option noinput
%option yylineno

%x string

ws		[ \t\n]+
cmtcode	[#;].*$
cmtline	^[#;].*$

oct		-?0[0-9]+
dec		-?[0-9]+
bin		0b[01]+
hex		-?0x[0-9a-fA-F]+

reg		r[0-7]
name	[a-zA-Z][a-zA-Z0-9_]*
pragma	\.[a-z]+
chars	[-+:,\[\]<>]

%%

{ws}		;
{cmtline}	{ m_yylval.val.s = strdup(m_yytext); return CMT_LINE; }
{cmtcode}	{ m_yylval.val.s = strdup(m_yytext); return CMT_CODE; }

\"						{ BEGIN string; strbufpos = strbuf; }
<string>\"				{ m_yylval.val.s = strdup(strbuf); BEGIN 0; return STRING; }
<string>[^\\\"\n\r\t]*	{ strcpy(strbufpos, m_yytext); strbufpos += strlen(m_yytext); }
<string>\\\\			{ *strbufpos++ = '\\'; }
<string>\\\"			{ *strbufpos++ = '\"'; }
<string>\\n				{ *strbufpos++ = '\n'; }
<string>\\r				{ *strbufpos++ = '\r'; }
<string>\\t				{ *strbufpos++ = '\t'; }
<string>\n				{ BEGIN 0; m_yyerror("newline in string"); }
<string>\r				{ BEGIN 0; m_yyerror("carriage return in string"); }
<string>\\.				{ BEGIN 0; m_yyerror("unknown escape sequence: %s", m_yytext); }

{oct}	{
	m_yylval.val.v = strtol(m_yytext, NULL, 8);
	m_yylval.val.s = strdup(m_yytext);
	return VALUE;
}
{dec} 	{
	m_yylval.val.v = strtol(m_yytext, NULL, 10);
	m_yylval.val.s = strdup(m_yytext);
	return VALUE;
}
{bin}	{
	m_yylval.val.v = strtol(m_yytext+2, NULL, 2);
	m_yylval.val.s = strdup(m_yytext);
	return VALUE;
}
{hex}	{
	m_yylval.val.v = strtol(m_yytext+2, NULL, 16);
	m_yylval.val.s = strdup(m_yytext);
	return VALUE;
}
{reg}	{
	m_yylval.val.v = strtol(m_yytext+1, NULL, 10);
	return REGISTER;
}
{pragma} {
	struct pragma_t *p = get_pragma(pragmas_modern, yytext);
	if (!p) {
		m_yyerror("unknown pragma: %s", yytext);
		return 0;
	}
	return p->type;
}

{chars}	{
	return *m_yytext;
}
"<<"	return SHL;
">>"	return SHR;

{name}	{
	struct op_t *op = get_op(m_yytext);
	if (op) {
		m_yylval.val.v = op->opcode;
		return uop_to_modern[op->type];
	} else {
		m_yylval.val.s = strdup(m_yytext);
		return NAME;
	}
}

.	{ m_yyerror("unknown character"); return 0; }

%%

int m_yywrap(void) {
	return 1;
}

// vim: tabstop=4
