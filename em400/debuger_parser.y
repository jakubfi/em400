%{
//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdlib.h>
#include <stdarg.h>

void yyerror(char *);
int yylex(void);
%}

%union {
    char *str;
    int val;
};

%token '[' ']' ':' '-' ',' '='
%token F_QUIT F_CLMEM F_RUN F_BRK F_REGS F_RESET F_MCL F_MEMDUMP F_STEP F_HELP F_DASM F_TRANS F_LOAD F_SAVE
%token IN OU INT
%token <val> VALUE REGISTER NB
%token <str> NAME CONDITION BIT

%%

commands:
	commands command
	|
	;
	
command:
	function
	| assignment
	| value
	;

assignment:
	REGISTER '=' value
	| memory '=' value
	;

value:
	REGISTER
	| memory
	| VALUE
	| bitchunk
	| BIT
	;

memory:
	'[' value ']'
	| NB ':' '[' value ']'

bitchunk:
	value '[' bitlist ']'

bitlist:
	bits
	| bitlist ',' bits
	;

bits:
	VALUE
	| VALUE '-' VALUE
	;

function:
	F_QUIT
	| F_CLMEM
	| f_memdump
	| f_step
	| F_RUN
	| f_help
	| F_REGS
	| F_RESET
	| f_dasm
	| f_trans
	| F_MCL
	| f_load
	| f_save
	| f_brk
	;

f_memdump:
	F_MEMDUMP value value
	F_MEMDUMP value '-' value
	| F_MEMDUMP value value value
	| F_MEMDUMP value ':' value value
	| F_MEMDUMP value value '-' value
	| F_MEMDUMP value ':' value '-' value
	;

f_step:
	F_STEP
	| F_STEP value
	;

f_help:
	F_HELP
	| F_HELP NAME
	;

f_dasm:
	F_DASM
	| F_DASM value
	;

f_trans:
	F_TRANS
	| F_TRANS value
	;

f_load:
	F_LOAD NAME
	| F_LOAD value NAME
	;

f_save:
	F_SAVE NAME
	| F_SAVE value NAME
	;

f_brk:
	F_BRK value CONDITION value
	| F_BRK value
	| F_BRK INT value
	| F_BRK IN
	| F_BRK OU
	;

%%
