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

void yyerror(char *);
int yylex(void);
char verr[128];
struct node_t *enode;
%}

%union {
	int16_t value;
	char *text;
	struct node_t *n;
};

%token <value> VALUE REG YERR
%token <text> TEXT FNAME CMDNAME
%token ':' '&' '|' '(' ')'
%token HEX OCT BIN DEC UINT
%token <value> F_QUIT F_CLMEM F_MEM F_REGS F_SREGS F_RESET F_STEP F_HELP F_DASM F_TRANS F_LOAD F_MEMCFG F_BRK F_RUN F_STACK
%token B_ADD B_LIST B_DEL B_TEST B_DISABLE B_ENABLE
%type <n> expr lval bitfield

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
	| command
	| UINT '(' expr ')' '\n' {
		awprint(W_CMD, C_DATA, "%i\n", (uint16_t) n_eval($3));
		n_discard_stack();
	}
	| DEC '(' expr ')' '\n' {
		awprint(W_CMD, C_DATA, "%i\n", n_eval($3));
		n_discard_stack();
	}
	| HEX '(' expr ')' '\n' {
		awprint(W_CMD, C_DATA, "0x%x\n", n_eval($3));
		n_discard_stack();
	}
	| OCT '(' expr ')' '\n' {
		awprint(W_CMD, C_DATA, "0%o\n", n_eval($3));
		n_discard_stack();
	}
	| BIN '(' expr ')' '\n' {
		awbinprint(W_CMD, C_DATA, "0b................\n", n_eval($3), 16);
		n_discard_stack();
	}
	| expr '\n' {
		awprint(W_CMD, C_DATA, "0x%x\n", n_eval($1));
		n_discard_stack();
	}
	| YERR '\n' {
		yyclearin;
		awprint(W_CMD, C_ERROR, "Error: unknown character: %c\n", (char) $1);
	}
	;

expr:
	VALUE { $$ = n_val($1); }
	| TEXT {
		$$ = n_var($1);
		if (!$$->mptr) {
			enode = $$; YYERROR;
		}
	}
	| REG { $$ = n_reg($1); }
	| expr bitfield { $$ = n_op2('.', $1, $2); }
	| '-' expr %prec UMINUS { $$ = n_op1(UMINUS, $2); }
	| expr '+' expr { $$ = n_op2('+', $1, $3); }
	| expr '-' expr { $$ = n_op2('-', $1, $3); }
	| expr '*' expr { $$ = n_op2('*', $1, $3); }
	| expr '|' expr { $$ = n_op2('|', $1, $3); }
	| expr '&' expr { $$ = n_op2('&', $1, $3); }
	| expr '^' expr { $$ = n_op2('^', $1, $3); }
	| expr SHR expr { $$ = n_op2(SHR, $1, $3); }
	| expr SHL expr { $$ = n_op2(SHL, $1, $3); }
	| expr OR expr { $$ = n_op2(OR, $1, $3); }
	| expr AND expr { $$ = n_op2(AND, $1, $3); }
	| expr EQ expr { $$ = n_op2(EQ, $1, $3); }
	| expr NEQ expr { $$ = n_op2(NEQ, $1, $3); }
	| expr GE expr { $$ = n_op2(GE, $1, $3); }
	| expr LE expr { $$ = n_op2(LE, $1, $3); }
	| expr '>' expr { $$ = n_op2('>', $1, $3); }
	| expr '<' expr { $$ = n_op2('<', $1, $3); }
	| '~' expr { $$ = n_op1('~', $2); }
	| '!' expr { $$ = n_op1('!', $2); }
	| '(' expr ')' { $$ = $2; }
	| '[' expr ']' {
		$$ = n_mem(n_val(SR_NB*SR_Q), $2);
		if (!$$->mptr) {
			enode = $$; YYERROR;
		}
	}
	| '[' expr ':' expr ']' {
		$$ = n_mem($2, $4);
		if (!$$->mptr) {
			enode = $$; YYERROR;
		}
	}
	| lval '=' expr { $$ = n_ass($1, $3); }
	| error '\n' {
		yyclearin;
		if (enode) {
			switch (enode->type) {
				case N_MEM:
					awprint(W_CMD, C_ERROR, "Error: address [%i:0x%04x] not available, memory not configured\n", enode->nb, (uint16_t) enode->val);
					break;
				case N_VAR:
					awprint(W_CMD, C_ERROR, "Error: undefined variable: %s\n", enode->var);
					break;
				default:
					awprint(W_CMD, C_ERROR, "Unknown error on node: type: %i, value: %i, var: %s\n", enode->type, (uint16_t) enode->val, enode->var);
					break;
			}
			enode = NULL;
			n_discard_stack();
		}
		YYERROR;
	}
	;

