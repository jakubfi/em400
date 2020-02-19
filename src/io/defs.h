//  Copyright (c) 2012-2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef IO_DEFS_H
#define IO_DEFS_H

#define IO_MAX_CHAN	16

enum io_dir {
	IO_OU = 0,
	IO_IN = 1,
};

enum io_result {
	// value matters (IN/OU opcodes use it)
	IO_NO = 0,  // no channel, no control unit, or no memory block
	IO_EN = 1,  // not ready (engaged)
	IO_OK = 2,  // OK
	IO_PE = 3,  // data error (parity error)
};

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
