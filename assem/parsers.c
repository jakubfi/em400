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

#include <stdio.h>

#include "parsers.h"
#include "parser_modern.h"
#include "parser_classic.h"

extern FILE *m_yyin;
extern FILE *c_yyin;
int m_yyparse();
int c_yyparse();
int m_yylex_destroy();
int c_yylex_destroy();

int syntax = MODERN;
int got_error;
int parser_lineno;

// -----------------------------------------------------------------------
int parse(FILE *source)
{
	m_yyin = c_yyin = source;

	int (*yyparser)();
	int (*yylex_destroy)();

	if (syntax == CLASSIC) {
		yyparser = c_yyparse;
		yylex_destroy = c_yylex_destroy;
	} else {
		yyparser = m_yyparse;
		yylex_destroy = m_yylex_destroy;
	}

	do {
		yyparser();
	} while (!feof(source));

	yylex_destroy();

	if (got_error) {
		return -1;
	}

	return 1;
}

// vim: tabstop=4
