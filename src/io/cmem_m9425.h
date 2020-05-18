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

#ifndef CMEM_M9425_H
#define CMEM_M9425_H

#include <pthread.h>

#include "io/dev/e4image.h"
#include "io/cmem.h"

// cmem control field - modes of operation
enum cmem_m9425_transmission_type_e {
	CMEM_M9425_RD	= 0b00, // read data
	CMEM_M9425_RA	= 0b01, // read address
	CMEM_M9425_WD	= 0b10, // write data
	CMEM_M9425_WA	= 0b11, // write address
};

// commands
enum cmem_m9425_cmd_e {
	// OU
	CMEM_M9425_CMD_ZER		= 0b100000, // reset
	CMEM_M9425_CMD_OTR		= 0b110000, // transmission with old addresses
	CMEM_M9425_CMD_NTR		= 0b110100, // tramsmission with new addresses
	CMEM_M9425_CMD_SEEK		= 0b111000, // seek to cylinder
	CMEM_M9425_CMD_RTZ		= 0b010000, // return to cylinder 0
	CMEM_M9425_CMD_SELOFF	= 0b101000, // disconnect device
	CMEM_M9425_CMD_RES		= 0b100100, // device test
	// IN
	CMEM_M9425_CMD_TEST		= 0b100000, // check device presence and availability
	CMEM_M9425_CMD_TSR		= 0b010000, // get last read/written sector status field
	CMEM_M9425_CMD_TCH		= 0b100100, // device test
};

// interrupts
enum cmem_m9425_int_e {
								// 001 - 004 are used by channel, see cmem_int_e
	CMEM_M9425_INT_ZER			= 005, // +reset done
	CMEM_M9425_INT_RES			= 006, // +device test done
	CMEM_M9425_INT_RTZ			= 010, // +return to track 0 done
	CMEM_M9425_INT_SEEK			= 011, // +seek done
	CMEM_M9425_INT_ALARM		= 012, // -control unit malfunction
	CMEM_M9425_INT_BLOCKED		= 013, // -drive malfunction
	CMEM_M9425_INT_NODEV		= 014, // no communication with drive
	CMEM_M9425_INT_NOSEEK		= 015, // -can't seek after 1s
	CMEM_M9425_INT_CF			= 016, // error in control field (frong field len, no such head, no such sector)
	CMEM_M9425_INT_SYNC_ADR		= 021, // -read/write while checking address field (?)
	CMEM_M9425_INT_SYNC_DATA	= 022, // -reading address while reading data (?)
	CMEM_M9425_INT_CRC_ADR		= 023, // -address field CRC error
	CMEM_M9425_INT_CRC_DATA		= 024, // -data CRC error
	CMEM_M9425_INT_CYL			= 025, // -cylinder mismatch
	CMEM_M9425_INT_HEAD			= 026, // -head mismatch
	CMEM_M9425_INT_SECTOR		= 027, // -sector mismatch
	CMEM_M9425_INT_KEY			= 030, // -key mismatch
	CMEM_M9425_INT_WRPROTECT	= 031, // sector write protected, but control field didn't ignore it
	CMEM_M9425_INT_SECT_PROTECT	= 032, // bad sector, but control field didn't ignore it
	CMEM_M9425_INT_CYL_END		= 033, // end of cylinder while reading/writing data (no more heads)
	CMEM_M9425_INT_END_MARK		= 034, // read end-mark character in data
	CMEM_M9425_INT_DONE			= 035, // transmission finished OK
};

struct cmem_unit_m9425_t {
	struct cmem_unit_proto_t proto;
	struct e4i_t *disk[2];

	pthread_t worker;
	int worker_trans_type;
	int worker_addr;
};

// --- transmit ----------------------------------------------------------
struct cmem_m9425_cf_t {
	int cf_len;
	int cpu;
	int nb;
	int oper;
	int len;
	int ign_wrprotect;
	int ign_defects;
	int ign_key;
	int ign_eof;
	int cyl;
	int platter;
	int head;
	int sector;
	int key;
	uint16_t addr;
};

struct cmem_unit_proto_t * cmem_m9425_create(struct cfg_arg *args);
void cmem_m9425_shutdown(struct cmem_unit_proto_t *unit);
void cmem_m9425_reset(struct cmem_unit_proto_t *unit);
void * cmem_m9425_worker(void *th_id);
int cmem_m9425_cmd(struct cmem_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg);

int cmem_m9425_decode_cf_t(int addr, struct cmem_m9425_cf_t *cf);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
