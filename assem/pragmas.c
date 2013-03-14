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
#include <strings.h>

#include "pragmas.h"
#include "parser_classic.h"

struct pragma_t pragmas[] = {
{ "S*",			S },
{ "RES*",		RES },
{ "F*",			F },
{ "PROG*",		PROG },
{ "FINPROG*",	FINPROG },
{ "SEG*",		SEG },
{ "FINSEG*",	FINSEG },
{ "MACRO*",		MACRO },
{ "FINMACRO*",	FINMACRO },
{ "ALL*",		ALL },
{ "NAME*",		NAME },
{ "BA*",		BA },
{ "MEM*",		MEM },
{ "OS*",		OS },
{ "IFUNK*",		IFUNK },
{ "IF UNK*",	IFUNK },
{ "IFUND*",		IFUND },
{ "IF UND*",	IFUND },
{ "IFDEF*",		IFDEF },
{ "IF DEF*",	IFDEF },
{ "FI*",		FI },
{ "SS*",		SS },
{ "MAX*",		MAX },
{ "LEN*",		LEN },
{ "E*",			E },
{ "#S", 		HS },
{ NULL, 0 }
};

// -----------------------------------------------------------------------
struct pragma_t * get_pragma(char *prname)
{
	struct pragma_t *pr = pragmas;

	while (pr->name) {
		if (!strcasecmp(pr->name, prname)) {
			return pr;
		}
		pr++;
	}

	return NULL;
}

// vim: tabstop=4
