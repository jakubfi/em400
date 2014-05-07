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

#include <signal.h>
#include <stdio.h>

#include "debugger/awin.h"

volatile int aw_layout_changed;

// -----------------------------------------------------------------------
static void _aw_sigwinch_handler(int signum)
{
	aw_layout_changed = 1;
}

// -----------------------------------------------------------------------
int aw_sigwinch_init()
{
	if (signal(SIGWINCH, _aw_sigwinch_handler) == SIG_ERR) {
		return -1;
	}

	return 0;
}
