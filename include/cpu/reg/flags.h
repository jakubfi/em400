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

#ifndef REG_FLAGS_H
#define REG_FLAGS_H

// -----------------------------------------------------------------------
// Flags in R0
// -----------------------------------------------------------------------

#define FL_Z	0b1000000000000000
#define FL_M	0b0100000000000000
#define FL_V	0b0010000000000000
#define FL_C	0b0001000000000000
#define FL_L	0b0000100000000000
#define FL_E	0b0000010000000000
#define FL_G	0b0000001000000000
#define FL_Y	0b0000000100000000
#define FL_X	0b0000000010000000

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
