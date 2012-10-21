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

#include <ctype.h>
#include <string.h>
#include <ncurses.h>
#include <stdio.h>
#include <signal.h>

#include "dasm.h"
#include "utils.h"
#include "debuger.h"
#include "debuger_ui.h"

int nc_w_changed = 0;

struct e4d_w_struct e4d_w[] = {
	{ "Memory",				NULL, NULL, 0, 0, 0, 0, 1, 1, 0, em400_debuger_wu_mem },
	{ "ASM",				NULL, NULL, 0, 0, 0, 0, 1, 1, 0, em400_debuger_wu_dasm },
	{ "System registers",	NULL, NULL, 0, 0, 0, 0, 1, 1, 0, em400_debuger_wu_sregs },
	{ "User registers",		NULL, NULL, 0, 0, 0, 0, 1, 1, 0, em400_debuger_wu_regs },
	{ "Commandline",		NULL, NULL, 0, 0, 0, 0, 1, 1, 1, em400_debuger_wu_cmd },
	{ "Status",				NULL, NULL, 0, 0, 0, 0, 0, 1, 0, em400_debuger_wu_status },
	{ "",					NULL, NULL, 0, 0, 0, 0, 0, 0, 0, em400_debuger_wu_none }
};

int attr[C_MAX] = {0};

// -----------------------------------------------------------------------
void em400_debuger_ui_init()
{
	// command line promt
	init_pair(C_PROMPT, COLOR_YELLOW, COLOR_BLACK);
	attr[C_PROMPT] = COLOR_PAIR(C_PROMPT) | A_BOLD;

	// status line label
	init_pair(C_ILABEL, COLOR_BLACK, COLOR_WHITE);
	attr[C_ILABEL] = COLOR_PAIR(C_ILABEL);

	// status line text
	//init_pair(C_IDATA, COLOR_BLUE, COLOR_WHITE);
	//attr[C_IDATA] = COLOR_PAIR(C_IDATA);
	attr[C_IDATA] = COLOR_PAIR(C_ILABEL) | A_BOLD;

	// standard label
	init_pair(C_LABEL, COLOR_WHITE, COLOR_BLACK);
	attr[C_LABEL] = COLOR_PAIR(C_LABEL);

	// standard data
	attr[C_DATA] = COLOR_PAIR(C_LABEL) | A_BOLD;

}

// -----------------------------------------------------------------------
int nc_readline(WINDOW *win, const char *prompt, char *buffer, int buflen)
{
	int old_curs = curs_set(1);
	int pos = 0;
	int len = 0;
	int x, y;
	int c;

	keypad(win, TRUE);
	getyx(win, y, x);

	wattron(win, attr[C_PROMPT]);
	mvwprintw(win, y, 0, prompt);
	wattroff(win, attr[C_PROMPT]);

	getyx(win, y, x);

	while (1) {
		buffer[len] = ' ';
		mvwaddnstr(win, y, x, buffer, len+1);
		wmove(win, y, x+pos);
		c = wgetch(win);

		if ((c == KEY_ENTER) || (c == '\n') || (c == '\r')) {
			wprintw(win, "\n");
			break;
		} else if (isprint(c)) {
			if (pos < buflen-1) {
				memmove(buffer+pos+1, buffer+pos, len-pos);
				buffer[pos++] = c;
				len += 1;
			}
		} else if (c == KEY_LEFT) {
			if (pos > 0) {
				pos -= 1;
			}
		} else if (c == KEY_RIGHT) {
			if (pos < len) {
				pos += 1;
			}
		} else if (c == KEY_HOME) {
			pos = 0;
		} else if (c == KEY_END) {
			pos = len;
		} else if ((c == KEY_BACKSPACE) || (c == KEY_BACKSPACE2)) {
			if (pos > 0) {
				memmove(buffer+pos-1, buffer+pos, len-pos);
				pos -= 1;
				len -= 1;
			}
		} else if (c == KEY_DC) {
			if (pos < len) {
				memmove(buffer+pos, buffer+pos+1, len-pos-1);
				len -= 1;
			}
		} else {
			break;
		}
	}

	buffer[len] = '\0';

	if (old_curs != ERR) {
		curs_set(old_curs);
	}
	return c;
}

