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
#include "log.h"

struct cfg_chan *this_chan;

void cyyerror(struct cfg_em400 *cfg, const char *s, ...);
int cyylex(void);
%}

%error-verbose
%locations
%parse-param {struct cfg_em400 *cfg}

%code requires {#include "cfg.h"}

%union {
	struct value_t {
		int v;
		float f;
		char *s;
	} value;
	struct cfg_arg *arg;
};

%token END 0 "end of file"
%token COMPUTER "`computer`"
%token FPGA "`fpga`"
%token SOUND "`sound`"
%token CHANNEL "`channel`"
%token UNIT "`unit`"
%token SPEED_REAL "`speed_real`"
%token CPU_SPEED_FACTOR "`cpu_speed_factor`"
%token THROTTLE_GRANULARITY "`throttle_granularity`"
%token CLOCK_PERIOD "`clock_period`"
%token CLOCK_START "`clock_start`"
%token CPU_MOD "`cpu_mod`"
%token CPU_USER_IO_ILLEGAL "`cpu_user_io_illegal`"
%token CPU_AWP "`cpu_awp`"
%token ELWRO "`elwro`"
%token MEGA "`mega`"
%token MEGA_PROM "`mega_prom`"
%token MEGA_BOOT "`mega_boot`"
%token OS_SEG "`os_seg`"
%token CPU_STOP_ON_NOMEM "`cpu_stop_on_nomem`"
%token LOG "`log`"
%token ENABLED "`enabled`"
%token LFILE "`file`"
%token COMPONENTS "loging components"
%token LINEBUF "line buffered"
%token DEVICE "`device`"
%token SPEED "`speed`"
%token DRIVER "`driver`"
%token RATE "`rate`"
%token BUFFER_LEN "`buffer_len`"
%token VOLUME "`volume`"
%token <value> NAME "parameter or device name"
%token <value> STRING "string"
%token <value> VALUE "integer value"
%token <value> FLOAT "floating point value"
%token <value> BOOL "`true` or `false`"
%type <arg> arg
%type <arg> arglist

%token '{' '}' '=' ':' ','

%%

objects:
	objects object
	|
	;

object:
	COMPUTER '{' computer_opts '}'
	| LOG '{' log_opts '}'
	| FPGA '{' fpga_opts '}'
	| SOUND '{' sound_opts '}'
	| CHANNEL VALUE '=' NAME { cfg_make_chan(cfg, $2.v, $4.s); free($2.s); } '{' units '}'
	;

units:
	unit units
	|
	;

unit:
	UNIT VALUE '=' NAME ':' arglist	{ cfg_make_unit(cfg, $2.v, $4.s, $6); free($2.s); }
	| UNIT VALUE '=' NAME			{ cfg_make_unit(cfg, $2.v, $4.s, NULL); free($2.s); }
	;

arglist:
	arg ',' arglist	{ $1->next = $3; $$ = $1; }
	| arg			{ $$ = $1; }
	;

arg:
	VALUE	{ $$ = cfg_make_arg($1.s); }
	| NAME	{ $$ = cfg_make_arg($1.s); }
	;

computer_opts:
	computer_opts computer_opt
	|
	;

computer_opt:
	SPEED_REAL '=' BOOL		{ cfg->speed_real = $3.v; free($3.s); }
	| CPU_SPEED_FACTOR '=' FLOAT { cfg->cpu_speed_factor = $3.f; free($3.s); }
	| CPU_SPEED_FACTOR '=' VALUE { cfg->cpu_speed_factor = $3.v; free($3.s); }
	| THROTTLE_GRANULARITY '=' VALUE { cfg->throttle_granularity = $3.v; free($3.s); }
	| CLOCK_PERIOD '=' VALUE{ cfg->clock_period = $3.v; free($3.s); }
	| CLOCK_START '=' BOOL	{ cfg->clock_start = $3.v; free($3.s); }
	| CPU_MOD '=' BOOL		{ cfg->cpu_mod = $3.v; free($3.s); }
	| CPU_USER_IO_ILLEGAL '=' BOOL { cfg->cpu_user_io_illegal = $3.v; free($3.s); }
	| CPU_AWP '=' BOOL		{ cfg->cpu_awp = $3.v; free($3.s); }
	| ELWRO '=' VALUE		{ cfg->mem_elwro = $3.v; free($3.s); }
	| MEGA '=' VALUE		{ cfg->mem_mega = $3.v; free($3.s); }
	| MEGA_BOOT '=' BOOL	{ cfg->mem_mega_boot = $3.v; free($3.s); }
	| CPU_STOP_ON_NOMEM '=' BOOL { cfg->cpu_stop_on_nomem = $3.v ; free($3.s); }
	| MEGA_PROM '=' STRING	{ free(cfg->mem_mega_prom); cfg->mem_mega_prom = $3.s; }
	| OS_SEG '=' VALUE		{ cfg->mem_os = $3.v; free($3.s); }
	| FPGA '=' BOOL			{ cfg->fpga = $3.v; free($3.s); }
	;

log_opts:
	log_opts log_opt
	|
	;

log_opt:
	ENABLED '=' BOOL { cfg->log_enabled = $3.v; free($3.s); }
	| LFILE '=' STRING { free(cfg->log_file); cfg->log_file = $3.s; }
	| COMPONENTS '=' STRING { free(cfg->log_components); cfg->log_components = $3.s; }
	| LINEBUF '=' BOOL { cfg->line_buffered = $3.v; free($3.s); }
	;

fpga_opts:
	fpga_opts fpga_opt
	|
	;

fpga_opt:
	SPEED '=' VALUE { cfg->fpga_speed = $3.v; free($3.s); }
	| DEVICE '=' STRING { free(cfg->fpga_dev); cfg->fpga_dev = $3.s; }
	;

sound_opts:
	sound_opts sound_opt
	|
	;

sound_opt:
	DEVICE '=' STRING { free(cfg->sound_device); cfg->sound_device = $3.s; }
	| DRIVER '=' STRING { free(cfg->sound_driver); cfg->sound_driver = $3.s; }
	| RATE '=' VALUE { cfg->sound_rate = $3.v; free($3.s); }
	| BUFFER_LEN '=' VALUE { cfg->sound_buffer_len = $3.v; free($3.s); }
	| ENABLED '=' BOOL { cfg->sound_enabled = $3.v; free($3.s); }
	| VOLUME '=' VALUE	{ cfg->sound_volume = $3.v; free($3.s); }
	;

%%

// -----------------------------------------------------------------------
void cyyerror(struct cfg_em400 *cfg, const char *s, ...)
{
	char buf[4096];
	va_list ap;
	va_start(ap, s);
	vsnprintf(buf, 4095, s, ap);
	va_end(ap);

	LOGERR("Error parsing config, line %d: %s.", yylloc.first_line, buf);
	cfg_error = 1;
}

// vim: tabstop=4 shiftwidth=4 autoindent
