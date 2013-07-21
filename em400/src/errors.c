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

int gerr = E_OK;

struct _em400_errordesc {
	int code;
	char *message;
} em400_errordesc[] = {
	{ E_OK, "no error" },
	{ E_MEM_NO_OS_MEM, "no segments defined for OS memory" },
	{ E_MEM_WRONG_OS_MEM, "wrong number of segments defined for OS memory" },
	{ E_MEM_BAD_SEGMENT_COUNT, "wrong segment count in a module" },
	{ E_MEM_CANNOT_ALLOCATE, "cannot allocate machine memory" },
	{ E_MEM_BLOCK_TOO_SMALL, "address outside configured memory block" },
	{ E_FILE_OPEN, "cannot open file" },
	{ E_FILE_OPERATION, "read/write error" },
	{ E_TIMER_SIGNAL, "cannot set timer handler" },
	{ E_TIMER_CREATE, "cannot create timer" },
	{ E_TIMER_SET, "cannot set timer" },
	{ E_ALLOC, "cannot allocate memory" },
	{ E_DEBUGGER_SIG_RESIZE, "cannot setup window resize handler" },
	{ E_IO_CHAN_UNKNOWN, "unknown channel type" },
	{ E_IO_UNIT_UNKNOWN, "unit incompatibile with channel, or unknown unit type" },
	{ E_IO_CHAN_INIT, "cannot initialize channel" },
	{ E_IO_UNIT_INIT, "cannot initialize unit" },
	{ E_IO_UNIT_INIT_ARGS, "error parsing unit arguments" },
	{ E_LOG_OPEN, "cannot open log file" },
	{ E_AW_INIT, "cannot initialize awin" },
	{ E_UI_INIT, "cannot initialize UI" },
	{ E_UI_SIG_CTRLC, "cannot install debugger Ctrl-C handler" },
	{ E_CFG_OPEN, "cannot open file" },
	{ E_CFG_PARSE, "cannot parse file" },
	{ E_QUIT_OK, "emulation terminated properly" },
	{ E_QUIT_NO_MEM, "OS segmentation fault" },
	{ E_CFG_DEFAULT_LOAD, "sorry" },
	{ E_CF, "error in control field" },
	{ E_DISK_NO, "opening disk that hasn't been created" },
	{ E_DISK_OPEN, "disk already open" },
	{ E_DISK_STAT, "cannot stat() image file" },
	{ E_DISK_SIZE, "wrong image size" },
	{ E_DISK_NO_SECTOR, "cannot find sector" },
	{ E_DISK_RW_SIZE, "wrong number of bytes read/written" },
	{ E_DISK_DATA_TOO_BIG, "data won't fit in sector" },
	{ E_ARG_TOO_MANY, "too many arguments" },
	{ E_ARG_NOT_ENOUGH, "not enough arguments" },
	{ E_ARG_CONVERSION, "argument conversion error" },
	{ E_ARG_FORMAT, "missing or unknown format" },
	{ E_MX_DECODE, "error decoding Multix field" },
	{ E_MX_TRANSMISSION, "Multix transmission error" },
	{ E_MX_CANCEL, "Multix transmission cancelled" },
	{ E_THREAD, "cannot create thread" },

	{ E_UNKNOWN, "unknown error" }
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
