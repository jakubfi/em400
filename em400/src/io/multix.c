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

#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "io/io.h"
#include "io/multix.h"
#include "io/multix_winch.h"

#include "cfg.h"
#include "errors.h"

#include "debugger/log.h"

#define CHAN ((struct mx_chan_t *)(chan))

struct unit_proto_t mx_unit_proto[] = {
	{ -1, "winchester",	mx_winch_create,	mx_winch_shutdown,	mx_winch_reset,	mx_winch_cmd },
	{ -1, NULL,			NULL,				NULL,				NULL,			NULL }
};

// -----------------------------------------------------------------------
struct chan_proto_t * mx_create(struct cfg_unit_t *units)
{
	struct mx_chan_t *chan = calloc(1, sizeof(struct mx_chan_t));
	if (!chan) {
		return NULL;
	}

	struct cfg_unit_t *cunit = units;
	while (cunit) {
		struct unit_proto_t *proto = io_unit_proto_get(mx_unit_proto, cunit->name);
		if (!proto) {
			gerr = E_IO_UNIT_UNKNOWN;
			return NULL;
		}

		eprint("    Unit %i: %s\n", cunit->num, proto->name);

		struct unit_proto_t *unit = proto->create(cunit->args);
		if (!unit) {
			return NULL;
		}

		unit->num = cunit->num;
		unit->name = proto->name;
		unit->create = proto->create;
		unit->reset = proto->reset;
		unit->shutdown = proto->shutdown;
		unit->cmd = proto->cmd;
		CHAN->pline[unit->num] = unit;
		cunit = cunit->next;
	}

	return (struct chan_proto_t*) chan;
}

// -----------------------------------------------------------------------
void mx_shutdown(struct chan_proto_t *chan)
{
	for (int i=0 ; i<MX_MAX_DEVICES ; i++) {
		struct unit_proto_t *unit = CHAN->pline[i];
		if (unit) {
			eprint("    Unit %i: %s\n", unit->num, unit->name);
			unit->shutdown(unit);
		}
	}
	free(chan);
}

// -----------------------------------------------------------------------
void mx_reset(struct chan_proto_t *chan)
{
}

