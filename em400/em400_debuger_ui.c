//	Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc.,
//	51 Franklin Street, Fifth Floor, Boston, MA	02110-1301	USA

#include <ctype.h>
#include <string.h>
#include <ncurses.h>

#define KEY_BACKSPACE2	127
#define KEY_CTRL_W		23

void nc_readline(const char *prompt, char *buffer, int buflen)
{
	int old_curs = curs_set(1);
	int pos = 0;
	int len = 0;
	int x, y;

	attron(COLOR_PAIR(1));
	attron(A_BOLD);
	printw(prompt);
	attroff(A_BOLD);
	attroff(COLOR_PAIR(1));

	getyx(stdscr, y, x);

	while (1) {
		buffer[len] = ' ';
		mvaddnstr(y, x, buffer, len+1);
		move(y, x+pos);
		int c = getch();

		if ((c == KEY_ENTER) || (c == '\n') || (c == '\r')) {
			printw("\n");
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
		}
	}

	buffer[len] = '\0';

	if (old_curs != ERR) {
		curs_set(old_curs);
	}
}

// vim: tabstop=4
