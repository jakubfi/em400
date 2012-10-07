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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mjc400_trans.h"
#include "mjc400_iset.h"
#include "mjc400_mem.h"
#include "mjc400_regs.h"
#include "mjc400.h"
#include "utils.h"

// -----------------------------------------------------------------------
int mjc400_trans(uint16_t* memptr, char **buf)
{
	return 1;
}

// -----------------------------------------------------------------------
int mjc400_trans_eff_arg(char *buf, uint16_t *memptr)
{
	int n = 0;

	if (_D(*memptr) == 1) {
		n += sprintf(buf+n, "[");
	}

	if (_C(*memptr) == 0) {
		n += sprintf(buf+n, "%i", *(memptr+1));
	} else {
		n += sprintf(buf+n, "r%i", _C(*memptr));
	}

	if (_B(*memptr) != 0) {
		n += sprintf(buf+n, "+r%i", _B(*memptr));
	}

	if (_D(*memptr) == 1) {
		n += sprintf(buf+n, "]");
	}

	return n;
}


