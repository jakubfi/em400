//  Copyright (c) 2013-2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_CF_H
#define MX_CF_H

struct mx_line;

// physical line direction
enum mx_pline_dir {
	MX_DIR_WINCH		= 0b000,
	MX_DIR_INPUT		= 0b010,
	MX_DIR_OUTPUT		= 0b100,
	MX_DIR_HALF_DUPLEX	= 0b110,
	MX_DIR_FULL_DUPLEX	= 0b111
};

// physical line types
enum mx_pline_type {
	MX_PHY_USART_ASYNC	= 0,
	MX_PHY_8255			= 1,
	MX_PHY_USART_SYNC	= 2,
	MX_PHY_WINCHESTER	= 3,
	MX_PHY_MTAPE		= 4,
	MX_PHY_FLOPPY		= 5,
	MX_PHY_MAX
};

// "set configuration" return field values
enum mx_setconf_err {
	MX_SC_E_OK				= -1, // OK
	MX_SC_E_CONFSET			= 0 << 8,  // configuration already set
	MX_SC_E_NUMLINES		= 1 << 8,  // wrong number of physical or logical lines
	MX_SC_E_DEVTYPE			= 2 << 8,  // unknown device type in physical line description
	MX_SC_E_DIR				= 3 << 8,  // unknown transmission direction
	MX_SC_E_PHY_INCOMPLETE	= 4 << 8,  // incomplete physical line description
	MX_SC_E_PROTO_MISSING	= 5 << 8,  // missing protocol
	MX_SC_E_PHY_UNUSED		= 6 << 8,  // physical line is not used
	MX_SC_E_DIR_MISMATCH	= 7 << 8,  // device vs. protocol transmission dricetion mismatch
	MX_SC_E_PHY_BUSY		= 8 << 8,  // physical line is busy
	MX_SC_E_NOMEM			= 9 << 8,  // memory exhausted
	MX_SC_E_PROTO_MISMATCH	= 10 << 8, // protocol vs. physical line type mismatch
	MX_SC_E_PROTO_PARAMS	= 11 << 8, // wrong protocol parameters
};


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
