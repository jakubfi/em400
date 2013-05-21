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

#ifndef DEBUGGER_UI_H
#define DEBUGGER_UI_H

#include <signal.h>

#include "debugger/awin.h"

enum _ui_attributes {
	C_PROMPT = 1,
	C_LABEL,
	C_DATA,
	C_DATAU,
	C_ILABEL,
	C_IDATA,
	C_ERROR,
	C_READ,
	C_WRITE,
	C_RW
};

enum _ui_windows {
	W_MEM = 0,
	W_STACK,
	W_DASM,
	W_SREGS,
	W_REGS,
	W_CMD,
	W_WATCH,
	W_STATUS,
};

int dbg_ui_init();
void dbg_wu_mem(int wid);
void dbg_wu_dasm(int wid);
void dbg_wu_regs(int wid);
void dbg_wu_sregs(int wid);
void dbg_wu_cmd(int wid);
void dbg_wu_status(int wid);
void dbg_wu_stack(int wid);
void dbg_wu_watch(int wid);
void dbg_wu_none(int wid);

#endif

// vim: tabstop=4
