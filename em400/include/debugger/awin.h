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

#ifndef AWIN_H
#define AWIN_H

#include <ncurses.h>
#include <inttypes.h>

#define AWIN struct awin_t
#define ACONT struct acont_t

#define TOP			1
#define BOTTOM		2
#define HORIZONTAL	3
#define LEFT		4
#define RIGHT		8
#define VERTICAL	12

#define FILL		-1
#define DIV2		-2

#define KEY_BACKSPACE2  127
#define KEY_CTRL_W	  23

#define NCCHECK if (aw_output != O_NCURSES) return

struct awin_t {
	char *title;
	int id;
	WINDOW *win;
	WINDOW *bwin;
	int bx, by, bw, bh;
	int ix, iy, iw, ih;
	int max, min, left;
	int border, enabled, scrollable;
	void (*fun)(int wid);
	struct awin_t *next;
};

struct acont_t {
	int calign;
	int walign;
	int x, y;
	int size, min, max, left;
	struct acont_t *next;
	struct awin_t *win;
	struct awin_t *win_last;
};

struct h_entry {
	char *cmd;
	int len;
	struct h_entry *next;
	struct h_entry *prev;
};

extern struct h_entry *aw_history, *aw_history_cur;

enum _output {
	O_STD,
	O_NCURSES
};

extern volatile int aw_layout_changed;
extern int aw_attr[64];

int aw_init(int output);
void aw_shutdown();

int aw_fit(int l_av, int l_max, int l_min, int l_left);
int aw_container_fit(ACONT *c);
int aw_window_fit(ACONT *c, AWIN *w);

void aw_windows_all_delete(AWIN *w);
void aw_containers_all_delete(ACONT *c);

void aw_window_nc_create(AWIN *w);
void aw_window_nc_delete(AWIN *w);

AWIN * aw_window_find(int id);
void aw_window_disable(AWIN *w);
void aw_container_disable(ACONT *c);

void aw_layout_create();
void aw_layout_remove();
void aw_layout_calculate();
void aw_layout_redo();
void aw_layout_refresh();

ACONT * aw_container_add(int calign, int walign, int max, int min, int left);
AWIN * aw_window_add(ACONT *container, int id, char *title, int border, int scrollable, void (*fun)(int wid), int max, int min, int left);

void aw_attr_new(int id, int bgcolor, int fgcolor, int attr);
void aw_clear_win(int id);
void awbinprint(int id, int attr, char *format, uint32_t value, int size);
void awxyprint(int id, int x, int y, int attr, char *format, ...);
#define awprint(id, attr, format, ...) awxyprint(id, -1, -1, attr, format, ##__VA_ARGS__)
void vawprint(int id, int attr, const char *format, va_list ap);
void awfillbg(int id, int attr, char c, int len);

void aw_nc_rl_history_add(char *cmd, int len);
void aw_nc_rl_history_delete(struct h_entry *h);
int aw_nc_readline(int id, int attr, char *prompt, char *buffer, int buflen);
int aw_readline(int id, int attr, char *prompt, char *buffer, int buflen);

#endif

// vim: tabstop=4
