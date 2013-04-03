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

void yyerror(char *);
int yylex(void);
char verr[128];
%}

%union {
	int16_t value;
	char *text;
	struct node_t *n;
};

%token <value> VALUE REG YERR
%token <text> TEXT NAME
%token ':' '&' '|' '(' ')'
%token HEX OCT BIN INT UINT
%token IRZ
%token <value> F_QUIT F_CLMEM F_MEM F_REGS F_SREGS F_RESET F_STEP F_HELP F_DASM F_TRANS F_LOAD F_MEMCFG F_BRK F_RUN F_STACK F_LOG
%token B_ADD B_DEL B_TEST B_DISABLE B_ENABLE
%token L_ON L_OFF L_FILE L_LEVEL
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

statement:
	'\n'
	| command '\n'	{ n_discard_stack(); }
	| exprlist '\n'	{ n_discard_stack(); awprint(W_CMD, C_DATA, "\n"); }
	| YERR '\n'		{ yyclearin; awprint(W_CMD, C_ERROR, "Error: unknown character: %c\n", (char) $1); }
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
	| lval {
		if (!$$->mptr) {
			switch ($$->type) {
				case N_MEM:
					awprint(W_CMD, C_ERROR, "Error: address [%i:0x%04x] not available, memory not configured\n", $$->nb, (uint16_t) $$->val);
					n_discard_stack();
					YYERROR;
					break;
				case N_VAR:
					awprint(W_CMD, C_ERROR, "Error: undefined variable: %s\n", $$->var);
					n_discard_stack();
					YYERROR;
					break;
			}
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

lval:
	TEXT { $$ = n_var($1); }
	| REG { $$ = n_reg($1); }
	| '[' expr ']' { $$ = n_mem(n_val(SR_NB*SR_Q), $2); }
	| '[' expr ':' expr ']' { $$ = n_mem($2, $4); }
	| IRZ '[' expr ']' { $$ = n_ireg(N_RZ, n_eval($3)); }
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
	| F_CLMEM 				{ dbg_c_clmem(); }
	| F_MEMCFG			 	{ dbg_c_memcfg(W_CMD); }
	| F_RUN 				{ dbg_c_run(); }
	| F_STACK 				{ dbg_c_stack(W_CMD, 12); }
	| F_HELP 				{ dbg_c_help(W_CMD, NULL); }
	| F_HELP NAME			{ dbg_c_help(W_CMD, $2); free($2); }
	| F_DASM				{ dbg_c_dt(W_CMD, DMODE_DASM, regs[R_IC], 1); }
	| F_DASM VALUE			{ dbg_c_dt(W_CMD, DMODE_DASM, regs[R_IC], $2); }
	| F_DASM expr VALUE		{ dbg_c_dt(W_CMD, DMODE_DASM, n_eval($2), $3); }
	| F_TRANS				{ dbg_c_dt(W_CMD, DMODE_TRANS, regs[R_IC], 1); }
	| F_TRANS VALUE			{ dbg_c_dt(W_CMD, DMODE_TRANS, regs[R_IC], $2); }
	| F_TRANS expr VALUE	{ dbg_c_dt(W_CMD, DMODE_TRANS, n_eval($2), $3); }
	| F_MEM expr '-' expr	{ dbg_c_mem(W_CMD, SR_Q*SR_NB, n_eval($2), n_eval($4), 122, 18); }
	| F_MEM expr ':' expr '-' expr	{ dbg_c_mem(W_CMD, n_eval($2), n_eval($4), n_eval($6), 122, 18); }
	| F_LOAD NAME			{ dbg_c_load(W_CMD, $2, SR_Q*SR_NB); }
	| F_LOAD NAME VALUE		{ dbg_c_load(W_CMD, $2, $3); }
	| F_BRK					{ dbg_c_brk_list(W_CMD); }
	| F_BRK B_ADD expr	{
		char expr[128];
		sscanf(input_buf, " brk add %[^\n]", expr);
		dbg_c_brk_add(W_CMD, expr, $3);
		n_reset_stack();
	}
	| F_BRK B_DEL VALUE		{ dbg_c_brk_del(W_CMD, $3); }
	| F_BRK B_TEST VALUE	{ dbg_c_brk_test(W_CMD, $3); }
	| F_BRK B_DISABLE VALUE	{ dbg_c_brk_disable(W_CMD, $3, 1); }
	| F_BRK B_ENABLE VALUE	{ dbg_c_brk_disable(W_CMD, $3, 0); }
	| F_LOG					{ dbg_c_log_show(W_CMD); }
	| F_LOG L_ON 			{ log_enable(); }
	| F_LOG L_OFF			{ log_disable(); }
	| F_LOG L_FILE NAME		{ log_shutdown(); log_init($3); }
	| F_LOG L_LEVEL NAME ':' VALUE	{ dbg_c_log_set(W_CMD, $3, $5); }
	;

%%

// -----------------------------------------------------------------------
void yyerror(char *s)
{
	awprint(W_CMD, C_ERROR, "Error: %s\n", s);
}

// vim: tabstop=4
