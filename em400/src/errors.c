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

#include "errors.h"

struct _em400_errordesc {
	int code;
	char *message;
} em400_errordesc[] = {
	{ E_OK, "No error." },
	{ E_MEM_NO_OS_MEM, "No segments defined for OS memory." },
	{ E_MEM_WRONG_OS_MEM, "Wrong number of segments defined for OS memory." },
	{ E_MEM_BAD_SEGMENT_COUNT, "Wrong segment count in a module." },
	{ E_MEM_CANNOT_ALLOCATE, "Cannot allocate machine memory." },
	{ E_MEM_BLOCK_TOO_SMALL, "Address outside configured memory block." },
	{ E_FILE_OPEN, "Cannot open file." },
	{ E_FILE_OPERATION, "Read/write error." },
	{ E_TIMER_SIGNAL, "Cannot set timer handler." },
	{ E_TIMER_CREATE, "Cannot create timer." },
	{ E_TIMER_SET, "Cannot set timer." },
	{ E_ALLOC, "Cannot allocate memory." },
	{ E_DEBUGGER_SIG_RESIZE, "Cannot setup window resize handler." },
	{ E_IO_CHAN_UNKNOWN, "Unknown channel type." },
	{ E_IO_UNIT_UNKNOWN, "Unit incompatibile with channel, or unknown unit type." },
	{ E_IO_CHAN_INIT, "Cannot initialize channel." },
	{ E_IO_UNIT_INIT, "Cannot initialize unit." },
	{ E_IO_UNIT_INIT_ARGS, "Error parsing unit arguments." },
	{ E_LOG_OPEN, "Cannot open log file." },
	{ E_AW_INIT, "Cannot initialize awin." },
	{ E_UI_INIT, "Cannot initialize UI." },
	{ E_UI_SIG_CTRLC, "Cannot install Ctrl-C handler." },
	{ E_CFG_OPEN, "Cannot open file." },
	{ E_CFG_PARSE, "Cannot parse file." },
	{ E_QUIT_OK, "Emulation terminated properly." },
	{ E_QUIT_NO_MEM, "OS segmentation fault." },
	{ E_CFG_DEFAULT_LOAD, "sorry" },
	{ E_CF, "Error in control field" },
	{ E_WINCH_NO, "Opening wincester that hasn't been created" },
	{ E_WINCH_OPEN, "Winchester already open" },
	{ E_WINCH_STAT, "Cannot stat() image file" },
	{ E_WINCH_SIZE, "Wrong image size" },
	{ E_WINCH_NO_SECTOR, "Cannot find sector" },
	{ E_WINCH_RW_SIZE, "Wrong number of bytes read/written" },
	{ E_WINCH_DATA_TOO_BIG, "Data won't fit in sector" },
	{ E_ARG_TOO_MANY, "Too many arguments" },
	{ E_ARG_NOT_ENOUGH, "Not enough arguments" },
	{ E_ARG_CONVERSION, "Argument conversion error" },
	{ E_ARG_FORMAT, "Missing or unknown format" },
	

	{ E_UNKNOWN, "Unknown error." }
};

// -----------------------------------------------------------------------
char * get_error(int e)
{
	struct _em400_errordesc *edict = em400_errordesc;
	while (edict->code != E_UNKNOWN) {
		if (e == edict->code) {
			return edict->message;
		}
		edict++;
	}
	return edict->message;
}

// vim: tabstop=4 shiftwidth=4 autoindent