lval:
	TEXT { $$ = n_var($1); }
	| REG { $$ = n_reg($1); }
	| '[' expr ']' {
		$$ = n_mem(n_val(SR_NB*SR_Q), $2);
		if (!$$->mptr) {
			enode = $$; YYERROR;
		}
	}
	| '[' expr ':' expr ']' {
		$$ = n_mem($2, $4);
		if (!$$->mptr) {
			enode = $$; YYERROR;
		}
	}
	;

bitfield:
	'[' VALUE ']' { $$ = n_bf($2, $2); }
	| '[' VALUE '-' VALUE ']' { $$ = n_bf($2, $4); }
	;

command:
	'\n' {}
	| F_QUIT '\n' {
		dbg_c_quit();
	}
	| F_STEP '\n' {
		dbg_c_step();
	}
	| f_help
	| F_REGS '\n' {
		dbg_c_regs(W_CMD);
	}
	| F_SREGS '\n' {
		dbg_c_sregs(W_CMD);
	}
	| F_RESET '\n' {
		dbg_c_reset();
	}
	| f_dasm
	| f_trans
	| f_mem
	| F_CLMEM '\n' {
		dbg_c_clmem();
	}
	| f_load
	| F_MEMCFG '\n' {
		dbg_c_memcfg(W_CMD);
	}
	| f_brk
	| F_RUN '\n' {
		dbg_c_run();
	}
	| F_STACK '\n' {
		dbg_c_stack(W_CMD, 12);
	}
	;

f_help:
	F_HELP '\n' {
		dbg_c_help(W_CMD, NULL);
	}
	| F_HELP CMDNAME '\n' {
		dbg_c_help(W_CMD, $2);
		free($2);
	}
	;

f_dasm:
	F_DASM '\n' {
		dbg_c_dt(W_CMD, DMODE_DASM, regs[R_IC], 1);
	}
	| F_DASM VALUE '\n' {
		dbg_c_dt(W_CMD, DMODE_DASM, regs[R_IC], $2);
	}
	| F_DASM expr VALUE '\n' {
		dbg_c_dt(W_CMD, DMODE_DASM, n_eval($2), $3);
		n_discard_stack();
	}
	;

f_trans:
	F_TRANS '\n' {
		dbg_c_dt(W_CMD, DMODE_TRANS, regs[R_IC], 1);
	}
	| F_TRANS VALUE '\n' {
		dbg_c_dt(W_CMD, DMODE_TRANS, regs[R_IC], $2);
	}
	| F_TRANS expr VALUE '\n' {
		dbg_c_dt(W_CMD, DMODE_TRANS, n_eval($2), $3);
		n_discard_stack();
	}
	;

f_mem:
	F_MEM expr '-' expr '\n' {
		dbg_c_mem(W_CMD, SR_Q*SR_NB, n_eval($2), n_eval($4), 122, 18);
		n_discard_stack();
	}
	| F_MEM expr ':' expr '-' expr '\n' {
		dbg_c_mem(W_CMD, n_eval($2), n_eval($4), n_eval($6), 122, 18);
		n_discard_stack();
	}
	;

f_load:
	F_LOAD FNAME '\n' {
		dbg_c_load(W_CMD, $2, SR_Q*SR_NB);
	}
	| F_LOAD FNAME VALUE '\n' {
		dbg_c_load(W_CMD, $2, $3);
	}
	;

f_brk:
	F_BRK B_LIST '\n' {
		dbg_c_brk_list(W_CMD);
	}
	| F_BRK B_ADD expr '\n' {
		char expr[128];
		sscanf(input_buf, " brk add %[^\n]", expr);
		dbg_c_brk_add(W_CMD, expr, $3);
		n_reset_stack();
	}
	| F_BRK B_DEL VALUE '\n' {
		dbg_c_brk_del(W_CMD, $3);
	}
	| F_BRK B_TEST VALUE '\n' {
		dbg_c_brk_test(W_CMD, $3);
	}
	| F_BRK B_DISABLE VALUE '\n' {
		dbg_c_brk_disable(W_CMD, $3, 1);
	}
	| F_BRK B_ENABLE VALUE '\n' {
		dbg_c_brk_disable(W_CMD, $3, 0);
	}
	;

%%

void yyerror(char *s)
{
	awprint(W_CMD, C_ERROR, "Error: %s\n", s);
}

// vim: tabstop=4
