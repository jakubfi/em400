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

struct mx_unit_proto_t;
struct mx_cf_sc_pl;
struct mx_cf_sc_ll;

typedef struct mx_unit_proto_t * (*mx_unit_f_create)(struct cfg_arg *args);
typedef struct mx_unit_proto_t * (*mx_unit_f_create_nodev)();
typedef void (*mx_unit_f_shutdown)(struct mx_unit_proto_t *unit);
typedef void (*mx_unit_f_reset)(struct mx_unit_proto_t *unit);
typedef int (*mx_unit_f_cfg_phy)(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy);
typedef int (*mx_unit_f_cfg_log)(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log);
typedef uint16_t (*mx_unit_f_get_status)(struct mx_unit_proto_t *unit);
typedef void (*mx_unit_f_cmd_attach)(struct mx_unit_proto_t *unit, uint16_t addr);
typedef void (*mx_unit_f_cmd_detach)(struct mx_unit_proto_t *unit, uint16_t addr);
typedef void (*mx_unit_f_cmd_transmit)(struct mx_unit_proto_t *unit, uint16_t addr);

struct mx_chan_t;

struct mx_unit_proto_t {
	const char *name;

	mx_unit_f_create create;
	mx_unit_f_create_nodev create_nodev;
	mx_unit_f_shutdown shutdown;
	mx_unit_f_reset reset;
	mx_unit_f_cfg_phy cfg_phy;
	mx_unit_f_cfg_log cfg_log;
	mx_unit_f_get_status get_status;
	mx_unit_f_cmd_attach cmd_attach;
	mx_unit_f_cmd_detach cmd_detach;
	mx_unit_f_cmd_transmit cmd_transmit;

	// physical
	int type;
	int phy_num;
	int dir;
	int used;

	// logical
	int log_num;
	int protocol;
	int attached;

	pthread_mutex_t status_mutex;
	uint16_t status;

	struct mx_chan_t *chan;

	pthread_t worker;
	pthread_mutex_t transmit_mutex;
	pthread_mutex_t worker_mutex;
	pthread_cond_t worker_cond;
	int worker_cmd;
	int worker_addr;
};

struct mx_cmd_int_t {
	int int_no_line;
	int int_line_not_attached;
	int int_line_attached;
	int int_line_busy;
};

struct mx_int_t {
	unsigned unit_n;
	unsigned interrupt;
	struct mx_int_t *next;
};

struct mx_chan_t {
	struct chan_proto_t proto;

	int confset;
	struct mx_unit_proto_t *lline[MX_MAX_DEVICES];
	struct mx_unit_proto_t *pline[MX_MAX_DEVICES];

	struct mx_int_t *int_head;
	struct mx_int_t *int_tail;
	int int_reported;
	pthread_mutex_t int_mutex;
};

// channel commands: always IN, bits 0..2 = 0, bits 3..4 = command
enum mx_cmd_chan_e {
	MX_CMD_RESET	= 0b00,
	MX_CMD_EXISTS	= 0b10,
	MX_CMD_INTSPEC	= 0b01,
	MX_CMD_INVALID	= 0b11,
};

// general and line commands: bit 3: 0=OUT/1=IN, bits 1-3: command bits 0-2
enum mx_cmd_e {
	MX_CMD_ERR_0		= 0b0000,
	MX_CMD_TEST			= 0b0001,
	MX_CMD_ATTACH		= 0b0010,
	MX_CMD_STATUS		= 0b0011,
	MX_CMD_TRANSMIT		= 0b0100,
	MX_CMD_SETCFG		= 0b0101,
	MX_CMD_ERR_6		= 0b0110,
	MX_CMD_ERR_7		= 0b0111,
	MX_CMD_CHAN			= 0b1000,
	MX_CMD_INTRQ		= 0b1001,
	MX_CMD_DETACH		= 0b1010,
	MX_CMD_CANCEL		= 0b1011,
	MX_CMD_ERR_C		= 0b1100,
	MX_CMD_ERR_D		= 0b1101,
	MX_CMD_ERR_E		= 0b1110,
	MX_CMD_ERR_F		= 0b1111,
};

