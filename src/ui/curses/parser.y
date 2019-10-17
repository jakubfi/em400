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

#include "ectl.h"

#include "ui/curses/debugger.h"
#include "ui/curses/ui.h"
#include "ui/curses/cmd.h"
#include "ui/curses/eval.h"

void reset_scanner();
void yyerror(char *s, ...);
int yylex(void);
char verr[128];
%}

%code requires {#include <inttypes.h>}

%union {
	int16_t value;
	char *text;
	struct node_t *n;
};

%token <value> VALUE REG YERR
%token <text> NAME TEXT
%token ':' '&' '|' '(' ')'
%token HEX OCT BIN INT UINT
%token IRZ
%token <value> F_QUIT F_MEM F_BIN F_REGS F_SREGS F_RESET F_CYCLE F_HELP F_DASM F_LOAD F_MEMCFG F_MEMMAP F_BRK F_START F_STOP F_STACK F_LOG F_WATCH F_DECODE F_FIND F_CLOCK
%token ADD DEL TEST
%token ON OFF
%type <n> expr lval bitfield basemod

%token BF

%left '='
%left OR
%left AND
%left '|'
%left '^'
%left '&'
%left EQ NEQ
%left GE LE '>' '<'
%left SHR SHL
%left '+' '-'
%left '*'
%left '~' '!'
%left '[' ']'
%nonassoc UMINUS

%%

line:
	statement '\n' { n_discard_stack(); }
	;

statement:
	command
	| exprlist 	{ awtbprint(W_CMD, C_DATA, "\n"); }
	| YERR 		{ yyclearin; yyerror("unknown character: %c", (char) $1); YYABORT; }
	|
	;

exprlist:
	expr { print_node($1); }
	| exprlist ',' expr { print_node($3); }
	;

expr:
	VALUE { $$ = n_val($1); }
	| basemod				{ $$ = $1; }
	| expr bitfield			{ $$ = n_op2(BF, $1, $2); }
	| '-' expr %prec UMINUS	{ $$ = n_op1(UMINUS, $2); }
	| expr '+' expr			{ $$ = n_op2('+', $1, $3); }
	| expr '-' expr			{ $$ = n_op2('-', $1, $3); }
	| expr '*' expr			{ $$ = n_op2('*', $1, $3); }
	| expr '|' expr			{ $$ = n_op2('|', $1, $3); }
	| expr '&' expr			{ $$ = n_op2('&', $1, $3); }
	| expr '^' expr			{ $$ = n_op2('^', $1, $3); }
	| expr SHR expr			{ $$ = n_op2(SHR, $1, $3); }
	| expr SHL expr			{ $$ = n_op2(SHL, $1, $3); }
	| expr OR expr			{ $$ = n_op2(OR, $1, $3); }
	| expr AND expr			{ $$ = n_op2(AND, $1, $3); }
	| expr EQ expr			{ $$ = n_op2(EQ, $1, $3); }
	| expr NEQ expr			{ $$ = n_op2(NEQ, $1, $3); }
	| expr GE expr			{ $$ = n_op2(GE, $1, $3); }
	| expr LE expr			{ $$ = n_op2(LE, $1, $3); }
	| expr '>' expr			{ $$ = n_op2('>', $1, $3); }
	| expr '<' expr			{ $$ = n_op2('<', $1, $3); }
	| '~' expr				{ $$ = n_op1('~', $2); }
	| '!' expr				{ $$ = n_op1('!', $2); }
	| '(' expr ')'			{ $$ = $2; }
	| lval {
		uint16_t data;
		switch ($$->type) {
			case N_MEM:
				if (!ectl_mem_get($$->nb, (uint16_t) $$->val, &data, 1)) {
					yyerror("address [%i:0x%04x] not available, memory not configured", $$->nb, (uint16_t) $$->val);
					YYABORT;
				}
				break;
			case N_VAR:
				if (!var_get($$->var)) {
					yyerror("undefined variable: %s", $$->var);
					YYABORT;
				}
				break;
		}
	}
	| lval '=' expr { $$ = n_ass($1, $3); }
	;

basemod:
	UINT '(' expr ')'	{ $3->base = UINT; $$ = $3; }
	| INT '(' expr ')'	{ $3->base = INT; $$ = $3; }
	| HEX '(' expr ')'	{ $3->base = HEX; $$ = $3; }
	| OCT '(' expr ')'	{ $3->base = OCT; $$ = $3; }
	| BIN '(' expr ')'	{ $3->base = BIN; $$ = $3; }
	;

lval:
	NAME { $$ = n_var($1); }
	| IRZ '[' expr ']' { $$ = n_ireg(N_RZ, n_eval($3)); }
	| REG { $$ = n_reg($1); }
	| '[' expr ']' { $$ = n_mem(n_val(ectl_reg_get(ECTL_REG_Q) * ectl_reg_get(ECTL_REG_NB)), $2); }
	| '[' expr ':' expr ']' { $$ = n_mem($2, $4); }
	;

bitfield:
	'[' VALUE ']'				{ $$ = n_bf($2, $2); }
	| '[' VALUE '-' VALUE ']'	{ $$ = n_bf($2, $4); }
	;

command:
	  F_QUIT 				{ dbg_c_quit(); }
	| F_CYCLE 				{ dbg_c_cycle(); }
	| F_REGS 				{ dbg_c_regs(W_CMD); }
	| F_SREGS 				{ dbg_c_sregs(W_CMD); }
	| F_RESET 				{ dbg_c_reset(); }
	| F_MEMCFG			 	{ dbg_c_memcfg(W_CMD); }
	| F_MEMMAP VALUE VALUE VALUE VALUE { dbg_c_memmap(W_CMD, $2, $3, $4, $5); }
	| F_START 				{ dbg_c_start(); }
	| F_STOP 				{ dbg_c_stop(); }
	| F_STACK 				{ dbg_c_stack(W_CMD, 12); }
	| F_HELP 				{ dbg_c_help(W_CMD, NULL); }
	| F_HELP TEXT			{ dbg_c_help(W_CMD, $2); free($2); }
	| F_DASM				{ dbg_c_dt(W_CMD, ectl_reg_get(ECTL_REG_IC), 1); }
	| F_DASM VALUE			{ dbg_c_dt(W_CMD, ectl_reg_get(ECTL_REG_IC), $2); }
	| F_DASM expr VALUE		{ dbg_c_dt(W_CMD, n_eval($2), $3); }
	| F_MEM expr			{ dbg_c_mem(W_CMD, ectl_reg_get(ECTL_REG_Q) * ectl_reg_get(ECTL_REG_NB), n_eval($2), n_eval($2)+15, 122, 18); }
	| F_MEM expr VALUE		{ dbg_c_mem(W_CMD, ectl_reg_get(ECTL_REG_Q) * ectl_reg_get(ECTL_REG_NB), n_eval($2), n_eval($2)+$3-1, 122, 18); }
	| F_MEM VALUE ':' expr	{ dbg_c_mem(W_CMD, $2, n_eval($4), n_eval($4)+15, 122, 18); }
	| F_MEM VALUE ':' expr VALUE { dbg_c_mem(W_CMD, $2, n_eval($4), n_eval($4)+$5-1, 122, 18); }
	| F_BIN VALUE			{ dbg_c_bin(W_CMD, $2); }
	| F_LOAD TEXT			{ dbg_c_load(W_CMD, $2); }
	| F_BRK					{ dbg_c_brk_list(W_CMD); }
	| F_BRK ADD expr	{
		char expr[128];
		sscanf(input_buf, " brk add %[^\n]", expr);
		dbg_c_brk_add(W_CMD, expr, $3);
		// we need those expressions when evaluating breakpoints later
		n_reset_stack();
	}
	| F_BRK DEL VALUE	{ dbg_c_brk_del(W_CMD, $3); }
	| F_BRK TEST VALUE	{ dbg_c_brk_test(W_CMD, $3); }
	| F_BRK OFF VALUE	{ dbg_c_brk_disable(W_CMD, $3, 1); }
	| F_BRK ON VALUE	{ dbg_c_brk_disable(W_CMD, $3, 0); }
	| F_BRK OFF			{ dbg_c_brk_disable_all(W_CMD, 1); }
	| F_BRK ON			{ dbg_c_brk_disable_all(W_CMD, 0); }
	| F_BRK error
	| F_LOG				{ dbg_c_log_info(W_CMD); }
	| F_LOG OFF 		{ dbg_c_log_disable(W_CMD); }
	| F_LOG ON			{ dbg_c_log_enable(W_CMD); }
	| F_LOG NAME ON		{ dbg_c_log_set_state(W_CMD, $2, 1); }
	| F_LOG NAME OFF	{ dbg_c_log_set_state(W_CMD, $2, 0); }
	| F_LOG error
	| F_WATCH			{ dbg_c_watch_list(W_CMD, 999999); }
	| F_WATCH ADD expr	{
		char expr[128];
		sscanf(input_buf, " watch add %[^\n]", expr);
		dbg_c_watch_add(W_CMD, expr, $3);
		// we need those expressions when evaluating breakpoints later
		n_reset_stack();
	}
	| F_WATCH DEL VALUE	{ dbg_c_watch_del(W_CMD, $3); }
	| F_DECODE			{ dbg_c_list_decoders(W_CMD); }
	| F_DECODE NAME expr{ dbg_c_decode(W_CMD, $2, n_eval($3), 0); }
	| F_FIND VALUE expr	{ dbg_c_find(W_CMD, $2, n_eval($3)); }
	| F_CLOCK ON 		{ ectl_clock_set(1); }
	| F_CLOCK OFF		{ ectl_clock_set(0); }
	;

%%

// -----------------------------------------------------------------------
void yyerror(char *format, ...)
{
	char error[1024];
	va_list ap;
	va_start(ap, format);
	vsprintf(error, format, ap);
	va_end(ap);
	awtbprint(W_CMD, C_ERROR, "Error: %s\n", error);
	n_discard_stack();
	reset_scanner();
}

// vim: tabstop=4 shiftwidth=4 autoindent
