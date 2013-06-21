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
#include <strings.h>
#include <pthread.h>

#include "io/io.h"
#include "io/multix.h"
#include "io/multix_winch.h"

#include "cfg.h"
#include "errors.h"
#include "cpu/interrupts.h"
#include "debugger/log.h"

#define CHAN ((struct mx_chan_t *)(chan))

struct mx_unit_proto_t mx_unit_proto[] = {
	{ -1, "winchester",	mx_winch_create,	mx_winch_shutdown,	mx_winch_reset,	mx_winch_cfg_phy,	mx_winch_cfg_log,	mx_winch_cmd },
	{ -1, NULL,			NULL,				NULL,				NULL,			NULL,				NULL,				NULL }
};

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_unit_proto_get(struct mx_unit_proto_t *proto, char *name)
{
	while (proto && proto->name) {
		if (strcasecmp(name, proto->name) == 0) {
			return proto;
		}
		proto++;
	}
	return NULL;
}

// -----------------------------------------------------------------------
struct chan_proto_t * mx_create(struct cfg_unit_t *units)
{
	struct mx_chan_t *chan = calloc(1, sizeof(struct mx_chan_t));
	if (!chan) {
		return NULL;
	}

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&chan->int_mutex, &attr);

	struct cfg_unit_t *cunit = units;
	while (cunit) {
		struct mx_unit_proto_t *proto = mx_unit_proto_get(mx_unit_proto, cunit->name);
		if (!proto) {
			gerr = E_IO_UNIT_UNKNOWN;
			free(chan);
			return NULL;
		}

		eprint("    Unit %i: %s\n", cunit->num, proto->name);

		struct mx_unit_proto_t *unit = proto->create(cunit->args);
		if (!unit) {
			free(chan);
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
		struct mx_unit_proto_t *unit = CHAN->pline[i];
		if (unit) {
			eprint("    Unit %i: %s\n", unit->num, unit->name);
			unit->shutdown(unit);
			CHAN->pline[i] = NULL;
		}
	}
	pthread_mutex_destroy(&CHAN->int_mutex);
	free(chan);
}

// -----------------------------------------------------------------------
void mx_reset(struct chan_proto_t *chan)
{
	for (int i=0 ; i<MX_MAX_DEVICES ; i++) {
		struct mx_unit_proto_t *punit = CHAN->pline[i];
		if (punit) {
			punit->reset(punit);
		}
		CHAN->lline[i] = NULL;
	}
	CHAN->confset = 0;
	mx_int(CHAN, 0, MX_INT_IWYZE);
}

// -----------------------------------------------------------------------
void mx_int_report(struct mx_chan_t *chan)
{
	if (CHAN->int_head) {
		int_set(chan->proto.num + 12);
	}
}

// -----------------------------------------------------------------------
void mx_int(struct mx_chan_t *chan, int unit_n, int interrupt)
{
	struct mx_int_t *mx_int = malloc(sizeof(struct mx_int_t));
	mx_int->unit_n = unit_n;
	mx_int->interrupt = interrupt;
	mx_int->next = NULL;

	pthread_mutex_lock(&CHAN->int_mutex);

	if (interrupt == MX_INT_IWYZE) {
		mx_int_setq(chan, mx_int);
		mx_int_report(chan);
	} else if (interrupt <= MX_INT_IWYTE) {
		mx_int_preq(chan, mx_int);
		mx_int_report(chan);
	} else {
		mx_int_enq(chan, mx_int);
	}

	pthread_mutex_unlock(&CHAN->int_mutex);
}

// -----------------------------------------------------------------------
void mx_int_enq(struct mx_chan_t *chan, struct mx_int_t *mx_int)
{
	if (chan->int_tail) {
		chan->int_tail->next = mx_int;
		chan->int_tail = mx_int;
	} else {
		chan->int_tail = chan->int_head = mx_int;
	}
}

// -----------------------------------------------------------------------
void mx_int_preq(struct mx_chan_t *chan, struct mx_int_t *mx_int)
{
	if (chan->int_head) {
		mx_int->next = chan->int_tail;
		chan->int_head = mx_int;
	} else {
		chan->int_tail = chan->int_head = mx_int;
	}
}

// -----------------------------------------------------------------------
void mx_int_setq(struct mx_chan_t *chan, struct mx_int_t *mx_int)
{
	struct mx_int_t *inth;
	struct mx_int_t *next;
	inth = chan->int_head;
	while (inth) {
		next = inth->next;
		free(inth);
		inth = next;
	}
	chan->int_tail = chan->int_head = mx_int;
}

