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
#include "io/multix_floppy.h"

#include "cfg.h"
#include "errors.h"
#include "cpu/interrupts.h"
#include "debugger/log.h"

#define CHAN ((struct mx_chan_t *)(chan))

// unit prototypes
struct mx_unit_proto_t mx_unit_proto[] = {
	{
		"winchester",
		mx_winch_create,
		mx_winch_create_nodev,
		mx_winch_shutdown,
		mx_winch_reset,
		mx_winch_cfg_phy,
		mx_winch_cfg_log,
		mx_winch_cmd,
		MX_PHY_WINCHESTER
	},
	{
		"floppy",
		mx_floppy_create,
		mx_floppy_create_nodev,
		mx_floppy_shutdown,
		mx_floppy_reset,
		mx_floppy_cfg_phy,
		mx_floppy_cfg_log,
		mx_floppy_cmd,
		MX_PHY_FLOPPY
	},
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		-1
	}
};

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_unit_proto_get_by_name(struct mx_unit_proto_t *proto, char *name)
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
struct mx_unit_proto_t * mx_unit_proto_get_by_type(struct mx_unit_proto_t *proto, int type)
{
	while (proto && proto->name) {
		if (type == proto->type) {
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
		struct mx_unit_proto_t *proto = mx_unit_proto_get_by_name(mx_unit_proto, cunit->name);
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

		unit->name = proto->name;
		unit->create = proto->create;
		unit->create_nodev = proto->create_nodev;
		unit->shutdown = proto->shutdown;
		unit->reset = proto->reset;
		unit->cfg_phy = proto->cfg_phy;
		unit->cfg_log = proto->cfg_log;
		unit->cmd = proto->cmd;

		unit->chan = chan;

		unit->type = -1;
		unit->phy_num = cunit->num;
		unit->dir = -1;
		unit->used = -1;
		unit->log_num = -1;
		unit->protocol = -1;
		unit->attached = 0;

		CHAN->pline[unit->phy_num] = unit;
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
			eprint("    Unit %i: %s\n", unit->phy_num, unit->name);
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
	LOG(D_IO, 1, "MULTIX (ch:%i) command: reset", chan->num);
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
		LOG(D_IO, 1, "MULTIX (ch:%i) reporting interrupt %i", chan->proto.num, chan->proto.num + 12);
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

	LOG(D_IO, 1, "MULTIX (ch:%i) interrupt %i, unit: %i", chan->proto.num, interrupt, unit_n);

	pthread_mutex_lock(&CHAN->int_mutex);

	if (interrupt == MX_INT_IWYZE) {
		mx_int_setq(chan, mx_int);
	} else if (interrupt <= MX_INT_IWYTE) {
		mx_int_preq(chan, mx_int);
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
		mx_int_report(chan);
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
	mx_int_report(chan);
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
	mx_int_report(chan);
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
	LOG(D_IO, 1, "MULTIX (ch:%i) command: intspec", chan->num);
	pthread_mutex_lock(&CHAN->int_mutex);

	struct mx_int_t *inth = mx_int_deq(CHAN);

	pthread_mutex_unlock(&CHAN->int_mutex);

	if (inth) {
		*r_arg = (inth->interrupt << 8) | (inth->unit_n);
		free(inth);
	} else {
		*r_arg = MX_INT_INIEA << 8;
	}

	mx_int_report(CHAN);

	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_test(struct chan_proto_t *chan, unsigned param, uint16_t *r_arg)
{
	LOG(D_IO, 1, "MULTIX (ch:%i) command: test", chan->num);
	mx_int(CHAN, 0, MX_INT_IWYTE);
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_setcfg(struct chan_proto_t *chan, uint16_t *r_arg)
{
	LOG(D_IO, 1, "MULTIX (ch:%i) command: setcfg", chan->num);
	// return field address
    uint16_t retf_addr = *r_arg + 1;
	uint16_t retf;

	// fail, if configuration is already set
	if (CHAN->confset) {
		LOG(D_IO, 10, "MULTIX setconf error: configuration already set");
		MEMBw(0, retf_addr, MX_SC_E_CONFSET << 8);
		mx_int(CHAN, 0, MX_INT_INKON);
		return IO_OK;
	}

	// allocate memory for cf
	struct mx_cf_sc *cf = calloc(1, sizeof(struct mx_cf_sc));

	// fail, if cannot
	if (!cf) {
		MEMBw(0, retf_addr, MX_SC_E_NOMEM << 8);
		mx_int(CHAN, 0, MX_INT_INKON);
		mx_free_cf_sc(cf);
		return IO_OK;
	}

	// decode cf field
	int res = mx_decode_cf_sc(*r_arg, cf);

	// decoding failed
	if (res != E_OK) {
		LOG(D_IO, 10, "MULTIX setconf error: decode failed (error: %i, line: %i)", cf->err_code, cf->err_line);
		retf = (cf->err_code << 8) | cf->err_line;
		MEMBw(0, retf_addr, retf);
		mx_int(CHAN, 0, MX_INT_INKON);
		mx_free_cf_sc(cf);
		return IO_OK;
	}

	int i;
	struct mx_unit_proto_t *unit;

	// set physical line configuration
	for (i=0 ; i<cf->pl_desc_count ; i++) {
		struct mx_cf_sc_pl *phy = cf->pl+i;
		// skip unused physical lines
		if (!phy->used) continue;

		// configure all units in range
		for (int unit_n = phy->offset ; unit_n < phy->offset+phy->count ; unit_n++) {
			unit = CHAN->pline[unit_n];
			if (unit) {
				res = unit->cfg_phy(unit, phy);
				LOG(D_IO, 1, "Physical line %i (type: %i) configured", unit_n, unit->type);
			} else {
				// if device, that OS tries to configure, is not configured in emulator,
				// physical/logical configuration is lost.
				// TODO: this needs to change when dynamic device configuration is added
				// we would need to use create_nodev() and later in unit code check if device is connected
				LOG(D_IO, 1, "Physical line %i NOT configured (missing in em400 configuration)", unit_n);
			}
		}
	}

	// set logical line configuration
	for (i=0 ; i<cf->ll_desc_count ; i++) {
		struct mx_cf_sc_ll *log = cf->ll+i;
		unit = CHAN->pline[log->pl_id];
		res = unit->cfg_log(unit, log);
		CHAN->lline[i] = unit;
		unit->log_num = i;
		LOG(D_IO, 1, "Logical line %i configured (physical: %i, protocol: %i)", i, log->pl_id, log->proto);
	}

	mx_int(CHAN, 0, MX_INT_IUKON);
	mx_free_cf_sc(cf);

	LOG(D_IO, 1, "MULTIX configuration set");

	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_attach(struct chan_proto_t *chan, int dir, int cmd, int lline_n, uint16_t addr)
{
	struct mx_unit_proto_t *unit = CHAN->lline[lline_n];
	if (!unit) {
		mx_int(CHAN, lline_n, MX_INT_INKDO);
		return IO_OK;
	}

	if (unit->attached) {
		mx_int(CHAN, lline_n, MX_INT_INDOL);
		return IO_OK;
	}

	return unit->cmd(unit, dir|cmd, addr);
}

// -----------------------------------------------------------------------
int mx_cmd_detach(struct chan_proto_t *chan, int dir, int cmd, int lline_n, uint16_t addr)
{
	struct mx_unit_proto_t *unit = CHAN->lline[lline_n];
	if (!unit) {
		mx_int(CHAN, lline_n, MX_INT_INKOD);
		return IO_OK;
	}

	if (!unit->attached) {
		mx_int(CHAN, lline_n, MX_INT_IODLI);
		return IO_OK;
	}

	return unit->cmd(unit, dir|cmd, addr);
}

// -----------------------------------------------------------------------
int mx_cmd_transmit(struct chan_proto_t *chan, int dir, int cmd, int lline_n, uint16_t addr)
{
	struct mx_unit_proto_t *unit = CHAN->lline[lline_n];
	if (!unit) {
		mx_int(CHAN, lline_n, MX_INT_INKTR);
		return IO_OK;
	}

	if (!unit->attached) {
		mx_int(CHAN, lline_n, MX_INT_INTRA);
		return IO_OK;
	}

	return unit->cmd(unit, dir|cmd, addr);
}

// -----------------------------------------------------------------------
int mx_cmd_status(struct chan_proto_t *chan, int dir, int cmd, int lline_n, uint16_t addr)
{
	struct mx_unit_proto_t *unit = CHAN->lline[lline_n];
	if (!unit) {
		mx_int(CHAN, lline_n, MX_INT_INKST);
		return IO_OK;
	}

	return unit->cmd(unit, dir|cmd, addr);
}

// -----------------------------------------------------------------------
int mx_cmd_break(struct chan_proto_t *chan, int dir, int cmd, int lline_n, uint16_t addr)
{
	struct mx_unit_proto_t *unit = CHAN->lline[lline_n];
	if (!unit) {
		mx_int(CHAN, lline_n, MX_INT_INKAB);
		return IO_OK;
	}

	return unit->cmd(unit, dir|cmd, addr);
}

// -----------------------------------------------------------------------
int mx_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	unsigned cmd = (n_arg & 0b1110000000000000) >> 13;
	unsigned lline_n = (n_arg & 0b0001111111100000) >> 5;

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
				LOG(D_IO, 1, "MULTIX (ch:%i) command: unknown channel command", chan->num);
				break;
		}
	} else {
		if (dir == IO_IN) {
			switch (cmd) {
				case MX_CMD_INTRQ:
					return mx_cmd_int_requeue(chan);
				case MX_LCMD_DETACH:
					LOG(D_IO, 1, "MULTIX (ch:%i, line:%i) command: detach", chan->num, lline_n);
					return mx_cmd_detach(chan, dir, cmd, lline_n, *r_arg);
				case MX_LCMD_BREAK:
					LOG(D_IO, 1, "MULTIX (ch:%i, line:%i) command: break transmission", chan->num, lline_n);
					return mx_cmd_break(chan, dir, cmd, lline_n, *r_arg);
				default:
					LOG(D_IO, 1, "MULTIX (ch:%i, line:%i) command: unknown line IN command", chan->num, lline_n);
					break;
			}
		} else {
			switch (cmd) {
				case MX_CMD_TEST:
					return mx_cmd_test(chan, lline_n, r_arg);
				case MX_CMD_SETCFG:
					return mx_cmd_setcfg(chan, r_arg);
				case MX_LCMD_ATTACH:
					LOG(D_IO, 1, "MULTIX (ch:%i, line:%i) command: attach", chan->num, lline_n);
					return mx_cmd_attach(chan, dir, cmd, lline_n, *r_arg);
				case MX_LCMD_STATUS:
					LOG(D_IO, 1, "MULTIX (ch:%i, line:%i) command: status", chan->num, lline_n);
					return mx_cmd_status(chan, dir, cmd, lline_n, *r_arg);
				case MX_LCMD_TRANSMIT:
					LOG(D_IO, 1, "MULTIX (ch:%i, line:%i) command: transmit", chan->num, lline_n);
					return mx_cmd_transmit(chan, dir, cmd, lline_n, *r_arg);
				default:
					LOG(D_IO, 1, "MULTIX (ch:%i, line:%i) command: unknown line OUT command", chan->num, lline_n);
					break;
			}
		}
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
struct mx_cf_sc_pl * mx_decode_cf_find_phy(struct mx_cf_sc_pl *phys, int count, int id)
{
	struct mx_cf_sc_pl *phy = phys;
	while (phy <= phys+count) {
		if ((id >= phy->offset) && (id < phy->offset+phy->count)) {
			return phy;
		}
		phy++;
	}
	return NULL;
}

// -----------------------------------------------------------------------
int mx_decode_cf_phy(int addr, struct mx_cf_sc_pl *phy, int offset)
{
	uint16_t data = MEMB(0, addr);
	phy->dir =   (data & 0b1110000000000000) >> 13;
	phy->used =  (data & 0b0001000000000000) >> 12;
	phy->type =  (data & 0b0000111100000000) >> 8;
	phy->count = (data & 0b0000000000011111) + 1;
	phy->offset = offset;
	phy->logical_configured = calloc(phy->count, sizeof(int));

	if (phy->used) {
		// check type for correctness
		if ((phy->type < 0) || (phy->type >= MX_PHY_MAX)) {
			return MX_SC_E_DEVTYPE;
		}

		// check direction for correctness
		if ((phy->type <= MX_PHY_USART_SYNC)
		&& (phy->dir != MX_DIR_OUTPUT)
		&& (phy->dir != MX_DIR_INPUT)
		&& (phy->dir != MX_DIR_HALF_DUPLEX)
		&& (phy->dir != MX_DIR_FULL_DUPLEX)) {
			return MX_SC_E_DIR;
		}
	}

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
int mx_decode_cf_log(int addr, struct mx_cf_sc_ll *log, struct mx_cf_sc_pl *phys, int phy_count)
{
	// TODO: MX_SC_E_DIR_MISMATCH check

	uint16_t data = MEMB(0, addr);
	log->proto = (data & 0b1111111100000000) >> 8;
	log->pl_id = (data & 0b0000000011111111);

	// find related physical line description
	struct mx_cf_sc_pl *phy = mx_decode_cf_find_phy(phys, phy_count, log->pl_id);
	if (!phy) {
		return MX_SC_E_NUMLINES;
	}

	// make sure that we are trying to configure physical line, that is active (configured)
	if (!phy->used) {
		return MX_SC_E_PHY_UNUSED;
	}

	// make sure that there is no other logical line which uses this physical line
	if (phy->logical_configured[log->pl_id-phy->offset]) {
		return MX_SC_E_PHY_BUSY;
	}

	switch (log->proto) {
		case 6: // Winchester
			if (phy->type != MX_PHY_WINCHESTER) {
				return MX_SC_E_PROTO_MISMATCH;
			}
			log->winch = calloc(1, sizeof(struct mx_ll_winch));
			if (!log->winch) {
				return MX_SC_E_NOMEM;
			}
			data = MEMB(0, addr+1);
			log->winch->type =				(data & 0b1111111100000000) >> 8;
			log->winch->format_protect =	(data & 0b0000000011111111);
			break;
		case 7: // magnetic tape
			if (phy->type != MX_PHY_MTAPE) {
				return MX_SC_E_PROTO_MISMATCH;
			}
			// MT protocol changes meaning of pl_id and adds formatter field
			log->formatter = (log->pl_id & 0b10000000) >> 7;
			log->pl_id &= 0b00011111;
			break;
		case 8: // floppy disk
			if (phy->type != MX_PHY_FLOPPY) {
				return MX_SC_E_PROTO_MISMATCH;
			}
			log->floppy = calloc(1, sizeof(struct mx_ll_floppy));
			if (!log->floppy) {
				return MX_SC_E_NOMEM;
			}
			data = MEMB(0, addr+1);
			log->floppy->type =				(data & 0b1111111100000000) >> 8;
			log->floppy->format_protect =	(data & 0b0000000011111111);
			break;
		default: // unknown protocol
			return MX_SC_E_PROTO_MISSING;
	}

	phy->logical_configured[log->pl_id-phy->offset] = 1;

	return MX_SC_E_OK;
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

	// --- word 1 - return field - used by 'cmd' multix function to report status back

	// sanity checks
	if ((cf->pl_desc_count <= 0) || (cf->pl_desc_count > MX_MAX_DEVICES)) {
		// missing physical line description
		cf->err_code = MX_SC_E_NUMLINES;
		return E_MX_DECODE;
	}
	if ((cf->ll_desc_count <= 0) || (cf->ll_desc_count > MX_MAX_DEVICES)) {
		// missing logical line descroption
		cf->err_code = MX_SC_E_NUMLINES;
		return E_MX_DECODE;
	}

	// create room for physical/logical line descriptions
	cf->pl = calloc(cf->pl_desc_count, sizeof(struct mx_cf_sc_pl));
	cf->ll = calloc(cf->ll_desc_count, sizeof(struct mx_cf_sc_ll));
	if (!cf->pl || !cf->ll) {
		cf->err_code = MX_SC_E_NOMEM;
		return E_MX_DECODE;
	}

	// --- physical lines, 1 word each ---
	int offset = 0;
	for (int pln=0 ; pln<cf->pl_desc_count ; pln++) {
		struct mx_cf_sc_pl *phy = cf->pl+pln;
		cf->err_line = pln;
		cf->err_code = mx_decode_cf_phy(addr+2+pln, phy, offset);
		if (cf->err_code != MX_SC_E_OK) {
			return E_MX_DECODE;
		}
		offset += phy->count;
	}

	// --- logical lines, 4 words each ---
	for (int lln=0 ; lln<cf->ll_desc_count ; lln++) {
		struct mx_cf_sc_ll *log = cf->ll+lln;

		cf->err_line = lln;
		cf->err_code = mx_decode_cf_log(addr+2+cf->pl_desc_count+(lln*4), log, cf->pl, cf->pl_desc_count);
		if (cf->err_code != MX_SC_E_OK) {
			return E_MX_DECODE;
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void mx_free_cf_sc(struct mx_cf_sc *cf)
{
	if (cf->pl) {
		if (cf->pl->logical_configured) {
			free(cf->pl->logical_configured);
		}
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