// multix interrupts
enum mx_int_e {
	MX_INT_INIEA = 0,	// + interrupt no longer valid
	// special
	MX_INT_INSKA = 1,	// - channel faulty
	MX_INT_IWYZE = 2,	// + channel reset successfully
	MX_INT_IWYTE = 3,	// - channel test finished
	// general
	MX_INT_INKON = 4,	// + 'set configuration' rejected (configuration error, out of memory, configuration already set)
	MX_INT_IUKON = 5,	// + 'set configuration' finished successfully
	MX_INT_INKOT = 6,	// - 'set configuration' unsuccessfull (transmission errors)
	// line
	MX_INT_ISTRE = 7,	// 'report status' OK
	MX_INT_INSTR = 8,	// - 'report status' rejected (previous 'report status' being executed)
	MX_INT_INKST = 9,	// + 'report status' for non existent line
	MX_INT_IDOLI = 10,	// + 'attach line' OK
	MX_INT_INDOL = 11,	// + 'attach line' rejected (errors in field, line already attached, previous 'attach line' beind excecuted)
	MX_INT_INKDO = 12,	// + 'attach line' for non existent line
	MX_INT_IETRA = 13,	// + 'transmit' OK
	MX_INT_INTRA = 14,	// + 'transmit' rejected (errors in field, line not attached, previous transmission ongoing)
	MX_INT_INKTR = 15,	// + 'transmit' for non existent line
	MX_INT_ITRER = 16,	// + 'transmit' finished with error (parity or other)
	MX_INT_ITRAB = 19,	// + 'transmit' cancelled (as ordered by 'cancel transmission')
	MX_INT_IABTR = 20,	// + 'cancel transmission' OK
	MX_INT_INABT = 21,	// + 'cancel transmission' while no transmission
	MX_INT_INKAB = 22,	// + 'cancel transmission' for nonexistent line
	MX_INT_IODLI = 23,	// + 'detach line' OK
	MX_INT_INODL = 24,	// + 'detach line' for a line with ongoing transmission
	MX_INT_INKOD = 25,	// + 'detach line' for non existent line
	MX_INT_INPAO = 32,	// - mera-multix transmission failure
	MX_INT_IPARE = 33,	// - mera-multix parity error
	MX_INT_IOPRU = 34,	// operator request
	MX_INT_IEPS0 = 35,	// unknown control command, code=0
	MX_INT_IEPS6 = 36,	// unknown control command, code=6
	MX_INT_IEPS7 = 37,	// unknown control command, code=7
	MX_INT_IEPS8 = 38,	// unknown control command, code=8
	MX_INT_IEPSC = 39,	// unknown control command, code=C
	MX_INT_IEPSD = 40,	// unknown control command, code=D
	MX_INT_IEPSE = 41,	// unknown control command, code=E
	MX_INT_IEPSF = 42	// unknown control command, code=F
};

// logical line protocols
enum mx_log_proto_e {
	MX_PROTO_PUNCH_READER		= 0,
	MX_PROTO_PUNCHER			= 1,
	MX_PROTO_TERMINAL			= 2,
	MX_PROTO_SOM_PUNCH_READER	= 3,
	MX_PROTO_SOM_PUNCHER		= 4,
	MX_PROTO_SOM_TERMINAL		= 5,
	MX_PROTO_WINCHESTER			= 6,
	MX_PROTO_MTAPE				= 7,
	MX_PROTO_FLOPPY				= 8,
	MX_PROTO_TTY_ITWL			= 9, // teleks
	MX_PROTO_MAX				= 10
};

// physical line direction
enum mx_phy_direction_e {
	MX_DIR_OUTPUT		= 0b100,
	MX_DIR_INPUT		= 0b010,
	MX_DIR_HALF_DUPLEX	= 0b110,
	MX_DIR_FULL_DUPLEX	= 0b111
};

// physical line types
enum mx_phy_type_e {
	MX_PHY_USART_ASYNC	= 0,
	MX_PHY_8255			= 1,
	MX_PHY_USART_SYNC	= 2,
	MX_PHY_WINCHESTER	= 3,
	MX_PHY_MTAPE		= 4,
	MX_PHY_FLOPPY		= 5,
	MX_PHY_MAX			= 6
};

