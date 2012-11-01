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
	int16_t value;
	char *text;
	struct node_t *n;
};

%token <value> VALUE REG YERR
%token <text> TEXT FNAME CMDNAME
%token ':' '&' '|' '(' ')'
%token HEX OCT BIN UINT
%token <value> F_QUIT F_CLMEM F_MEM F_REGS F_SREGS F_RESET F_STEP F_HELP F_DASM F_TRANS F_LOAD F_MEMCFG F_BRK
%token B_ADD B_LIST B_DEL
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
		waprintw(WCMD, attr[C_DATA], "%i\n", (uint16_t) n_eval($3));
		n_free($3);
	}
	| HEX '(' expr ')' '\n' {
		waprintw(WCMD, attr[C_DATA], "0x%x\n", n_eval($3));
		n_free($3);
	}
	| OCT '(' expr ')' '\n' {
		waprintw(WCMD, attr[C_DATA], "0%o\n", n_eval($3));
		n_free($3);
	}
	| BIN '(' expr ')' '\n' {
		char *b = int2bin(n_eval($3), 16);
		waprintw(WCMD, attr[C_DATA], "0b%s\n", b);
		n_free($3);
		free(b);
	}
	| expr '\n' {
		waprintw(WCMD, attr[C_DATA], "%i\n", n_eval($1));
		n_free($1);
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
	| TEXT {
		struct debuger_var *v = debuger_get_var($1);
		if (!v) {
			char verr[128];
			sprintf(verr, "undefined variable: %s", $1);
			yyerror(verr);
			YYERROR;
		} else {
			$$ = n_var($1);
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
		uint16_t v = n_eval($2);
		uint16_t *aptr = em400_mem_ptr((SR_NB*SR_Q), v, 0);
		if (!aptr) {
			char verr[128];
			sprintf(verr, "address [%i:%i] not available, memory not configured", (SR_NB*SR_Q), v);
			yyerror(verr);
			YYERROR;
		} else {
			$$ = n_op2('[', n_val(SR_NB*SR_Q), $2);
		}
	}
	| '[' expr ':' expr ']' {
		uint16_t nb = n_eval($2);
		uint16_t v = n_eval($4);
		uint16_t *aptr = em400_mem_ptr(nb, v, 0);
		if (!aptr) {
			char verr[128];
			sprintf(verr, "address [%i:%i] not available, memory not configured", nb, v);
			yyerror(verr);
			YYERROR;
		} else {
			$$ = n_op2('[', $2, $4);
		}
	}
	| lval '=' expr { $$ = n_op2('=', $1, $3); }
	;

lval:
	TEXT { $$ = n_var($1); }
	| REG { $$ = n_reg($1); }
	| '[' expr ']' { $$ = n_op2('[', n_val(SR_NB*SR_Q), $2); }
	| '[' expr ':' expr ']' { $$ = n_op2('[', $2, $4); }
	;

bitfield:
	'[' VALUE ']' { $$ = n_bf($2, $2); }
	| '[' VALUE '-' VALUE ']' { $$ = n_bf($2, $4); }
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
	| f_brk
	;

f_help:
	F_HELP '\n' {
		em400_debuger_c_help(WCMD, NULL);
	}
	| F_HELP CMDNAME '\n'{
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
		em400_debuger_c_dt(WCMD, DMODE_DASM, n_eval($2), $3);
		n_free($2);
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
		em400_debuger_c_dt(WCMD, DMODE_TRANS, n_eval($2), $3);
		n_free($2);
	}
	;

f_mem:
	F_MEM expr '-' expr '\n' {
		em400_debuger_c_mem(WCMD, SR_Q*SR_NB, n_eval($2), n_eval($4));
		n_free($2);
		n_free($4);
	}
	| F_MEM expr ':' expr '-' expr '\n' {
		em400_debuger_c_mem(WCMD, n_eval($2), n_eval($4), n_eval($6));
		n_free($2);
		n_free($4);
		n_free($6);
	}
	;

f_load:
	F_LOAD FNAME '\n' {
		em400_debuger_c_load(WCMD, $2, SR_Q*SR_NB);
	}
	| F_LOAD FNAME VALUE '\n' {
		em400_debuger_c_load(WCMD, $2, $3);
	}
	;

f_brk:
	F_BRK B_LIST '\n' {
		brk_list();
	}
	| F_BRK B_ADD expr '\n' {
		brk_add("test", $3);
	}
	| F_BRK B_DEL VALUE '\n' {
		if (brk_del($3)) {
			waprintw(WCMD, attr[C_ERROR], "No such breakpoint: %i\n", $3);
		}
	}
	;

%%

void yyerror(char *s)
{
    waprintw(WCMD, attr[C_ERROR], "Error: %s\n", s);
}

// vim: tabstop=4
