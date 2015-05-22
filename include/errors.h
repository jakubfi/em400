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

#ifndef ERRORS_H
#define ERRORS_H

extern int gerr;

enum em400_error {
	E_UNKNOWN = -32000,
	E_DEFAULT,
	E_MEM,
	E_MEM_BLOCK_TOO_SMALL,
	E_FILE_OPEN,
	E_FILE_OPERATION,
	E_TIMER_VALUE,
	E_TIMER_SIGNAL,
	E_TIMER_CREATE,
	E_TIMER_SET,
	E_ALLOC,
	E_DEBUGGER_SIG_RESIZE,
	E_IO_CHAN_UNKNOWN,
	E_IO_DEV_UNKNOWN,
	E_IO_CHAN_INIT,
	E_IO_DEV_INIT,
	E_IO_DEV_INIT_ARGS,
	E_LOG_OPEN,
	E_AW_INIT,
	E_UI_INIT,
	E_UI_SIG_CTRLC,
	E_CFG_OPEN,
	E_CFG_PARSE,
	E_QUIT_NO_MEM,
	E_CF,
	E_ARG_NOT_ENOUGH,
	E_ARG_CONVERSION,
	E_ARG_FORMAT,
	E_MX_DECODE,
	E_MX_TRANSMISSION,
	E_MX_CANCEL,
	E_THREAD,
	E_IMAGE,
	E_TERM,
	E_TERM_UNKNOWN,
	E_TERM_CONSOLE_DEBUG,
	E_TERM_CONSOLE_TERM,
	E_NO_OPCODE,
	E_MUTEX_INIT,
	E_SEM_INIT,
	E_SLID_INIT,
	E_LOGGER,
	E_LEVELS,
	E_DASM,
	E_AWP,
	E_MX_EVQ,
	E_MX_IRQQ,
	E_MX_TIMER,

	E_OK = 0,
	E_QUIT_OK
};

char * get_error(int e);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
