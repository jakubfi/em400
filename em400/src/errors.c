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

#include <stdlib.h>

#include "errors.h"

struct _em400_errordesc {
	int code;
	char *message;
} em400_errordesc[] = {
	{ E_OK, "No error." },
	{ E_MEM_NO_OS_MEM, "No segments defined for OS memory." },
	{ E_MEM_BAD_SEGMENT_COUNT, "Wrong segment count in a module." },
	{ E_MEM_CANNOT_ALLOCATE, "Cannot allocate machine memory." },
	{ E_MEM_BLOCK_TOO_SMALL, "Address outside the block. Block too small." },
	{ E_FILE_OPEN, "Cannot open file." },
	{ E_FILE_OPERATION, "Read/write error." },
	{ E_TIMER_SIGNAL, "Cannot set timer handler." },
	{ E_TIMER_CREATE, "Cannot create timer." },
	{ E_TIMER_SET, "Cannot set timer." },
	{ E_ALLOC, "Cannot allocade memory." },
	{ E_DEBUGGER_SIG_RESIZE, "Cannot setup window resize handler." },
	{ E_IO_INCOMPATIBILE_UNIT, "Unit incompatibile with channel." },
	{ E_IO_DRV_CHAN_BAD, "Bad channel driver definition." },
	{ E_IO_DRV_UNIT_BAD, "Bad unit driver definition." },


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

// vim: tabstop=4
