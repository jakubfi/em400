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

#ifndef CCHAR_TERM_CONS_H
#define CCHAR_TERM_CONS_H

#include <stdbool.h>

#include "io/dev/fdbridge.h"
#include "io/cchar/cchar.h"
#include "cfg.h"

#define TERM_BUF_LEN 1024

struct cchar_unit_term_t {
	struct cchar_unit_proto_t proto;
	struct fdb *term;
	int spec;
};

// commands
enum cchar_term_cmd_e {
	// OU
	CCHAR_TERM_CMD_RESET		= 0b100000, // reset
	CCHAR_TERM_CMD_DISCONNECT	= 0b101000, // disconnect device (soft reset)
	CCHAR_TERM_CMD_WRITE		= 0b110000, // write
	// IN
	CCHAR_TERM_CMD_SPU			= 0b100000, // check device
	CCHAR_TERM_CMD_READ			= 0b101000, // read
};

// interrupts
enum char_term_int_e {
	CCHAR_TERM_INT_OUTDATED	= 0, // interrupt out of date
	CCHAR_TERM_INT_READY	= 1, // ready again
	CCHAR_TERM_INT_OPR_CALL	= 3, // operator call
	CCHAR_TERM_INT_TOO_SLOW	= 5, // transmission too slow
};

struct cchar_unit_proto_t * cchar_term_create(em400_cfg *cfg, int ch_num, int dev_num);
void cchar_term_shutdown(struct cchar_unit_proto_t *unit);
void cchar_term_reset(struct cchar_unit_proto_t *unit);
void * cchar_term_worker(void *ptr);
int cchar_term_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg);
int cchar_term_intspec(struct cchar_unit_proto_t *unit);
bool cchar_term_has_interrupt(struct cchar_unit_proto_t *unit);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
