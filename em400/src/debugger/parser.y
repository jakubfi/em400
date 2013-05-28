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

#include "registers.h"
#include "memory.h"
#include "utils.h"

#include "debugger/dasm.h"
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "debugger/cmd.h"
#include "debugger/eval.h"
#include "debugger/log.h"

void reset_scanner();
void yyerror(char *s, ...);
int yylex(void);
char verr[128];
%}

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
%token <value> F_QUIT F_MEMCL F_MEM F_REGS F_SREGS F_RESET F_STEP F_HELP F_DASM F_TRANS F_LOAD F_MEMCFG F_BRK F_RUN F_STACK F_LOG F_SCRIPT F_WATCH F_DECODE
%token ADD DEL TEST
%token ON OFF FFILE LEVEL
%type <n> expr lval bitfield basemod act actval

%token BF
%token PVAL
%token ACT WACT RACT

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
	| exprlist 	{ awprint(W_CMD, C_DATA, "\n"); }
	| YERR 		{ yyclearin; yyerror("unknown character: %c", (char) $1); YYABORT; }
	| 
	;

exprlist:
	expr { print_node($1); }
	| exprlist ',' expr { print_node($3); }
	;

expr:
	VALUE { $$ = n_val($1); }
	| basemod
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
	| act					{ $$ = $1; }
	| actval PVAL			{ $$ = n_pval($1); }
	| lval {
		if (!$$->mptr) {
			switch ($$->type) {
				case N_MEM:
					yyerror("address [%i:0x%04x] not available, memory not configured", $$->nb, (uint16_t) $$->val);
					YYABORT;
					break;
				case N_VAR:
					yyerror("undefined variable: %s", $$->var);
					YYABORT;
					break;
			}
		}
	}
	| lval '=' expr { $$ = n_ass($1, $3); }
	;

act:
	ACT actval		{ $$ = n_act(TOUCH_R+TOUCH_W, $2); }
	| RACT actval	{ $$ = n_act(TOUCH_R, $2); }
	| WACT actval	{ $$ = n_act(TOUCH_W, $2); }
	;

actval:
	REG { $$ = n_reg($1); }
	| '[' expr ']' { $$ = n_mem(n_val(SR_NB*SR_Q), $2); }
	| '[' expr ':' expr ']' { $$ = n_mem($2, $4); }
	;

basemod:
	UINT '(' expr ')'	{ $3->base = UINT; $$ = $3; }
	| INT '(' expr ')'	{ $3->base = INT; $$ = $3; }
	| HEX '(' expr ')'	{ $3->base = HEX; $$ = $3; }
	| OCT '(' expr ')'	{ $3->base = OCT; $$ = $3; }
	| BIN '(' expr ')'	{ $3->base = BIN; $$ = $3; }

lval:
	NAME { $$ = n_var($1); }
	| IRZ '[' expr ']' { $$ = n_ireg(N_RZ, n_eval($3)); }
	| actval
	;

bitfield:
	'[' VALUE ']'				{ $$ = n_bf($2, $2); }
	| '[' VALUE '-' VALUE ']'	{ $$ = n_bf($2, $4); }
	;

command:
	  F_QUIT 				{ dbg_c_quit(); }
	| F_STEP 				{ dbg_c_step(); }
	| F_REGS 				{ dbg_c_regs(W_CMD); }
	| F_SREGS 				{ dbg_c_sregs(W_CMD); }
	| F_RESET 				{ dbg_c_reset(); }
	| F_MEMCL 				{ dbg_c_clmem(); }
	| F_MEMCFG			 	{ dbg_c_memcfg(W_CMD); }
	| F_RUN 				{ dbg_c_run(); }
	| F_STACK 				{ dbg_c_stack(W_CMD, 12); }
	| F_HELP 				{ dbg_c_help(W_CMD, NULL); }
	| F_HELP TEXT			{ dbg_c_help(W_CMD, $2); free($2); }
	| F_DASM				{ dbg_c_dt(W_CMD, DMODE_DASM, regs[R_IC], 1); }
	| F_DASM VALUE			{ dbg_c_dt(W_CMD, DMODE_DASM, regs[R_IC], $2); }
	| F_DASM expr VALUE		{ dbg_c_dt(W_CMD, DMODE_DASM, n_eval($2), $3); }
	| F_TRANS				{ dbg_c_dt(W_CMD, DMODE_TRANS, regs[R_IC], 1); }
	| F_TRANS VALUE			{ dbg_c_dt(W_CMD, DMODE_TRANS, regs[R_IC], $2); }
	| F_TRANS expr VALUE	{ dbg_c_dt(W_CMD, DMODE_TRANS, n_eval($2), $3); }
	| F_MEM expr '-' expr	{ dbg_c_mem(W_CMD, SR_Q*SR_NB, n_eval($2), n_eval($4), 122, 18); }
	| F_MEM expr ':' expr '-' expr	{ dbg_c_mem(W_CMD, n_eval($2), n_eval($4), n_eval($6), 122, 18); }
	| F_LOAD TEXT			{ dbg_c_load(W_CMD, $2, SR_Q*SR_NB); }
	| F_LOAD TEXT VALUE		{ dbg_c_load(W_CMD, $2, $3); }
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
	| F_BRK error
	| F_LOG				{ dbg_c_log_show(W_CMD); }
	| F_LOG ON 			{ log_enable(); }
	| F_LOG OFF			{ log_disable(); }
	| F_LOG FFILE TEXT	{ log_shutdown(); log_init($3); }
	| F_LOG NAME ':' VALUE	{ dbg_c_log_set(W_CMD, $2, $4); }
	| F_LOG error
	| F_SCRIPT TEXT		{ dbg_c_script_load(W_CMD, $2); }
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
	| F_DECODE NAME expr	{ dbg_c_decode(W_CMD, $2, n_eval($3), 0); }
	;

%%

// -----------------------------------------------------------------------
void yyerror(char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	awprint(W_CMD, C_ERROR, "Error: ");
	vawprint(W_CMD, C_ERROR, format, ap);
	awprint(W_CMD, C_ERROR, "\n");
	va_end(ap);
	n_discard_stack();
	reset_scanner();
}

// vim: tabstop=4
