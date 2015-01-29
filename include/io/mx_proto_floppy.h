//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_PROTO_FLOPPY_H
#define MX_PROTO_FLOPPY_H

#include "io/mx_proto.h"

struct mx_proto_floppy {
	struct mx_proto base;

	int type;
	int fprotect;

};

// Floppy types
enum mx_floppy_type { 
	MX_FLOPPY_SD = 0,
	MX_FLOPPY_DD = 1,
	MX_FLOPPY_HD = 2,
	MX_FLOPPY_MAX
};  

// Floppy operations
enum mx_floppy_oper {
	MX_FLOPPY_OP_FORMAT		= 1,
	MX_FLOPPY_OP_READ		= 2,
	MX_FLOPPY_OP_WRITE		= 3,
	MX_FLOPPY_OP_BAD_SECTOR	= 4,
	MX_FLOPPY_OP_MAX
};

// Floppy return field (state word) flags
enum mx_floppy_status {
	// status word +1
	MX_FS_NO_FLOPPY		= 0b1000000000000000,   // hardware error, no floppy disk inserted
	MX_FS_ERR_TRANSMIT	= 0b0100000000000000,   // transmission error
	MX_FS_CRC			= 0b0000000000100000,   // data/address CRC error
	MX_FS_NO_ADDRESS	= 0b0000000000000100,   // address field not found
	MX_FS_WR_PROTECT	= 0b0000000000000010,   // floppy is write protected
	MX_FS_UNKNOWN		= 0b0000000000000001,   // no data
	// status word +2
	MX_FS_BAD_SECTOR	= 0b0100000000000000,   // bad sector found
	MX_FS_DATA_CRC		= 0b0010000000000000	// data CRC error
};


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
