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

#ifndef IO_H
#define IO_H

enum em400_io_dir {
	IO_IN,
	IO_OU
};

enum em400_io_result {
	IO_NO = 0,  // no channel, no control unit, or no memory block
	IO_EN = 1,  // not ready
	IO_OK = 2,  // OK
	IO_PE = 3   // data error (parity error?)
};

int em400_io_dispatch(int dir, int is_mem, int chan, int unit, int cmd, uint16_t arg);

#endif

// vim: tabstop=4
