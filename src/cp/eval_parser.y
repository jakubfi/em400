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
#include <stdarg.h>
#include <stdio.h>

#include "cp/eval.h"

void eval_yyerror(struct eval_est **tree, const char *s, ...);
int eval_yylex(void);
extern int eval_yycolumn;

%}

%define parse.error verbose
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
%token ':' '&' '|' '(' ')'
%token IRZ "RZ"
%token ALARM "ALARM"
%token MC "MC"
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
    VALUE					{ $$ = eval_est_val($1); }
	| ALARM					{ $$ = eval_est_alarm(); }
	| MC					{ $$ = eval_est_mc(); }
	| REG					{ $$ = eval_est_reg($1); }
	| FLAG					{ $$ = eval_est_flag($1); }
	| '[' expr ']'			{ $$ = eval_est_mem(eval_est_val(-1), $2); }
	| '[' VALUE ':' expr ']'{ $$ = eval_est_mem(eval_est_val($2), $4); }
	| IRZ '[' VALUE ']'		{ $$ = eval_est_rz($3); }
	| '-' expr %prec UMINUS	{ $$ = eval_est_op(UMINUS, $2, NULL); }
	| expr '+' expr			{ $$ = eval_est_op('+', $1, $3); }
	| expr '-' expr			{ $$ = eval_est_op('-', $1, $3); }
	| expr '*' expr			{ $$ = eval_est_op('*', $1, $3); }
	| expr '/' expr			{ $$ = eval_est_op('/', $1, $3); }
	| expr '^' expr			{ $$ = eval_est_op('^', $1, $3); }
	| expr '|' expr			{ $$ = eval_est_op('|', $1, $3); }
	| expr '&' expr			{ $$ = eval_est_op('&', $1, $3); }
	| expr SHR expr			{ $$ = eval_est_op(SHR, $1, $3); }
	| expr SHL expr			{ $$ = eval_est_op(SHL, $1, $3); }
	| expr OR expr			{ $$ = eval_est_op(OR, $1, $3); }
	| expr AND expr			{ $$ = eval_est_op(AND, $1, $3); }
	| expr EQ expr			{ $$ = eval_est_op(EQ, $1, $3); }
	| expr NEQ expr			{ $$ = eval_est_op(NEQ, $1, $3); }
	| expr GE expr			{ $$ = eval_est_op(GE, $1, $3); }
	| expr LE expr			{ $$ = eval_est_op(LE, $1, $3); }
	| expr '>' expr			{ $$ = eval_est_op('>', $1, $3); }
	| expr '<' expr			{ $$ = eval_est_op('<', $1, $3); }
	| '~' expr				{ $$ = eval_est_op('~', $2, NULL); }
	| '!' expr				{ $$ = eval_est_op('!', $2, NULL); }
	| '(' expr ')'			{ $$ = $2; }
	| TOK_INVALID			{ $$ = NULL; eval_yyerror(tree, "Invalid input: '%s'", $1); yyclearin; YYERROR; }
	| expr TOK_INVALID 		{ $$ = NULL; eval_est_delete($1); eval_yyerror(tree, "Invalid input: '%s'", $2); yyclearin; YYERROR; }
	;
%%

// -----------------------------------------------------------------------
void eval_yyerror(struct eval_est **tree, const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	*tree = eval_est_err(buf);
	eval_yycolumn = 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