// "set configuration" return field values
enum mx_setconf_errors_e {
	MX_SC_E_OK				= -1,// everything went fine. (it's not multix constant, it's em400 indicator)
	MX_SC_E_CONFSET			= 0, // configuration already set
    MX_SC_E_NUMLINES		= 1, // wrong number of physical or logical lines
    MX_SC_E_DEVTYPE			= 2, // unknown device type in physical line description
    MX_SC_E_DIR				= 3, // unknown transmission direction
    MX_SC_E_PHY_INCOMPLETE	= 4, // incomplete physical line description
    MX_SC_E_PROTO_MISSING	= 5, // missing protocol
    MX_SC_E_PHY_UNUSED		= 6, // physical line is not used
    MX_SC_E_DIR_MISMATCH	= 7, // device vs. protocol transmission dricetion mismatch
    MX_SC_E_PHY_BUSY		= 8, // physical line is busy
    MX_SC_E_NOMEM			= 9, // memory exhausted
    MX_SC_E_PROTO_MISMATCH	= 10,// protocol vs. physical line type mismatch
    MX_SC_E_PROTO_PARAMS	= 11 // wrong protocol parameters
};

// "get status" bits
enum mx_get_status_e {
	MX_STATUS_ATTACHED		= 0b0000000100000000, // line is attached
	MX_STATUS_OPR			= 0b0000000010000000, // operator called
	MX_STATUS_PARITY		= 0b0000000001000000, // parity error
	MX_STATUS_RECV_EOT		= 0b0000000000100000, // EOT received
	MX_STATUS_RECV			= 0b0000000000001000, // receive ongoing
	MX_STATUS_RECV_STARTED	= 0b0000000000000100, // receive started
	MX_STATUS_SEND			= 0b0000000000000010, // send ongoing
	MX_STATUS_SEND_STARTED	= 0b0000000000000001, // send started

	MX_STATUS_NOTRANS		= 0b0000000011111111, // em400 internal: clear transmission
};

// --- cf: set configuration -------------------------------------------------

// physical line
struct mx_cf_sc_pl {
	int dir;
	int used;
	int type;
	int count;

	int offset;					// physical line offset at which this phy block starts
	int *logical_configured;	// array with indicators saying that physical line has been configured as logical
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
	int err_code;
	int err_line;
	struct mx_cf_sc_pl *pl;
	struct mx_cf_sc_ll *ll;
};

// -----------------------------------------------------------------------

struct mx_unit_proto_t * mx_unit_proto_get_by_name(struct mx_unit_proto_t *proto, char *name);
struct mx_unit_proto_t * mx_unit_proto_get_by_type(struct mx_unit_proto_t *proto, int type);
struct chan_proto_t * mx_create(struct cfg_unit *units);
void mx_shutdown(struct chan_proto_t *chan);
void mx_reset(struct chan_proto_t *chan);
int mx_cmd(struct chan_proto_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg);
void * mx_unit_worker(void *th_id);

void mx_int(struct mx_chan_t *chan, int unit_n, int interrupt);
void mx_int_enq(struct mx_chan_t *chan, struct mx_int_t *mx_int);
void mx_int_preq(struct mx_chan_t *chan, struct mx_int_t *mx_int);
void mx_int_setq(struct mx_chan_t *chan, struct mx_int_t *mx_int);
struct mx_int_t * mx_int_deq(struct mx_chan_t *chan);

void mx_status_set(struct mx_unit_proto_t *unit, uint16_t status);
void mx_status_clear(struct mx_unit_proto_t *unit, uint16_t status);

struct mx_cf_sc_pl * mx_decode_cf_find_phy(struct mx_cf_sc_pl *phys, int count, int id);
int mx_decode_cf_phy(int addr, struct mx_cf_sc_pl *phy, int offset);
int mx_decode_cf_log(int addr, struct mx_cf_sc_ll *log, struct mx_cf_sc_pl *phys, int phy_count);
int mx_decode_cf_sc(int addr, struct mx_cf_sc *cf);
void mx_free_cf_sc(struct mx_cf_sc *cf);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
