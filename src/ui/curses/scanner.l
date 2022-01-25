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

#include "ui/curses/debugger.h"
#include "ui/curses/ui.h"
#include "ui/curses/cmd.h"
#include "ui/curses/keywords.h"
#include "debugger_parser.h"

void yyerror(char *s, ...);
%}

%top{
	#define _XOPEN_SOURCE 500
}

%option nounput
%option noinput
%option case-insensitive

%s args texts opts

oct		0[0-7]+
hex		0x[0-9a-f]+
bin		0b[01]+
dec		[0-9]+
reg		r[0-7]
regs	ic|sr|ir|ar|ac|kb|mod|modc|zc17|alarm
bases	int|uint|bin|oct|hex
name	[a-z%#_][a-z0-9#%]*
text	[^\n\t\r ]+
chars	[-,=+*&|^><:()\]\[~!]

%%

<texts>{text}	{ yylval.text = strdup(yytext); return TEXT; }

{oct}	{ yylval.value = strtoul(yytext+1, NULL, 8); return VALUE; }
{hex}	{ yylval.value = strtoul(yytext+2, NULL, 16); return VALUE; }
{bin}	{ yylval.value = strtoul(yytext+2, NULL, 2); return VALUE; }
{dec}	{ yylval.value = atoi(yytext); return VALUE; }

<INITIAL,args>{bases}	{ return find_token(kw_bases, yytext); }
<INITIAL,args>{reg}		{ yylval.value = atoi(yytext+1); return REG; }
<INITIAL,args>{regs}	{ yylval.value = find_token(kw_regs, yytext); return REG; }
<INITIAL,args>rz		{ return IRZ; }

<args>{name}	{ yylval.text = strdup(yytext); return NAME; }

<opts>add	{ BEGIN(INITIAL); return ADD; }
<opts>del	{ BEGIN(INITIAL); return DEL; }
<opts>test	{ BEGIN(INITIAL); return TEST; }
<opts>on	{ BEGIN(INITIAL); return ON; }
<opts>off	{ BEGIN(INITIAL); return OFF; }
<opts>enable { BEGIN(INITIAL); return ON; }
<opts>disable { BEGIN(INITIAL); return OFF; }
<opts>mem   { BEGIN(INITIAL); yylval.text = strdup(yytext); return NAME; }

quit	{ return F_QUIT; }
cycle	{ return F_CYCLE; }
regs	{ return F_REGS; }
sregs	{ return F_SREGS; }
clear	{ return F_CLEAR; }
memcfg	{ return F_MEMCFG; }
memmap	{ return F_MEMMAP; }
start	{ return F_START; }
stop	{ return F_STOP; }
stk		{ return F_STACK; }
help	{ BEGIN(texts); return F_HELP; }
dasm	{ BEGIN(args); return F_DASM; }
mem		{ BEGIN(args); return F_MEM; }
binload	{ BEGIN(args); return F_BIN; }
load	{ BEGIN(texts); return F_LOAD; }
brk		{ BEGIN(opts); return F_BRK; }
log		{ BEGIN(opts); return F_LOG; }
watch	{ BEGIN(opts); return F_WATCH; }
decode	{ BEGIN(opts); return F_DECODE; }
find	{ BEGIN(args); return F_FIND; }
clock	{ BEGIN(opts); return F_CLOCK; }
memdump	{ BEGIN(args); return F_MEMDUMP; }

{name}	{ yylval.text = strdup(yytext); return NAME; }

">>"	return SHR;
"<<"	return SHL;
"=="	return EQ;
"!="	return NEQ;
"<>"	return NEQ;
">="	return GE;
"<="	return LE;
"||"	return OR;
"&&"	return AND;

{chars} return *yytext;

\n {
	BEGIN(INITIAL);
	return *yytext;
}

[ \t]+	;

. {
	yylval.value = *yytext;
	return YERR;
}

%%

// -----------------------------------------------------------------------
void reset_scanner()
{
	BEGIN(INITIAL);

}

// -----------------------------------------------------------------------
int yywrap(void) {
	return 1;
}

// vim: tabstop=4 shiftwidth=4 autoindent
