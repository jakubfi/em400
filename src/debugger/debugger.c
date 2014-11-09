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
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>

#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "mem/mem.h"

#include "em400.h"
#include "errors.h"
#include "utils.h"

#include "debugger/dasm.h"
#include "debugger/awin.h"
#include "debugger/debugger.h"
#include "debugger/ui.h"
#include "debugger/parser.h"
#include "debugger/eval.h"

#include "cfg.h"

char *script_name = NULL;

int ui_mode;

// debugger flow
int dbg_loop_fin = 0;
volatile int dbg_enter = 1;

// store user input here
char input_buf[INPUT_BUF_SIZE];

// breakpoints
struct evlb_t *brk_stack = NULL;
struct evlb_t *brk_top = NULL;

// watches
struct evlb_t *watch_stack = NULL;
struct evlb_t *watch_top = NULL;

extern int yyparse();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(char *yy_str);
void yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
static void _dbg_sigint_handler(int signum)
{
	signal(SIGINT, _dbg_sigint_handler);
	dbg_enter = 1;
}

// -----------------------------------------------------------------------
int dbg_init(struct cfg_em400_t *cfg)
{
	eprint("Initializing debugger: ");
	// set UI mode
	if (cfg->ui_simple == 1) {
		eprint("simple\n");
		ui_mode = O_STD;
	} else {
		eprint("ncurses\n");
		ui_mode = O_NCURSES;
	}

	const char *hfile = "/.em400/history";
	char *home = getenv("HOME");
	char *hist_file = malloc(strlen(home) + strlen(hfile) + 1);
	sprintf(hist_file, "%s%s", home, hfile);

	if (aw_init(ui_mode, hist_file) != 0) {
		free(hist_file);
		return E_AW_INIT;
	}

	script_name = cfg->script_name;

	free(hist_file);

	if ((ui_mode == O_NCURSES) &&  (dbg_ui_init() != 0)) {
		return E_UI_INIT;
	}

	aw_layout_changed = 1;

	// prepare handler for ctrl-c (break emulation, enter debugger loop)
	if (signal(SIGINT, _dbg_sigint_handler) == SIG_ERR) {
		return E_UI_SIG_CTRLC;
	}

	// register/memory action is none when debugger starts
	dbg_fin_cycle();

	return 0;
}

// -----------------------------------------------------------------------
void dbg_shutdown()
{
	eprint("Shutdown debugger\n");
	aw_shutdown();
}

// -----------------------------------------------------------------------
struct evlb_t * dbg_brk_check()
{
	struct evlb_t *b = brk_stack;
	while (b) {
		if ((!b->disabled) && (n_eval(b->n))) {
			awtbprint(W_CMD, C_LABEL, "Hit breakpoint ");
			awtbprint(W_CMD, C_DATA, "%i", b->nr);
			awtbprint(W_CMD, C_LABEL, ": \"");
			awtbprint(W_CMD, C_DATA, "%s", b->label);
			awtbprint(W_CMD, C_LABEL, "\" (at: ");
			awtbprint(W_CMD, C_DATA, "0x%04x", regs[R_IC]);
			awtbprint(W_CMD, C_LABEL, ", cnt: ");
			awtbprint(W_CMD, C_DATA, "%i", b->value);
			awtbprint(W_CMD, C_LABEL, ")\n");
			b->value++;
			return b;
		}
		b = b->next;
	}
	return NULL;
}

// -----------------------------------------------------------------------
void dbg_fin_cycle()
{
	// we're leaving debugger, clear memory/register traces
}

// -----------------------------------------------------------------------
int dbg_parse(char *c)
{
	YY_BUFFER_STATE yb = yy_scan_string(c);
	int res = yyparse();
	yy_delete_buffer(yb);
	return res;
}

// -----------------------------------------------------------------------
int read_script(char *filename)
{
	FILE *sf = fopen(filename, "r");
	if (!sf) {
		return INT_MIN;;
	}

	char *b = input_buf;
	int lines = 0;
	int linebeg = 1;
	int comment = 0;

	while (1) {
		int c = fread(b, 1, 1, sf);

		if (c <= 0) {
			break;
		}

		// skip leading blanks
		if (((*b == ' ') || (*b == '\t')) && (linebeg)) {
		// comment start
		} else if (*b == ';') {
			comment = 1;
		// skip comments
		} else if (comment && (*b != '\n')) {
		// EOL, do parse
		} else if (*b == '\n') {
			linebeg = 1;
			comment = 0;
			*(b+1) = '\0';
			*b = '\0';
			if (b != input_buf) {
				awtbprint(W_CMD, C_LABEL, "%s ", input_buf);
				awtbprint(W_CMD, C_PROMPT, "-> ");
				*b = '\n';
				int res = dbg_parse(input_buf);
				if (res != 0) {
					return -(lines+1);
				}
				b = input_buf;
				lines++;
			}
		// next char
		} else {
			linebeg = 0;
			b++;
		}
	}

	fclose(sf);
	return lines;
}

// -----------------------------------------------------------------------
void dbg_step()
{
	struct evlb_t *bhit = NULL;

	if ((!dbg_enter) && (!(bhit=dbg_brk_check()))) {
		dbg_fin_cycle();
		return;
	}

	dbg_loop_fin = 0;

	while (!dbg_loop_fin) {
		if (aw_layout_changed) {
			awin_tb_scroll_end(W_CMD);
			aw_layout_redo();
		} else {
			aw_layout_refresh();
		}

		if (script_name) {
			int sr = read_script(script_name);
			if (sr == INT_MIN) {
				awtbprint(W_CMD, C_ERROR, "Cannot open script file: %s\n", script_name);
			} else if (sr<0) {
				awtbprint(W_CMD, C_ERROR, "Error at line: %i\n", -sr);
			} else {
				awtbprint(W_CMD, C_LABEL, "Read %i line(s)\n", sr);
			}
			free(script_name);
			script_name = NULL;
			aw_layout_refresh();
		}

		int res = aw_readline(W_CMD, C_PROMPT, "em400> ", C_INPUT, input_buf, INPUT_BUF_SIZE);

		if ((res == KEY_ENTER) && (*input_buf)) {
			if (ui_mode == O_NCURSES) {
				awtbprint(W_CMD, C_PROMPT, "%s", "em400> ");
				awtbprint(W_CMD, C_LABEL, "%s\n", input_buf);
				awin_tb_scroll_end(W_CMD);
			}
			int len = strlen(input_buf);
			input_buf[len] = '\n';
			input_buf[len+1] = '\0';
			dbg_parse(input_buf);
		} else if (res == KEY_NPAGE) {
			awin_tb_scroll(W_CMD, 10);
		} else if (res == KEY_PPAGE) {
			awin_tb_scroll(W_CMD, -10);
		}

	}
	dbg_fin_cycle();
}


// vim: tabstop=4 shiftwidth=4 autoindent
