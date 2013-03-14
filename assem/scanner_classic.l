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

#include "parser_classic.h"
#include "ops.h"
#include "pragmas.h"

void c_yyerror(char *, ...);

#define YY_USER_ACTION c_yylloc.first_line = c_yylineno;
#define YYABORT     goto c_yyabortlab


%}

%option nounput
%option noinput
%option yylineno

comment_silent	\[.*\]
comment_output	<.*>
identifier1		[a-zA-Z][a-zA-Z0-9]*
identifier2		[@#%][a-zA-Z0-9]*
dec				-?[0-9]+
oct				-?0[0-7]+
char2			!..
bitfield		\?[ZMVCLEGYX1234567]{1,16}
sfloat			-?[0-9]*\.[0-9]+
string			".+"

chars			[-+/=:\.,()&]
pragma			[A-Z ]\*
hs				#S

%%

{comment_silent} ;
{comment_output} printf("%s\n", yytext);
{identifier1} {
	c_yylval.str = strndup(yytext, 6);
	return IDENTIFIER;
}
{identifier2} {
	c_yylval.str = strndup(yytext, 7);
	return IDENTIFIER;
}
{oct} {
	c_yylval.val = strtol(yytext, NULL, 8);
	return VALUE;
}
{dec} {
	c_yylval.val = strtol(yytext, NULL, 10);
	return VALUE;
}
{char2} {
	c_yylval.val = ((*yytext) << 16) + *(yytext+1);
	return VALUE;
}
{bitfield} {
	int value = 0;
	while (*yytext) {
		switch (*yytext) {
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
	}
	return VALUE;
}
{sfloat} {
	c_yylval.str = strdup(yytext);
	return FLOAT;
}
{string} {
	c_yylval.str = strndup(yytext+1, strlen(yytext)-2);
	return STRING;
}

{chars} { return *yytext; }

{pragma}|{hs} {
	struct pragma_t *pragma = get_pragma(yytext);
	if (!pragma) c_yyerror("unknown pragma: %s", yytext);
	else return pragma->type;
}


.	{ c_yyerror("unknown character"); }

%%

int c_yywrap(void) {
	return 1;
}

// vim: tabstop=4
