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
#include <stdatomic.h>
#include <emdas.h>

#include "libem400.h"
#include "ui/ui.h"

#include "ui/curses/awin.h"
#include "ui/curses/debugger.h"
#include "ui/curses/ui.h"
#include "ui/curses/eval.h"
#include "ui_curses_parser.h"

int ui_mode;

static atomic_bool dbg_quit = false;

// store user input here
char input_buf[INPUT_BUF_SIZE];

// breakpoints
struct evlb_t *brk_stack = NULL;
struct evlb_t *brk_top = NULL;

// watches
struct evlb_t *watch_stack = NULL;
struct evlb_t *watch_top = NULL;

// emdas
struct emdas *emd;

int yyparse();
int yylex_destroy();
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(char *yy_str);
void yy_delete_buffer(YY_BUFFER_STATE b);

// -----------------------------------------------------------------------
static void _dbg_sigint_handler(int signum)
{
	signal(SIGINT, _dbg_sigint_handler);
}

// -----------------------------------------------------------------------
int dbg_mem_read(int nb, uint16_t addr, uint16_t *data)
{
	return em400_mem_read(nb, addr, data, 1);
}

// -----------------------------------------------------------------------
void * dbg_init(const char *call_name)
{
	ui_mode = O_NCURSES;

	const char *hfile = "/.em400/history";
	char *home = getenv("HOME");
	char *hist_file = (char *) malloc(strlen(home) + strlen(hfile) + 1);
	sprintf(hist_file, "%s%s", home, hfile);

	if (aw_init(ui_mode, hist_file) != 0) {
		free(hist_file);
		return NULL;
	}

	free(hist_file);

	if ((ui_mode == O_NCURSES) &&  (dbg_ui_init() != 0)) {
		return NULL;
	}

	aw_layout_changed = 1;

	if (signal(SIGINT, _dbg_sigint_handler) == SIG_ERR) {
		return NULL;
	}

	emd = emdas_create(EMD_ISET_MX16, dbg_mem_read);
	if (!emd) {
		return NULL;
	}

	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 4, 4);

	return (void *)-1;
}

// -----------------------------------------------------------------------
void dbg_stop(void *data)
{
	atomic_store_explicit(&dbg_quit, true, memory_order_relaxed);
}

// -----------------------------------------------------------------------
void dbg_shutdown(void *data)
{
	aw_shutdown();
	emdas_destroy(emd);
}

// -----------------------------------------------------------------------
int dbg_parse(char *c)
{
	YY_BUFFER_STATE yb = yy_scan_string(c);
	int res = yyparse();
	yy_delete_buffer(yb);
	yylex_destroy();
	return res;
}

// -----------------------------------------------------------------------
void dbg_loop(void *data)
{
	while (!atomic_load_explicit(&dbg_quit, memory_order_relaxed)) {

		if (aw_layout_changed) {
			awin_tb_scroll_end(W_CMD);
			aw_layout_redo();
		} else {
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
}

// -----------------------------------------------------------------------
struct ui_drv ui_curses = {
	.name = "curses",
	.setup = dbg_init,
	.loop = dbg_loop,
	.stop = dbg_stop,
	.destroy = dbg_shutdown
};

// vim: tabstop=4 shiftwidth=4 autoindent
