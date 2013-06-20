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

#ifndef MULTIX_H
#define MULTIX_H

#include <inttypes.h>
#include <pthread.h>

#include "cfg.h"
#include "io.h"

#define MX_MAX_DEVICES 256

struct mx_int_t {
	unsigned unit_n;
	unsigned interrupt;
	struct mx_int_t *next;
};

struct mx_chan_t {
	struct chan_proto_t proto;

	int confset;
	struct unit_proto_t *lline[MX_MAX_DEVICES];
	struct unit_proto_t *pline[MX_MAX_DEVICES];

	struct mx_int_t *int_head;
	struct mx_int_t *int_tail;
	int int_reported;
	pthread_mutex_t int_mutex;
};

// multix commands
enum mx_cmd_e {
	// channel (bits 0..2 = 0, bits 3..4 = command)
	MX_CMD_RESET	= 0b00, // IN
	MX_CMD_EXISTS	= 0b10, // IN
	MX_CMD_INTSPEC	= 0b01, // IN
	// control, general (bits 0..2 = command)
	MX_CMD_INTRQ	= 0b001, // IN
	MX_CMD_TEST		= 0b001, // OU
	MX_CMD_SETCFG	= 0b101, // OU
	// control, line (bits 0..2 = command)
	MX_LCMD_ATTACH	= 0b010, // OU
	MX_LCMD_DETACH	= 0b010, // IN
	MX_LCMD_STATUS	= 0b011, // OU
	MX_LCMD_TRANSMIT= 0b100, // OU
	MX_LCMD_BREAK	= 0b011, // IN
};

// multix interrupts
enum mx_int_e {
	// special
	MX_INT_INSKA = 1,
	MX_INT_IWYZE = 2,
	MX_INT_IWYTE = 3,
	// general
	MX_INT_INKON = 4,
	MX_INT_IUKON = 5,
	MX_INT_INKOT = 6,
	// line
	MX_INT_ISTRE = 7,
	MX_INT_INSTR = 8,
	MX_INT_INKST = 9,
	MX_INT_IDOLI = 10,
	MX_INT_INDOL = 11,
	MX_INT_INKDO = 12,
	MX_INT_IETRA = 13,
	MX_INT_INTRA = 14,
	MX_INT_INKTR = 15,
	MX_INT_ITRER = 16,
	MX_INT_ITRAB = 19,
	MX_INT_IABTR = 20,
	MX_INT_INABT = 21,
	MX_INT_INKAB = 22,
	MX_INT_IODLI = 23,
	MX_INT_INODL = 24,
	MX_INT_INKOD = 25,
	MX_INT_INPAO = 32,
	MX_INT_IPARE = 33,
	MX_INT_IOPRU = 34
};

// --- cf: set configuration -------------------------------------------------

// physical line
struct mx_cf_sc_pl {
	int dir;
	int used;
	int dev_type;
	int count;
};

// logical line - winchester
struct mx_ll_winch {
	int type;
	int format_protect;
};

// logical line - floppy
struct mx_ll_floppy {
	int type;
	int format_protect;
};

// logical line
struct mx_cf_sc_ll {
	int proto;
	int pl_id;
	int formatter;
	struct mx_ll_winch *winch;
	struct mx_ll_floppy *floppy;
};

// set configuration
struct mx_cf_sc {
	int pl_desc_count;
	int ll_desc_count;
	uint16_t *retf;
	struct mx_cf_sc_pl *pl;
	struct mx_cf_sc_ll *ll;
};

// --- cf: connect line ------------------------------------------------------

// punch tape reader
struct mx_cf_cl_punch_reader {
	int watch_eot;
	int no_parity;
	int odd_parity;
	int eight_bits;
	int bs_can;
	int watch_oprq;
	int eot_code;
	int oprq_code;
	int txt_proc;
};

// tape puncher
struct mx_cf_cl_puncher {
	int odd_parity;
	int eight_bits;
	int low_to_up;
	int txt_proc;
};

// terminal (monitor)
struct mx_cf_cl_monitor {
	int watch_eot;
	int no_parity;
	int odd_parity;
	int eight_bits;
	int xon_xoff;
	int bs_can;
	int low_to_up;
	int watch_oprq;
	int eot_code;
	int oprq_code;
	int txt_proc;
	int txt_proc_params;
};

// --- get line status ---------------------------------------------------
// only return field



// -----------------------------------------------------------------------

struct chan_proto_t * mx_create(struct cfg_unit_t *units);
void mx_shutdown(struct chan_proto_t *chan);
void mx_reset(struct chan_proto_t *chan);
int mx_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg);

void mx_int(struct mx_chan_t *chan, int unit_n, int interrupt);
void mx_int_enq(struct mx_chan_t *chan, struct mx_int_t *mx_int);
void mx_int_preq(struct mx_chan_t *chan, struct mx_int_t *mx_int);
void mx_int_setq(struct mx_chan_t *chan, struct mx_int_t *mx_int);
struct mx_int_t * mx_int_deq(struct mx_chan_t *chan);

struct mx_cf_sc * mx_decode_cf_sc(int addr);
void mx_free_cf_sc(struct mx_cf_sc *cf);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
