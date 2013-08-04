//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef _MULTIX_FLOPPY_H_
#define _MULTIX_FLOPPY_H_

#include <inttypes.h>

#include "io/io.h"
#include "io/multix.h"
#include "e4image.h"

struct mx_unit_floppy_t {
	struct mx_unit_proto_t proto;
	struct e4i_t *floppy;
	int floppy_type;
	int format_protect;
};

// Floppy types
enum mx_floppy_type_e {
	MX_FLOPPY_SD = 0,
	MX_FLOPPY_DD = 1,
	MX_FLOPPY_HD =2
};

// Floppy operations
enum mx_floppy_t_oper_e {
	MX_FLOPPY_FORMAT = 1,
	MX_FLOPPY_READ = 2,
	MX_FLOPPY_WRITE = 3,
	MX_FLOPPY_BAD_SECTOR = 4
};

// Floppy return field (state word) flags

enum mx_floppy_t_status {
	// status word +1
	MX_FS_NO_FLOPPY		= 0b1000000000000000,   // hardware error, no floppy disk inserted
	MX_FS_ERR_TRANSMIT	= 0b0100000000000000,   // transmission error
	MX_FS_CRC			= 0b0000000000100000,   // data/address CRC error
	MX_FS_NO_ADDRESS	= 0b0000000000000100,   // address field not found
	MX_FS_WR_PROTECT	= 0b0000000000000010,   // floppy is write protected
	MX_FS_UNKNOWN		= 0b0000000000000001,	// no data
	// status word +2
	MX_FS_BAD_SECTOR	= 0b0100000000000000,	// bad sector found
	MX_FS_DATA_CRC		= 0b0010000000000000	// data CRC error
};

// transmit: format track
struct mx_floppy_cf_format {
	int start_sector;
};

// transmit: read/write
struct mx_floppy_cf_transmit {
	int ign_crc;
	int cpu;
	int nb;
	int addr;
	uint16_t len;
	int sector;
};

// transmit: mark bad sector
struct mx_floppy_cf_bad_sector {
	int sector;
};

// transmit
struct mx_floppy_cf_t {
	int oper;
	struct mx_floppy_cf_format *format;
	struct mx_floppy_cf_transmit *transmit;
	struct mx_floppy_cf_bad_sector *bad_sector;
	uint16_t *ret_len;
	uint16_t *ret_status;
};


struct mx_unit_proto_t * mx_floppy_create(struct cfg_arg_t *args);
struct mx_unit_proto_t * mx_floppy_create_nodev();
void mx_floppy_connect(struct mx_unit_floppy_t *unit, struct e4i_t *floppy);
void mx_floppy_disconnect(struct mx_unit_floppy_t *unit);
void mx_floppy_shutdown(struct mx_unit_proto_t *unit);
void mx_floppy_reset(struct mx_unit_proto_t *unit);
int mx_floppy_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy);
int mx_floppy_cfg_log(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log);

void mx_floppy_cmd_attach(struct mx_unit_proto_t *unit, uint16_t addr);
void mx_floppy_cmd_detach(struct mx_unit_proto_t *unit, uint16_t addr);
void mx_floppy_cmd_status(struct mx_unit_proto_t *unit, uint16_t addr);
void mx_floppy_cmd_transmit(struct mx_unit_proto_t *unit, uint16_t addr);

struct mx_floppy_cf_t * mx_floppy_cf_t_decode(int addr);
void mx_floppy_cf_t_free(struct mx_floppy_cf_t *cf);



#endif

// vim: tabstop=4 shiftwidth=4 autoindent
