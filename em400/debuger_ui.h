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

#include <ncurses.h>

#define KEY_BACKSPACE2	127
#define KEY_CTRL_W		23

enum {
	WIN_NONE = -1,
	WIN_MEM = 0,
	WIN_DASM = 1,
	WIN_SREGS = 2,
	WIN_REGS = 3,
	WIN_CMD = 4,
	WIN_STATUS = 5,
	WIN_LAST = 6
};

struct e4d_w_struct {
	char *title;
	WINDOW *win;
	WINDOW *bwin;
	int w;
	int h;
	int x;
	int y;
	int box;
	int enabled;
	int scrollable;
	void (*fun)(WINDOW *win);
};

extern int nc_w_changed;

extern struct e4d_w_struct e4d_w[];

int nc_readline(WINDOW *win, const char *prompt, char *buffer, int buflen);
void e400_debuger_w_recalculate();
void e400_debuger_w_destroy(int id);
void e400_debuger_w_create(int id);
void e400_debuger_w_reinit_all();
void e400_debuger_w_redraw_all();
void _em400_debuger_w_resize_sig(int signum, siginfo_t *si, void *ctx);
void em400_debuger_wu_mem(WINDOW * win);
void em400_debuger_wu_dasm(WINDOW * win);
void em400_debuger_wu_regs(WINDOW * win);
void em400_debuger_wu_sregs(WINDOW * win);
void em400_debuger_wu_cmd(WINDOW * win);
void em400_debuger_wu_status(WINDOW * win);
void em400_debuger_wu_none(WINDOW * win);

// vim: tabstop=4
