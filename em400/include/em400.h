//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef EM400_H
#define EM400_H

enum em400_states_e {
	STATE_WORK = 0,
	STATE_QUIT = 1,
	STATE_MEM_FAIL = 2,
};

enum em400_console_e {
	CONSOLE_NONE = 0,
	CONSOLE_DEBUGGER,
	CONSOLE_TERMINAL,
};

extern int em400_console;
extern int em400_state;

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
