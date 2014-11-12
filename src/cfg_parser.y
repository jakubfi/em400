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
#include <stdio.h>
#include <stdarg.h>

#include "cfg.h"

struct cfg_chan *this_chan;

void cyyerror(struct cfg_em400 *cfg, char *s, ...);
int cyylex(void);
%}

%locations
%parse-param {struct cfg_em400 *cfg}

%union {
	struct value_t {
		int v;
		char *s;
	} value;
	struct cfg_arg *arg;
};

%token COMPUTER CHANNEL UNIT
%token SPEED_REAL TIMER_STEP TIMER_START CPU_MOD CPU_USER_IO_ILLEGAL CPU_AWP
%token ELWRO MEGA MEGA_PROM MEGA_BOOT OS_SEG CPU_STOP_ON_NOMEM
%token LOG ENABLED LFILE LEVELS PNAME_OFFSET
%token <value> TEXT STRING
%token <value> VALUE
%token <value> BOOL
%type <arg> arg arglist

%token '{' '}' '=' ':' ','

%%

objects:
	objects object
	|
	;

object:
	COMPUTER '{' computer_opts '}'
	| LOG '{' log_opts '}'
	| CHANNEL VALUE '=' TEXT { cfg_make_chan(cfg, $2.v, $4.s); free($2.s); } '{' units '}'
	;

units:
	unit units
	|
	;

unit:
	UNIT VALUE '=' TEXT ':' arglist	{ cfg_make_unit(cfg, $2.v, $4.s, $6); free($2.s); }
	| UNIT VALUE '=' TEXT			{ cfg_make_unit(cfg, $2.v, $4.s, NULL); free($2.s); }
	;

arglist:
	arg ',' arglist	{ $1->next = $3; $$ = $1; }
	| arg			{ $$ = $1; }
	;

arg:
	VALUE	{ $$ = cfg_make_arg($1.s); }
	| TEXT	{ $$ = cfg_make_arg($1.s); }
	;

computer_opts:
	computer_opts computer_opt
	|
	;

computer_opt:
	SPEED_REAL '=' BOOL		{ cfg->speed_real = $3.v; free($3.s); }
	| TIMER_STEP '=' VALUE	{ cfg->timer_step = $3.v; free($3.s); }
	| TIMER_START '=' BOOL	{ cfg->timer_start = $3.v; free($3.s); }
	| CPU_MOD '=' BOOL		{ cfg->cpu_mod = $3.v; free($3.s); }
	| CPU_USER_IO_ILLEGAL '=' BOOL	{ cfg->cpu_user_io_illegal = $3.v; free($3.s); }
	| CPU_AWP '=' BOOL		{ cfg->cpu_awp = $3.v; free($3.s); }
	| ELWRO '=' VALUE		{ cfg->mem_elwro = $3.v; free($3.s); }
	| MEGA '=' VALUE		{ cfg->mem_mega = $3.v; free($3.s); }
	| MEGA_BOOT '=' BOOL	{ cfg->mem_mega_boot = $3.v; cfg->cpu_stop_on_nomem = 0; free($3.s); }
	| CPU_STOP_ON_NOMEM '=' BOOL { if (!cfg->mem_mega_boot) cfg->cpu_stop_on_nomem = $3.v ; free($3.s)}
	| MEGA_PROM '=' STRING	{ cfg->mem_mega_prom = $3.s; }
	| OS_SEG '=' VALUE		{ cfg->mem_os = $3.v; free($3.s); }
	;

log_opts:
	log_opts log_opt
	|
	;

log_opt:
	ENABLED '=' BOOL { cfg->log_enabled = $3.v; free($3.s); }
	| LFILE '=' STRING { free(cfg->log_file); cfg->log_file = $3.s; }
	| LEVELS '=' STRING { free(cfg->log_levels); cfg->log_levels = $3.s; }
	| PNAME_OFFSET '=' VALUE { cfg->log_pname_offset = $3.v; free($3.s); }
	;
%%

// -----------------------------------------------------------------------
void cyyerror(struct cfg_em400 *cfg, char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	printf("Error parsing config, line %d: ", yylloc.first_line);
	vprintf(s, ap);
	printf("\n");
	cfg_error = 1;
}

// vim: tabstop=4 shiftwidth=4 autoindent
