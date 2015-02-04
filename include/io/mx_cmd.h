//  Copyright (c) 2013-2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_CMD_H
#define MX_CMD_H

#include "io/mx.h"

// channel commands: always IN, bits 0..2 = 0, bits 3..4 = command
enum mx_cmd_chan {
	MX_CMD_RESET	= 0b00,
	MX_CMD_INTSPEC	= 0b01,
	MX_CMD_EXISTS	= 0b10,
	MX_CMD_INVALID	= 0b11,
};

// general and line commands: bit 3: 0=OUT/1=IN, bits 2-0: command bits 0-2
enum mx_cmd {
	MX_CMD_ERR_0		= 0b0000,
	MX_CMD_TEST			= 0b0001,
	MX_CMD_ATTACH		= 0b0010,
	MX_CMD_STATUS		= 0b0011,
	MX_CMD_TRANSMIT		= 0b0100,
	MX_CMD_SETCFG		= 0b0101,
	MX_CMD_ERR_6		= 0b0110,
	MX_CMD_ERR_7		= 0b0111,
	MX_CMD_CHAN			= 0b1000,
	MX_CMD_ERR_8		= 0b1000,
	MX_CMD_INTRQ		= 0b1001,
	MX_CMD_DETACH		= 0b1010,
	MX_CMD_ABORT		= 0b1011,
	MX_CMD_ERR_C		= 0b1100,
	MX_CMD_ERR_D		= 0b1101,
	MX_CMD_ERR_E		= 0b1110,
	MX_CMD_ERR_F		= 0b1111,
	MX_CMD_QUIT			= 16, // em400 internal: stop emulation
	MX_CMD_NONE			= 17, // em400 internal: no command
};

// "get status" bits
enum mx_status {
	MX_STATUS_ATTACHED		= 0b0000000100000000, // line is attached
	MX_STATUS_OPR			= 0b0000000010000000, // operator called
	MX_STATUS_PARITY		= 0b0000000001000000, // parity error
	MX_STATUS_RECV_EOT		= 0b0000000000100000, // EOT received
	MX_STATUS_RECV			= 0b0000000000001000, // receive ongoing
	MX_STATUS_RECV_STARTED	= 0b0000000000000100, // receive started
	MX_STATUS_SEND			= 0b0000000000000010, // send ongoing
	MX_STATUS_SEND_STARTED	= 0b0000000000000001, // send started
};

struct mx;

void mx_cmd_int_requeue(struct mx *chan);
void mx_cmd_intspec(struct mx *chan, uint16_t *r_arg);
void mx_cmd_illegal(struct mx *chan, int llinen, int intr);
void mx_cmd_test(struct mx *chan);
void mx_cmd_confset(struct mx *chan, uint16_t addr);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