// -----------------------------------------------------------------------
void e400_debuger_w_recalculate()
{
	// these are predefined
	e4d_w[WIN_STATUS].h = 1;
	e4d_w[WIN_DASM].w = 30;
	e4d_w[WIN_SREGS].h = 10;
	e4d_w[WIN_REGS].h = 10;

	// STATUS: always there, at the bottom
	e4d_w[WIN_STATUS].enabled = 1;
	e4d_w[WIN_STATUS].h = 1;
	e4d_w[WIN_STATUS].w = COLS;

	// DASM: width 28, full height
	e4d_w[WIN_DASM].enabled = 1;
	e4d_w[WIN_DASM].w = 30;
	e4d_w[WIN_DASM].h = LINES - e4d_w[WIN_STATUS].h;

	// MEM: at the top, fill horizontaly
	e4d_w[WIN_MEM].enabled = 1;
	e4d_w[WIN_MEM].w = COLS - e4d_w[WIN_DASM].w;
	e4d_w[WIN_MEM].h = (LINES - e4d_w[WIN_REGS].h - e4d_w[WIN_STATUS].h) / 2;
	// MEM: disable if <6
	if (e4d_w[WIN_MEM].h < 6) {
		e4d_w[WIN_MEM].enabled = 0;
		e4d_w[WIN_MEM].w = 0;
		e4d_w[WIN_MEM].h = 0;
	}
	if (e4d_w[WIN_MEM].h > 20 ) {
		e4d_w[WIN_MEM].h = 20;
	}

	// DASM: if terminal is too narrow, disable DASM
	if (COLS - e4d_w[WIN_DASM].w < 30) {
		e4d_w[WIN_DASM].enabled = 0;
		e4d_w[WIN_DASM].w = 0;
		e4d_w[WIN_DASM].w = 0;
	}

	// SREGS: height 10 sharp, width variable
	e4d_w[WIN_SREGS].enabled = 1;
	e4d_w[WIN_SREGS].h = 10;
	e4d_w[WIN_SREGS].w = (COLS - e4d_w[WIN_DASM].w) / 2;
	// SREGS: if width<58, disable
	if (e4d_w[WIN_SREGS].w < 58) {
		e4d_w[WIN_SREGS].enabled = 0;
		e4d_w[WIN_SREGS].w = 0;
		e4d_w[WIN_SREGS].h = 0;
	}

	// REGS: height 10 sharp, width variable
	e4d_w[WIN_REGS].enabled = 1;
	e4d_w[WIN_REGS].h = 10;
	e4d_w[WIN_REGS].w = COLS - e4d_w[WIN_DASM].w - e4d_w[WIN_SREGS].w;
	// REGS: if width<58, disable
	if ((e4d_w[WIN_REGS].w < 58) || (LINES - e4d_w[WIN_REGS].h - e4d_w[WIN_STATUS].h < 4)) {
		e4d_w[WIN_REGS].enabled = 0;
		e4d_w[WIN_REGS].w = 0;
		e4d_w[WIN_REGS].h = 0;
	}

	// CMD: 
	e4d_w[WIN_CMD].enabled = 1;
	e4d_w[WIN_CMD].w = COLS - e4d_w[WIN_DASM].w;
	e4d_w[WIN_CMD].h = LINES - e4d_w[WIN_REGS].h - e4d_w[WIN_STATUS].h - e4d_w[WIN_MEM].h;

	// POSITIONS

	e4d_w[WIN_STATUS].x = 0;
	e4d_w[WIN_STATUS].y = LINES-1;

	e4d_w[WIN_DASM].x = COLS - e4d_w[WIN_DASM].w;
	e4d_w[WIN_DASM].y = 0;

	e4d_w[WIN_REGS].x = 0;
	e4d_w[WIN_REGS].y = e4d_w[WIN_MEM].h;

	e4d_w[WIN_SREGS].x = e4d_w[WIN_REGS].w;
	e4d_w[WIN_SREGS].y = e4d_w[WIN_MEM].h;

	e4d_w[WIN_CMD].x = 0;
	e4d_w[WIN_CMD].y = e4d_w[WIN_MEM].h + e4d_w[WIN_REGS].h;

	e4d_w[WIN_MEM].x = 0;
	e4d_w[WIN_MEM].y = 0;
}