// -----------------------------------------------------------------------
int mx_cmd_int_requeue(struct chan_proto_t *chan)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_test(struct chan_proto_t *chan, unsigned param, uint16_t *r_arg)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_setcfg(struct chan_proto_t *chan, uint16_t *r_arg)
{
	//struct mx_cf_sc *cf = mx_decode_cf_sc(*r_arg);
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_attach(struct chan_proto_t *chan, unsigned lline, uint16_t *r_arg)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_detach(struct chan_proto_t *chan, unsigned lline)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_status(struct chan_proto_t *chan, unsigned lline, uint16_t *r_arg)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_transmit(struct chan_proto_t *chan, unsigned lline, uint16_t *r_arg)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_break(struct chan_proto_t *chan, unsigned lline)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	unsigned cmd = (n_arg & 0b1110000000000000) >> 13;
	unsigned lline = (n_arg & 0b0001111111100000) >> 5;

	if (cmd == 0) { // channel commands
		cmd = (n_arg & 0b0001100000000000) >> 11;
		switch (cmd) {
			case MX_CMD_RESET:
				mx_reset(chan);
				// generates interrupt
				return IO_OK;
			case MX_CMD_EXISTS:
				return IO_OK;
			case MX_CMD_INTSPEC:
				return IO_OK;
			default:
				break;
		}
	} else {
		if (dir == IO_IN) {
			switch (cmd) {
				case MX_CMD_INTRQ:
					return mx_cmd_int_requeue(chan);
				case MX_LCMD_DETACH:
					return mx_cmd_detach(chan, lline);
				case MX_LCMD_BREAK:
					return mx_cmd_break(chan, lline);
				default:
					break;
			}
		} else {
			switch (cmd) {
				case MX_CMD_TEST:
					return mx_cmd_test(chan, lline, r_arg);
				case MX_CMD_SETCFG:
					return mx_cmd_setcfg(chan, r_arg);
				case MX_LCMD_ATTACH:
					return mx_cmd_attach(chan, lline, r_arg);
				case MX_LCMD_STATUS:
					return mx_cmd_status(chan, lline, r_arg);
				case MX_LCMD_TRANSMIT:
					return mx_cmd_transmit(chan, lline, r_arg);
				default:
					break;
			}
		}
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
struct mx_cf_sc * mx_decode_cf_sc(int addr)
{
	struct mx_cf_sc *cf = calloc(1, sizeof(struct mx_cf_sc));
	uint16_t data;

	// --- word 0 - header ---
	data = MEMB(0, addr);
	cf->pl_desc_count = (data & 0b1111111100000000) >> 8;
	cf->ll_desc_count = (data & 0b0000000011111111);

	// --- word 1 - return field ---
	cf->retf = mem_ptr(0, addr+1);

	if (cf->pl_desc_count <= 0) {
		// missing physical line description
		return NULL;
	}

	if (cf->ll_desc_count <= 0) {
		// missing logical line descroption
		return NULL;
	}

	cf->pl = calloc(cf->pl_desc_count, sizeof(struct mx_cf_sc_pl));
	cf->ll = calloc(cf->ll_desc_count, sizeof(struct mx_cf_sc_ll));

	if (!cf->pl || !cf->ll) {
		// cannot allocate memory
		return NULL;
	}

	// --- physical lines, 1 word each ---
	for (int pln=0 ; pln<cf->pl_desc_count ; pln++) {
		data = MEMB(0, addr+2+pln);
		cf->pl[pln].dir =		(data & 0b1110000000000000) >> 13;
		cf->pl[pln].used =		(data & 0b0001000000000000) >> 12;
		cf->pl[pln].dev_type =	(data & 0b0000111100000000) >> 8;
		cf->pl[pln].count =		(data & 0b0000000000011111) + 1;
	}
	// --- logical lines, 4 words each ---
	for (int lln=0 ; lln<cf->ll_desc_count ; lln++) {
		data = MEMB(0, addr+2+cf->pl_desc_count+(lln*4));
		cf->ll[lln].proto = (data & 0b1111111100000000) >> 8;
		cf->ll[lln].pl_id = (data & 0b0000000011111111);
		switch (cf->ll[lln].proto) {
			case 6: // Winchester
				cf->ll[lln].winch = calloc(1, sizeof(struct mx_ll_winch));
				if (!cf->ll[lln].winch) {
					return NULL;
				}
				data = MEMB(0, addr+2+cf->pl_desc_count+(lln*4)+1);
				cf->ll[lln].winch->type =			(data & 0b1111111100000000) >> 8;
				cf->ll[lln].winch->format_protect =	(data & 0b0000000011111111);
				break;
			case 7: // magnetic tape
				// MT protocol changes meaning of pl_id and adds formatter field
				cf->ll[lln].pl_id &= 0b00011111;
				cf->ll[lln].formatter = (cf->ll[lln].pl_id & 0b10000000) >> 7;
				break;
			case 8: // floppy disk
				cf->ll[lln].floppy = calloc(1, sizeof(struct mx_ll_floppy));
				if (!cf->ll[lln].floppy) {
					return NULL;
				}
				data = MEMB(0, addr+2+cf->pl_desc_count+(lln*4)+1);
				cf->ll[lln].floppy->type =				(data & 0b1111111100000000) >> 8;
				cf->ll[lln].floppy->format_protect =	(data & 0b0000000011111111);
				break;
			default: // unknown protocol
				return NULL;
		}
	}

	return cf;
}


// -----------------------------------------------------------------------
void mx_free_cf_sc(struct mx_cf_sc *cf)
{
	if (cf->pl) {
		free(cf->pl);
	}

	if (cf->ll) {
		for (int i=0 ; i<cf->ll_desc_count ; i++) {
			if (cf->ll[i].winch) {
				free(cf->ll[i].winch);
			}
			if (cf->ll[i].floppy) {
				free(cf->ll[i].floppy);
			}
		}
		free(cf->ll);
	}

	free(cf);
}


// vim: tabstop=4 shiftwidth=4 autoindent
