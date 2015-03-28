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

#ifndef MX_CMDS_H
#define MX_CMDS_H

/*	general and line commands:
	0bABBB
	  |`-> command bits 0-2
	  `--> 0=OUT / 1=IN
*/
enum mx_cmd {
	MX_CMD_ERR_0		= 0b0000,	// (invalid command)
	MX_CMD_TEST			= 0b0001,	// GENERAL: run test (not implemented)
	MX_CMD_ATTACH		= 0b0010,	// LINE: attach
	MX_CMD_STATUS		= 0b0011,	// LINE: get status
	MX_CMD_TRANSMIT		= 0b0100,	// LINE: transmit
	MX_CMD_SETCFG		= 0b0101,	// GENERAL: set configuration
	MX_CMD_ERR_6		= 0b0110,	// (invalid command)
	MX_CMD_ERR_7		= 0b0111,	// (invalid command)
	MX_CMD_CHAN			= 0b1000,	// (channel commands, see below)
	MX_CMD_ERR_8		= 0b1000,	// (invalid command)
	MX_CMD_INTRQ		= 0b1001,	// GENERAL: requeue interrupt
	MX_CMD_DETACH		= 0b1010,	// LINE: detach
	MX_CMD_ABORT		= 0b1011,	// LINE: abort transmission
	MX_CMD_ERR_C		= 0b1100,	// (invalid command)
	MX_CMD_ERR_D		= 0b1101,	// (invalid command)
	MX_CMD_ERR_E		= 0b1110,	// (invalid command)
	MX_CMD_ERR_F		= 0b1111,	// (invalid command)
};

// channel commands: always IN, bits 0..2 = 0, bits 3..4 = command
enum mx_cmd_chan {
	MX_CMD_RESET	= 0b00,	// software reset
	MX_CMD_INTSPEC	= 0b01,	// get interrupt specification
	MX_CMD_EXISTS	= 0b10,	// check if channel exists
	MX_CMD_INVALID	= 0b11,	// (invalid channel command)
};

extern const char *mx_chan_cmd_names[];
extern const char *mx_cmd_names[];

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
