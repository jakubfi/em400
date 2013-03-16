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

struct pragma_t pragmas_classic[] = {
{ "S*",			CP_S },
{ "RES*",		CP_RES },
{ "F*",			CP_F },
{ "PROG*",		CP_PROG },
{ "FINPROG*",	CP_FINPROG },
{ "SEG*",		CP_SEG },
{ "FINSEG*",	CP_FINSEG },
{ "MACRO*",		CP_MACRO },
{ "FINMACRO*",	CP_FINMACRO },
{ "ALL*",		CP_ALL },
{ "NAME*",		CP_NAME },
{ "BA*",		CP_BA },
{ "MEM*",		CP_MEM },
{ "OS*",		CP_OS },
{ "IFUNK*",		CP_IFUNK },
{ "IF UNK*",	CP_IFUNK },
{ "IFUND*",		CP_IFUND },
{ "IF UND*",	CP_IFUND },
{ "IFDEF*",		CP_IFDEF },
{ "IF DEF*",	CP_IFDEF },
{ "FI*",		CP_FI },
{ "SS*",		CP_SS },
{ "MAX*",		CP_MAX },
{ "LEN*",		CP_LEN },
{ "E*",			CP_E },
{ "#S", 		CP_HS },

{ NULL, 0 }
};

// vim: tabstop=4
