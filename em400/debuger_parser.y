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

#include <stdlib.h>
#include <stdarg.h>

#include "registers.h"
#include "memory.h"
#include "utils.h"
#include "dasm.h"
#include "debuger.h"
#include "debuger_ui.h"
#include "debuger_eval.h"

void yyerror(char *);
int yylex(void);
%}

%union {
	int value;
	char *text;
	struct node_t *n;
};

%token <value> VALUE REG YERR
%token <text> TEXT
%token ':' '&' '|' '(' ')' '[' ']'
%token HEX OCT BIN INT UINT
%token <value> F_QUIT F_CLMEM F_MEM F_REGS F_SREGS F_RESET F_STEP F_HELP F_DASM F_TRANS F_LOAD F_MEMCFG
%type <n> expr

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
%nonassoc UMINUS

%%

statement:
	| command
	| UINT '(' expr ')' '\n' {
		struct eval_res *e = n_eval($3);
		if (!e->res) waprintw(WCMD, attr[C_DATA], "%i\n", e->val);
		n_free($3);
		free(e);
	}
	| HEX '(' expr ')' '\n' {
		struct eval_res *e = n_eval($3);
		if (!e->res) waprintw(WCMD, attr[C_DATA], "0x%x\n", e->val);
		n_free($3);
		free(e);
	}
	| OCT '(' expr ')' '\n' {
		struct eval_res *e = n_eval($3);
		if (!e->res) waprintw(WCMD, attr[C_DATA], "0%o\n", e->val);
		n_free($3);
		free(e);
	}
	| BIN '(' expr ')' '\n' {
		struct eval_res *e = n_eval($3);
		char *b = int2bin(e->val, 16);
		if (!e->res) waprintw(WCMD, attr[C_DATA], "0b%s\n", e->val);
		free(b);
		n_free($3);
		free(e);
	}
	| expr '\n' {
		struct eval_res *e = n_eval($1);
		if (!e->res) waprintw(WCMD, attr[C_DATA], "%i\n", e->val);
		n_free($1);
		free(e);
	}
	| TEXT '=' expr '\n' {
		struct eval_res *e = n_eval($3);
		if (!e->res) debuger_set_var($1, e->val);
		n_free($3);
		free(e);
	}
	| REG '=' expr '\n' {
		struct eval_res *e = n_eval($3);
		if (!e->res) Rw($1, e->val);
		n_free($3);
		free(e);
	}
	| '[' expr ']' '=' expr '\n' {
		struct eval_res *e1 = n_eval($2);
		struct eval_res *e2 = n_eval($5);
		if ((!e1->res) && (!e2->res)) MEMw(e1->val, e2->val);
		n_free($2);
		n_free($5);
		free(e1);
		free(e2);
	}
	| '[' expr ':' expr ']' '=' expr '\n' {
		struct eval_res *e1 = n_eval($2);
		struct eval_res *e2 = n_eval($4);
		struct eval_res *e3 = n_eval($7);
		if ((!e1->res) && (!e2->res) && (!e3->res)) MEMBw(e1->val, e2->val, e3->val);
		n_free($2);
		n_free($4);
		n_free($7);
		free(e1);
		free(e2);
		free(e3);
	}
	| YERR {
		char *s_err = malloc(1024);
		sprintf(s_err, "unknown character: %c", (char) $1);
		yyerror(s_err);
		free(s_err);
	}
	;

