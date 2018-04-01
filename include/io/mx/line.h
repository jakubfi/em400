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
#include "utils/elst.h"
#include "io/mx/mx.h"

// physical line direction
enum mx_phy_dir {
	MX_DIR_NONE			= 0b000,
	MX_DIR_INPUT		= 0b010,
	MX_DIR_OUTPUT		= 0b100,
	MX_DIR_HALF_DUPLEX	= 0b110,
	MX_DIR_FULL_DUPLEX	= 0b111,
	MX_DIR_CNT
};

// physical line types
enum mx_phy_type {
	MX_PHY_USART_ASYNC	= 0,
	MX_PHY_8255			= 1,
	MX_PHY_USART_SYNC	= 2,
	MX_PHY_WINCHESTER	= 3,
	MX_PHY_MTAPE		= 4,
	MX_PHY_FLOPPY		= 5,
	MX_PHY_CNT
};

// protocols
enum mx_protocols {
	MX_PROTO_PUNCH_READER		= 0,
	MX_PROTO_PUNCHER			= 1,
	MX_PROTO_TERMINAL			= 2,
	MX_PROTO_SOM_PUNCH_READER	= 3,
	MX_PROTO_SOM_PUNCHER		= 4,
	MX_PROTO_SOM_TERMINAL		= 5,
	MX_PROTO_WINCHESTER			= 6,
	MX_PROTO_MTAPE				= 7,
	MX_PROTO_FLOPPY				= 8,
	MX_PROTO_CNT
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
	MX_SC_E_CNT,
	MX_SC_E_OK, // em400: OK
};

// line status
enum mx_line_status {
	MX_LSTATE_NONE			= 0,
	MX_LSTATE_SEND_START	= 1 << 0,	// send started
	MX_LSTATE_SEND_RUN		= 1 << 1,	// send running
	MX_LSTATE_RECV_START	= 1 << 2,	// receive started
	MX_LSTATE_RECV_RUN		= 1 << 3,	// receive running
	MX_LSTATE_CAN_STOP		= 1 << 4,	// stop after CAN (protocol 5)
	MX_LSTATE_STOP_CHAR		= 1 << 5,	// stop character received
	MX_LSTATE_PARITY_ERR	= 1 << 6,	// parity error
	MX_LSTATE_OPRQ			= 1 << 7,	// OPRQ
	MX_LSTATE_ATTACHED		= 1 << 8,	// line attached
	MX_LSTATE_TRANS			= 1 << 9,	// transmission active
	MX_LSTATE_5				= 1 << 10,	// (unused)
	MX_LSTATE_4				= 1 << 11,	// (unused)
	MX_LSTATE_TASK_XOFF		= 1 << 12,	// task suspended due to XOFF
	MX_LSTATE_TRANS_XOFF	= 1 << 13,	// transmission suspended due to XOFF
	MX_LSTATE_TRANS_LAST	= 1 << 14,	// transmission of a last fragment
	MX_LSTATE_0				= 1 << 15,	// (unused)
	// em400 specific: these are not sent to the cpu, and may or may not be in use
	// by the emulation depending on whether command is processed sync or async
	MX_LSTATE_ATTACH		= 1 << 16,	// 'attach' running
	MX_LSTATE_DETACH		= 1 << 17,	// 'detach' running
	MX_LSTATE_ABORT			= 1 << 19,	// 'abort' running
};

const char * mx_line_dir_name(unsigned i);
const char * mx_line_type_name(unsigned i);
const char * mx_line_sc_err_name(unsigned i);

const struct mx_proto* mx_proto_get(unsigned i);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
