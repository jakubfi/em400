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
	int value;
	char *text;
};

%token <value> VALUE
%token <text> TEXT
%token '-' ':'
%token F_QUIT F_CLMEM F_MEM F_REGS F_RESET F_STEP F_HELP F_DASM F_TRANS F_LOAD

%%

commands:
	commands command
	|
	;
	
command:
	function '\n'
	;

function:
	F_QUIT	{
		printf("Got: quit\n");
	}
	| F_STEP {
		printf("Got: step\n");
	}
	| f_help
	| F_REGS {
		printf("Got: regs\n");
	}
	| F_RESET {
		printf("Got: reset\n");
	}
	| f_dasm
	| f_trans
	| f_mem
	| F_CLMEM {
		printf("Got: clmem\n");
	}
	| f_load
	;

f_help:
	F_HELP {
		printf("Got: help\n");
	}
	| F_HELP TEXT {
		printf("Got: help %s\n", $2);
		free($2);
	}
	;

f_dasm:
	F_DASM {
		printf("Got: dasm\n");
	}
	| F_DASM VALUE {
		printf("Got: dasm %i\n", $2);
	}
	| F_DASM VALUE VALUE {
		printf("Got: dasm %i %i\n", $2, $3);
	}
	;

f_trans:
	F_TRANS
	| F_TRANS VALUE
	| F_TRANS VALUE VALUE
	;

f_mem:
	F_MEM VALUE '-' VALUE
	| F_MEM VALUE VALUE '-' VALUE
	| F_MEM VALUE ':' VALUE '-' VALUE

f_load:
	F_LOAD TEXT
	| F_LOAD TEXT VALUE
	;

%%

void yyerror(char *s) {
    fprintf(stdout, "%s\n", s);
}

// vim: tabstop=4