expr:
	VALUE { $$ = n_val($1); }
	| TEXT { $$ = n_var($1); }
	| REG { $$ = n_reg($1); }
	| '-' expr %prec UMINUS { $$ = n_oper(UMINUS, 1, $2, NULL); }
	| expr '+' expr { $$ = n_oper('+', 2, $1, $3); }
	| expr '-' expr { $$ = n_oper('-', 2, $1, $3); }
	| expr '*' expr { $$ = n_oper('*', 2, $1, $3); }
	| expr '|' expr { $$ = n_oper('|', 2, $1, $3); }
	| expr '&' expr { $$ = n_oper('&', 2, $1, $3); }
	| expr '^' expr { $$ = n_oper('^', 2, $1, $3); }
	| expr SHR expr { $$ = n_oper(SHR, 2, $1, $3); }
	| expr SHL expr { $$ = n_oper(SHL, 2, $1, $3); }
	| expr EQ expr { $$ = n_oper(EQ, 2, $1, $3); }
	| expr NEQ expr { $$ = n_oper(NEQ, 2, $1, $3); }
	| expr GE expr { $$ = n_oper(GE, 2, $1, $3); }
	| expr LE expr { $$ = n_oper(LE, 2, $1, $3); }
	| expr '>' expr { $$ = n_oper('>', 2, $1, $3); }
	| expr '<' expr { $$ = n_oper('<', 2, $1, $3); }
	| '~' expr { $$ = n_oper('~', 1, $2, NULL); }
	| '!' expr { $$ = n_oper('!', 1, $2, NULL); }
	| '(' expr ')' { $$ = $2; }
	| '[' expr ']' { $$ = n_oper('[', 1, $2, NULL); }
	| '[' expr ':' expr ']' { $$ = n_oper('[', 2, $2, $4); }
	;

command:
	'\n' {}
	| F_QUIT'\n' {
		em400_debuger_c_quit();
	}
	| F_STEP '\n' {
		em400_debuger_c_step();
	}
	| f_help
	| F_REGS '\n' {
		em400_debuger_c_regs(WCMD);
	}
	| F_SREGS '\n' {
		em400_debuger_c_sregs(WCMD);
	}
	| F_RESET '\n' {
		em400_debuger_c_reset();
	}
	| f_dasm
	| f_trans
	| f_mem
	| F_CLMEM '\n' {
		em400_debuger_c_clmem();
	}
	| f_load
	| F_MEMCFG '\n' {
		em400_debuger_c_memcfg(WCMD);
	}
	;

f_help:
	F_HELP '\n' {
		em400_debuger_c_help(WCMD, NULL);
	}
	| F_HELP TEXT '\n'{
		em400_debuger_c_help(WCMD, $2);
	}
	;

f_dasm:
	F_DASM '\n' {
		em400_debuger_c_dt(WCMD, DMODE_DASM, R(R_IC), 1);
	}
	| F_DASM VALUE '\n' {
		em400_debuger_c_dt(WCMD, DMODE_DASM, R(R_IC), $2);
	}
	| F_DASM expr VALUE '\n' {
		struct eval_res *e = n_eval($2);
		if (!e->res) em400_debuger_c_dt(WCMD, DMODE_DASM, e->val, $3);
		n_free($2);
		free(e);
	}
	;

f_trans:
	F_TRANS '\n' {
		em400_debuger_c_dt(WCMD, DMODE_TRANS, R(R_IC), 1);
	}
	| F_TRANS VALUE '\n' {
		em400_debuger_c_dt(WCMD, DMODE_TRANS, R(R_IC), $2);
	}
	| F_TRANS expr VALUE '\n' {
		struct eval_res *e = n_eval($2);
		if (!e->res) em400_debuger_c_dt(WCMD, DMODE_TRANS, e->val, $3);
		n_free($2);
		free(e);
	}
	;

f_mem:
	F_MEM expr '-' expr '\n' {
		struct eval_res *e1 = n_eval($2);
		struct eval_res *e2 = n_eval($4);
		if ((!e1->res) && (!e2->res)) em400_debuger_c_mem(WCMD, SR_Q*SR_NB, e1->val, e2->val);
		n_free($2);
		n_free($4);
		free(e1);
		free(e2);
	}
	| F_MEM expr ':' expr '-' expr '\n' {
		struct eval_res *e1 = n_eval($2);
		struct eval_res *e2 = n_eval($4);
		struct eval_res *e3 = n_eval($6);
		if ((!e1->res) && (!e2->res) && (e3->res)) em400_debuger_c_mem(WCMD, e1->val, e2->val, e3->val);
		n_free($2);
		n_free($4);
		n_free($6);
		free(e1);
		free(e2);
		free(e3);
	}
	;

f_load:
	F_LOAD TEXT '\n' {
		em400_debuger_c_load(WCMD, $2, SR_Q*SR_NB);
	}
	| F_LOAD TEXT VALUE '\n' {
		em400_debuger_c_load(WCMD, $2, $3);
	}
	;

%%

void yyerror(char *s)
{
    waprintw(WCMD, attr[C_ERROR], "Error: %s\n", s);
}

// vim: tabstop=4
