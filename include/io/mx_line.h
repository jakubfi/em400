//  Copyright (c) 2013-2015 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_LINE_H
#define MX_LINE_H

#include "log.h"

#include "io/mx_task.h"
#include "io/mx_proto.h"

#include "io/dev/dev.h"

#define MX_LINE_MAX 32

// physical line direction
enum mx_phy_dir {
	MX_DIR_NONE			= 0b000,
	MX_DIR_INPUT		= 0b010,
	MX_DIR_OUTPUT		= 0b100,
	MX_DIR_HALF_DUPLEX	= 0b110,
	MX_DIR_FULL_DUPLEX	= 0b111,
	MX_DIR_MAX
};

// physical line types
enum mx_phy_type {
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
	// nothing on the LSB
	MX_SC_E_CONFSET			= 0, // configuration already set
	MX_SC_E_NUMLINES		= 1, // wrong number of physical or logical lines
	// LSB: physical line number
	MX_SC_E_DEVTYPE			= 2, // unknown device type in physical line description
	MX_SC_E_DIR				= 3, // wrong transmission direction
	MX_SC_E_PHY_INCOMPLETE	= 4, // incomplete physical line description
	// LSB: logical line number
	MX_SC_E_PROTO_MISSING	= 5, // protocol not available
	MX_SC_E_PHY_UNUSED		= 6, // physical line is not used
	MX_SC_E_DIR_MISMATCH	= 7, // device vs. protocol transmission direction mismatch
	MX_SC_E_PHY_USED		= 8, // physical line is already used
	MX_SC_E_NOMEM			= 9, // memory exhausted
	MX_SC_E_PROTO_MISMATCH	= 10, // protocol vs. physical line type mismatch
	MX_SC_E_PROTO_PARAMS	= 11, // wrong protocol parameters
	MX_SC_E_MAX,
	MX_SC_E_OK, // em400: OK
};

// line status
enum mx_line_status {
	MX_LSTATE_NONE			= 0,
	MX_LSTATE_ATTACHED		= 1 << 0,	// line attached
	MX_LSTATE_TRANS			= 1 << 1,	// transmission active
	MX_LSTATE_2				= 1 << 2,	// (unused)
	MX_LSTATE_3				= 1 << 3,	// (unused)
	MX_LSTATE_TASK_XOFF		= 1 << 4,	// task suspended due to XOFF
    MX_LSTATE_TRANS_XOFF	= 1 << 5,	// transmission suspended due to XOFF
	MX_LSTATE_TRANS_LAST	= 1 << 6,	// transmission of a last fragment
	MX_LSTATE_7				= 1 << 7,	// (unused)
	MX_LSTATE_SEND_START	= 1 << 8,	// send started
	MX_LSTATE_SEND_RUN		= 1 << 9,	// send running
	MX_LSTATE_RECV_START	= 1 << 10,	// receive started
	MX_LSTATE_RECV_RUN		= 1 << 11,	// receive running
	MX_LSTATE_CAN_STOP		= 1 << 12,	// stop after CAN (protocol 5)
	MX_LSTATE_STOP_CHAR		= 1 << 13,	// stop character received
	MX_LSTATE_PARITY_ERR	= 1 << 14,	// parity error
	MX_LSTATE_OPRQ			= 1 << 15,	// OPRQ
};

struct mx_line {
	// physical line properties
	int used;			// line is used (1=physical 2=logical)
	int dir;			// data direction
	int type;			// line type

	// logical line properties
	uint16_t status;	// line status
	const struct mx_proto *proto;   // line protocol
	void * proto_data;	// protocol data

	// actual device
	const struct dev_drv *device;
	void *dev_obj;

	// task
	struct mx_task task[MX_TASK_MAX];
	LOG_ID_DEF;
};

const char * mx_line_dir_name(unsigned i);
const char * mx_line_type_name(unsigned i);
const char * mx_line_sc_err_name(unsigned i);
int mx_line_conf_phy(struct mx_line *line, uint16_t data);
int mx_line_conf_log(struct mx_line *line, uint16_t *data);
void mx_line_deconfigure(struct mx_line *line);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
