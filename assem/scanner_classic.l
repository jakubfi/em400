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
#include <ctype.h>

#include "ops.h"
#include "parser_classic.h"
#include "pragmas_classic.h"

void c_yyerror(char *, ...);

int uop_to_classic[] = { COP_2ARG, COP_FD, COP_KA1, COP_JS, COP_KA2, COP_C, COP_SHC, COP_HLT, COP_S, COP_J, COP_L, COP_G, COP_BN };

#define YY_USER_ACTION	c_yylloc.first_line = c_yylineno;
#define YYABORT			goto c_yyabortlab

%}

%option nounput
%option noinput
%option yylineno
%s is_id in_comment

ws				[ \t\n\r\f]
comment_output	\<.*\>
name			[a-zA-Z]+
identifier		[$@#%a-zA-Z][a-zA-Z0-9]*
suffix_op		[,\(]
dec				[0-9]+
oct				0[0-7]+
char2			!..
bitfield		\?[ZMVCLEGYX1234567]{1,16}
sfloat			[0-9]*\.[0-9]+
string			\".+\"
chars			[-+/=:,\(&'\*]
echars			[\)\.]

%%

<INITIAL,is_id>\[ BEGIN(in_comment);
<in_comment>{
[^\]\n]+ ;
\n ;
\] BEGIN(INITIAL);
}
{comment_output} {
	printf("%s\n", c_yytext);
	return COMMENT;
}

{oct} {
	printf("oct: %s", c_yytext);
	c_yylval.val.v = strtol(c_yytext, NULL, 8);
	c_yylval.val.s = strdup(c_yytext);
	return VALUE;
}
{dec} {
	printf("dec: %s", c_yytext);
	c_yylval.val.v = strtol(c_yytext, NULL, 10);
	c_yylval.val.s = strdup(c_yytext);
	return VALUE;
}
{char2} {
	printf("2char: %s", c_yytext);
	c_yylval.val.v = ((*c_yytext) << 16) + *(c_yytext+1);
	c_yylval.val.s = strdup(c_yytext);
	return VALUE;
}
{bitfield} {
	printf("bitfield: %s", c_yytext);
	int value = 0;
	while (*c_yytext) {
		switch (*c_yytext) {
			case 'Z': value |= 1 << 15; break;
			case 'M': value |= 1 << 14; break;
			case 'V': value |= 1 << 13; break;
			case 'C': value |= 1 << 12; break;
			case 'L': value |= 1 << 11; break;
			case 'E': value |= 1 << 10; break;
			case 'G': value |= 1 << 9; break;
			case 'Y': value |= 1 << 8; break;
			case 'X': value |= 1 << 7; break;
			case '1': value |= 1 << 6; break;
			case '2': value |= 1 << 5; break;
			case '3': value |= 1 << 4; break;
			case '4': value |= 1 << 3; break;
			case '5': value |= 1 << 2; break;
			case '6': value |= 1 << 1; break;
			case '7': value |= 1 << 0; break;
		}
		c_yytext++;
	}
	c_yylval.val.v = value;
	c_yylval.val.s = strdup(c_yytext);
	return VALUE;
}
{string} {
	printf("string: %s", c_yytext);
	c_yylval.val.s = strndup(c_yytext+1, strlen(c_yytext)-2);
	return STRING;
}

{chars} { printf(" '%c' ", *c_yytext); return *c_yytext; }
{echars} { BEGIN(INITIAL); printf(" '%c'\n", *c_yytext); return *c_yytext; }

<is_id>{name} {
	printf("id: %s ", c_yytext);
	int copy = 7;
	if (isalpha(*c_yytext)) copy = 6;
	c_yylval.val.s = strndup(c_yytext, copy);
	return IDENTIFIER;
}

{name}/{suffix_op} {
	struct op_t *op = get_op(c_yytext);
	if (op) {
		BEGIN(is_id);
		printf("opcode: %s ", c_yytext);
		c_yylval.val.v = op->opcode;
		return uop_to_classic[op->type];
	} else {
		printf("id: %s ", c_yytext);
		c_yylval.val.s = strndup(c_yytext, 6);
		return IDENTIFIER;
	}
}

{name}/\* {
	BEGIN(is_id);
	struct pragma_t *pragma = get_pragma(pragmas_classic, c_yytext);
	if (!pragma) {
		c_yyerror("unknown pragma: %s", c_yytext);
	} else {
		printf("pragma: %s ", c_yytext);
		return pragma->type;
	}
}

{identifier}/: {
	printf("labeldef: %s ", c_yytext);
	int copy = 7;
	if (isalpha(*c_yytext)) copy = 6;
	c_yylval.val.s = strndup(c_yytext, copy);
	return LABEL;
}

{identifier}/= {
	printf("vardef: %s ", c_yytext);
	int copy = 7;
	if (isalpha(*c_yytext)) copy = 6;
	c_yylval.val.s = strndup(c_yytext, copy);
	return VAR;
}

{identifier} {
	printf("id: %s ", c_yytext);
	int copy = 7;
	if (isalpha(*c_yytext)) copy = 6;
	c_yylval.val.s = strndup(c_yytext, copy);
	return IDENTIFIER;
}

{ws} ;
. { printf("Unknown character: '%c'", *c_yytext); }

%%

int c_yywrap(void) {
	return 1;
}

// vim: tabstop=4