// -----------------------------------------------------------------------
void e400_debuger_w_destroy(int id)
{
	if ((!e4d_w[id].enabled) || (!e4d_w[id].win)) {
		return;
	}

	wrefresh(e4d_w[id].win);
	delwin(e4d_w[id].win);

	if (e4d_w[id].box) {
		wborder(e4d_w[id].bwin, ' ', ' ', ' ',' ',' ',' ',' ',' ');
		wrefresh(e4d_w[id].bwin);
		delwin(e4d_w[id].bwin);
	}
}

// -----------------------------------------------------------------------
void e400_debuger_w_create(int id)
{
	if (!e4d_w[id].enabled) {
		return;
	}

	if (e4d_w[id].box) {
		e4d_w[id].bwin = newwin(e4d_w[id].h, e4d_w[id].w, e4d_w[id].y, e4d_w[id].x);
		box(e4d_w[id].bwin, 0, 0);
		mvwprintw(e4d_w[id].bwin, 0, 3, "[ %s ]", e4d_w[id].title);
		wrefresh(e4d_w[id].bwin);
		e4d_w[id].win = newwin(e4d_w[id].h-2, e4d_w[id].w-2, e4d_w[id].y+1, e4d_w[id].x+1);
	} else {
		e4d_w[id].win = newwin(e4d_w[id].h, e4d_w[id].w, e4d_w[id].y, e4d_w[id].x);
	}

	if (e4d_w[id].scrollable) {
		scrollok(e4d_w[id].win, TRUE);
	}

	e4d_w[id].fun(e4d_w[id].win);
	wrefresh(e4d_w[id].win);
}

// -----------------------------------------------------------------------
void e400_debuger_w_redraw_all()
{
	for (int i=WIN_MEM ; i<WIN_LAST ; i++) {
		if (e4d_w[i].enabled) {
			if (!e4d_w[i].scrollable) {
				wmove(e4d_w[i].win, 0, 0);
			}
			e4d_w[i].fun(e4d_w[i].win);
			wrefresh(e4d_w[i].win);
		}
	}
}

// -----------------------------------------------------------------------
void e400_debuger_w_reinit_all()
{
	int i;

	endwin();

	for (i=WIN_MEM ; i<WIN_LAST ; i++) {
		e400_debuger_w_destroy(i);
	}

	e400_debuger_w_recalculate();

	for (i=WIN_MEM ; i<WIN_LAST ; i++) {
		e400_debuger_w_create(i);
	}
}

// -----------------------------------------------------------------------
void _em400_debuger_w_resize_sig(int signum, siginfo_t *si, void *ctx)
{
	nc_w_changed = 1;
}