// -----------------------------------------------------------------------
struct mx_int_t * mx_int_deq(struct mx_chan_t *chan)
{
	struct mx_int_t *inth = chan->int_head;

	if (inth) {
		if (chan->int_head == chan->int_tail) {
			chan->int_tail = NULL;
		}
		chan->int_head = chan->int_head->next;
	}

	return inth;
}

// -----------------------------------------------------------------------
int mx_cmd_int_requeue(struct chan_proto_t *chan)
{
	pthread_mutex_lock(&CHAN->int_mutex);

	if (CHAN->int_head) {
		struct mx_int_t *inth = mx_int_deq(CHAN);
		mx_int_enq(CHAN, inth);
	}

	pthread_mutex_unlock(&CHAN->int_mutex);

	mx_int_report(CHAN);

	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_intspec(struct chan_proto_t *chan, uint16_t *r_arg)
{
	pthread_mutex_lock(&CHAN->int_mutex);

	struct mx_int_t *inth = mx_int_deq(CHAN);

	pthread_mutex_unlock(&CHAN->int_mutex);

	if (inth) {
		*r_arg = (inth->interrupt << 8) | (inth->unit_n);
		free(inth);
	} else {
		*r_arg = 0;
	}

	mx_int_report(CHAN);

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
	// return field address
    uint16_t retf_addr = *r_arg + 1;

	// fail, if configuration is already set
	if (CHAN->confset) {
		MEMBw(0, retf_addr, MX_SC_E_CONFSET);
		mx_int(CHAN, 0, MX_INT_INKON);
		return IO_OK;
	}

	// allocate memory for cf
	struct mx_cf_sc *cf = calloc(1, sizeof(struct mx_cf_sc));

	// fail, if cannot
	if (!cf) {
		MEMBw(0, retf_addr, MX_SC_E_NOMEM);
		mx_int(CHAN, 0, MX_INT_INKON);
		return IO_OK;
	}

	// decode cf field
	int res = mx_decode_cf_sc(*r_arg, cf);

	if (res != E_OK) {
	}

	mx_free_cf_sc(cf);

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
				return IO_OK;
			case MX_CMD_EXISTS:
				return IO_OK;
			case MX_CMD_INTSPEC:
				return mx_cmd_intspec(chan, r_arg);
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
int mx_decode_cf_sc(int addr, struct mx_cf_sc *cf)
{
	if (!cf) {
		return E_MX_DECODE;
	}

	uint16_t data;

	// --- word 0 - header ---
	data = MEMB(0, addr);
	cf->pl_desc_count = (data & 0b1111111100000000) >> 8;
	cf->ll_desc_count = (data & 0b0000000011111111);

	// --- word 1 - return field ---
	// used by command function

	if ((cf->pl_desc_count <= 0) || (cf->pl_desc_count > MX_MAX_DEVICES)) {
		// missing physical line description
		cf->retf = MX_SC_E_NUMLINES;
		return E_MX_DECODE;
	}

	if ((cf->ll_desc_count <= 0) || (cf->ll_desc_count > MX_MAX_DEVICES)) {
		// missing logical line descroption
		cf->retf = MX_SC_E_NUMLINES;
		return E_MX_DECODE;
	}

	cf->pl = calloc(cf->pl_desc_count, sizeof(struct mx_cf_sc_pl));
	cf->ll = calloc(cf->ll_desc_count, sizeof(struct mx_cf_sc_ll));

	if (!cf->pl || !cf->ll) {
		cf->retf = MX_SC_E_NOMEM;
		return E_MX_DECODE;
	}

	// --- physical lines, 1 word each ---
	for (int pln=0 ; pln<cf->pl_desc_count ; pln++) {
		data = MEMB(0, addr+2+pln);
		cf->pl[pln].dir =	(data & 0b1110000000000000) >> 13;
		cf->pl[pln].used =	(data & 0b0001000000000000) >> 12;
		cf->pl[pln].type =	(data & 0b0000111100000000) >> 8;
		cf->pl[pln].count =	(data & 0b0000000000011111) + 1;
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
					cf->retf = MX_SC_E_NOMEM;
					return E_MX_DECODE;
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
					cf->retf = MX_SC_E_NOMEM;
					return E_MX_DECODE;
				}
				data = MEMB(0, addr+2+cf->pl_desc_count+(lln*4)+1);
				cf->ll[lln].floppy->type =				(data & 0b1111111100000000) >> 8;
				cf->ll[lln].floppy->format_protect =	(data & 0b0000000011111111);
				break;
			default: // unknown protocol
				cf->retf = MX_SC_E_PROTO_MISSING;
				return E_MX_DECODE;
		}
	}

	return E_OK;
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
