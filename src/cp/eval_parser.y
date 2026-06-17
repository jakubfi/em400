%{
//  Copyright (c) 2012-2026 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdio.h>

#include "cp/eval.h"

void eval_yyerror(struct eval_est **tree, const char *s, ...);
int eval_yylex(void);
extern int eval_yycolumn;

#define MK(dst, ctor) \
    (dst) = (ctor); \
	if (!(dst)) { \
	    eval_yyerror(tree, "out of memory"); YYABORT; \
	}

%}

%define parse.error custom
%locations

%parse-param {struct eval_est **tree}

%union {
	int value;
	char *str;
	struct eval_est *n;
};

%token <value> REG "register"
%token <value> FLAG "CPU flag"
%token <value> VALUE "value"
%token <str> TOK_INVALID "error"
%token ':' '&' '|' '(' ')' '@'
%token IRZ "RZ"
%token ALARM "ALARM"
%token MC "MC"
%token NB "NB"
%token Q "Q"
%token BS "BS"
%token RM "RM"
%type <n> expr "expression"
%token OR "||"
%token AND "&&"
%token EQ "=="
%token NEQ "!="
%token GE ">="
%token LE "<="
%token SHR ">>"
%token SHL "<<"
%token UMINUS "unary minus"

%left '='
%right OR
%right AND
%left '|'
%left '^'
%left '&'
%left EQ
%left NEQ
%left GE LE '>' '<'
%left SHR
%left SHL
%left '+' '-'
%left '*' '/'
%left '~' '!'
%left '[' ']'
%left TOK_INVALID
%nonassoc UMINUS

%destructor { eval_est_delete($$); } <n>
%destructor { free($$); } <str>

%%

statement:
    expr	 		{ *tree = $1; eval_yycolumn = 0; }
	;

expr:
    VALUE					{ MK($$, eval_est_leaf(EVAL_AST_N_VAL, $1)); }
	| ALARM					{ MK($$, eval_est_leaf(EVAL_AST_N_ALARM, 0)); }
	| MC					{ MK($$, eval_est_leaf(EVAL_AST_N_MC, 0)); }
	| NB					{ MK($$, eval_est_leaf(EVAL_AST_N_NB, 0)); }
	| Q						{ MK($$, eval_est_leaf(EVAL_AST_N_Q, 0)); }
	| BS					{ MK($$, eval_est_leaf(EVAL_AST_N_BS, 0)); }
	| RM					{ MK($$, eval_est_leaf(EVAL_AST_N_RM, 0)); }
	| REG					{ MK($$, eval_est_leaf(EVAL_AST_N_REG, $1)); }
	| FLAG					{ MK($$, eval_est_leaf(EVAL_AST_N_FLAG, $1)); }
	| '[' expr ']'			{ MK($$, eval_est_mem(eval_est_leaf(EVAL_AST_N_VAL, -1), $2)); }
	| '[' VALUE ':' expr ']'{ MK($$, eval_est_mem(eval_est_leaf(EVAL_AST_N_VAL, $2), $4)); }
	| '@' VALUE ':' VALUE	{ MK($$, eval_est_loc($2, $4)); }
	| IRZ					{ MK($$, eval_est_leaf(EVAL_AST_N_RZ, 0)); }
	| IRZ '[' VALUE ']'		{ MK($$, eval_est_leaf(EVAL_AST_N_RZ_BIT, $3)); }
	| '-' expr %prec UMINUS	{ MK($$, eval_est_op(UMINUS, $2, NULL)); }
	| expr '+' expr			{ MK($$, eval_est_op('+', $1, $3)); }
	| expr '-' expr			{ MK($$, eval_est_op('-', $1, $3)); }
	| expr '*' expr			{ MK($$, eval_est_op('*', $1, $3)); }
	| expr '/' expr			{ MK($$, eval_est_op('/', $1, $3)); }
	| expr '^' expr			{ MK($$, eval_est_op('^', $1, $3)); }
	| expr '|' expr			{ MK($$, eval_est_op('|', $1, $3)); }
	| expr '&' expr			{ MK($$, eval_est_op('&', $1, $3)); }
	| expr SHR expr			{ MK($$, eval_est_op(SHR, $1, $3)); }
	| expr SHL expr			{ MK($$, eval_est_op(SHL, $1, $3)); }
	| expr OR expr			{ MK($$, eval_est_op(OR, $1, $3)); }
	| expr AND expr			{ MK($$, eval_est_op(AND, $1, $3)); }
	| expr EQ expr			{ MK($$, eval_est_op(EQ, $1, $3)); }
	| expr NEQ expr			{ MK($$, eval_est_op(NEQ, $1, $3)); }
	| expr GE expr			{ MK($$, eval_est_op(GE, $1, $3)); }
	| expr LE expr			{ MK($$, eval_est_op(LE, $1, $3)); }
	| expr '>' expr			{ MK($$, eval_est_op('>', $1, $3)); }
	| expr '<' expr			{ MK($$, eval_est_op('<', $1, $3)); }
	| '~' expr				{ MK($$, eval_est_op('~', $2, NULL)); }
	| '!' expr				{ MK($$, eval_est_op('!', $2, NULL)); }
	| '(' expr ')'			{ $$ = $2; }
	| TOK_INVALID			{ $$ = NULL; eval_yyerror(tree, "Invalid input: '%s'", $1); yyclearin; YYERROR; }
	| expr TOK_INVALID 		{ $$ = NULL; eval_est_delete($1); eval_yyerror(tree, "Invalid input: '%s'", $2); yyclearin; YYERROR; }
	;
%%

// -----------------------------------------------------------------------
int yyreport_syntax_error(const yypcontext_t *ctx, struct eval_est **tree)
{
	yysymbol_kind_t tok = yypcontext_token(ctx);
	if (tok == YYSYMBOL_YYEOF) {
		eval_yyerror(tree, "incomplete expression");
	} else if (tok != YYSYMBOL_YYEMPTY) {
		eval_yyerror(tree, "unexpected %s", yysymbol_name(tok));
	} else {
		eval_yyerror(tree, "syntax error");
	}
	return 0;
}

// -----------------------------------------------------------------------
void eval_yyerror(struct eval_est **tree, const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap); // bounded: TOK_INVALID text can be long
	va_end(ap);
	*tree = eval_est_err(buf);
	eval_yycolumn = 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