// -----------------------------------------------------------------------
void em400_debuger_wu_mem(WINDOW *win)
{
	__em400_debuger_dump_mem(e4d_w[WIN_MEM].win, 0, 0, 0xff);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_dasm(WINDOW *win)
{
	int offset = (e4d_w[WIN_DASM].h-2) / 3;
	int start;
	if (IC < offset) {
		start = 0;
	} else {
		start = IC - offset;
	}
	em400_debuger_dt(win, DMODE_DASM, start, e4d_w[WIN_DASM].h-2);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_regs(WINDOW *win)
{
	wprintw(win, "    hex    oct    dec    bin              ch R40\n");
	for (int i=1 ; i<=7 ; i++) {
		char *b = int2bin(R[i], 16);
		char *r = int2r40(R[i]);
		char *c = int2chars(R[i]);

		wprintw(win, "R%i: ", i);
		wattron(win, A_BOLD);
		wprintw(win, "0x%04x %6o %6i %s %s %s\n", R[i], R[i], (int16_t)R[i], b, c, r);
		wattroff(win, A_BOLD);
		free(c);
		free(r);
		free(b);
	}
}

// -----------------------------------------------------------------------
void em400_debuger_wu_sregs(WINDOW *win)
{
	char *ir = int2bin(IR>>10, 6);
	int d = (IR>>9) & 1;
	char *a = int2bin(IR>>6, 3);
	char *b = int2bin(IR>>3, 3);
	char *c = int2bin(IR, 3);

	char *rm = int2bin(SR>>6, 10);
	int q = (SR>>5) & 1;
	int s = (SR>>6) & 1;
	char *nb = int2bin(SR, 4);

	char *i1 = int2bin(RZ>>27, 5);
	char *i2 = int2bin(RZ>>20, 7);
	char *i3 = int2bin(RZ>>18, 2);
	char *i4 = int2bin(RZ>>16, 2);
	char *i5 = int2bin(RZ>>10, 6);
	char *i6 = int2bin(RZ>>4, 6);
	char *i7 = int2bin(RZ, 4);

	char *sf = int2bin(R[0]>>8, 8);
	char *uf = int2bin(R[0], 8);

	wprintw(win, "            OPCODE D A   B   C\n");
	wprintw(win, "IR: ");
	wattron(win, A_BOLD);
	wprintw(win, "0x%04x  %s %i %s %s %s\n", IR, ir, d, a, b, c);
	wattroff(win, A_BOLD);
	wprintw(win, "            RM         Q s NB\n");
	wprintw(win, "SR: ");
	wattron(win, A_BOLD);
	wprintw(win, "0x%04x  %s %i %i %s\n", SR, rm, q, s, nb);
	wattroff(win, A_BOLD);
	wprintw(win, "                ZPMCZ TIFFFFx 01 23 456789 abcdef OCSS\n");
	wprintw(win, "RZ: ");
	wattron(win, A_BOLD);
	wprintw(win, "0x%08x  %s %s %s %s %s %s %s\n", RZ, i1, i2, i3, i4, i5, i6, i7);
	wattroff(win, A_BOLD);
	wprintw(win, "            ZMVCLEGY Xuser\n");
	wprintw(win, "R0: ");
	wattron(win, A_BOLD);
	wprintw(win, "0x%04x  %s %s\n", R[0], sf, uf);
	wattroff(win, A_BOLD);

	free(uf);
	free(sf);

	free(i1);
	free(i2);
	free(i3);
	free(i4);
	free(i5);
	free(i6);
	free(i7);

	free(rm);
	free(nb);

	free(ir);
	free(a);
	free(b);
	free(c);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_cmd(WINDOW *win)
{
}

// -----------------------------------------------------------------------
void em400_debuger_wu_status(WINDOW *win)
{
	int x,y;
	wattron(win, attr[C_ILABEL]);
	whline(win, ' ', COLS);
	getyx(win, y, x);
	wmove(win, y, 0);
	wattrset(win, attr[C_ILABEL]); wprintw(win, "  Q:");
	wattrset(win, attr[C_IDATA]); wprintw(win, "%i", SR_Q);
	wattrset(win, attr[C_ILABEL]); wprintw(win, "  NB:");
	wattrset(win, attr[C_IDATA]); wprintw(win, "%i", SR_NB);
	wattrset(win, attr[C_ILABEL]); wprintw(win, "  IC:");
	wattrset(win, attr[C_IDATA]); wprintw(win, "0x%04x", IC);
	wattrset(win, attr[C_ILABEL]); wprintw(win, "  P:");
	wattrset(win, attr[C_IDATA]); wprintw(win, "%i", P);
	wattrset(win, attr[C_ILABEL]); wprintw(win, "  MOD:");
	wattrset(win, attr[C_IDATA]); wprintw(win, "%06i (0x%04x)", MOD, MOD);
	wattrset(win, attr[C_ILABEL]); wprintw(win, "  MODcnt:");
	wattrset(win, attr[C_IDATA]); wprintw(win, "%i", MODcnt);
	wattrset(win, attr[C_ILABEL]); wprintw(win, "  ZC17:");
	wattrset(win, attr[C_IDATA]); wprintw(win, "%i", ZC17);
	wattroff(win, attr[C_IDATA]);
}

// -----------------------------------------------------------------------
void em400_debuger_wu_none(WINDOW *win)
{
}


// vim: tabstop=4
