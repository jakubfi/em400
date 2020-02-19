//  Copyright (c) 2013-2018 Jakub Filipowicz <jakubf@gmail.com>
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

/*	general and line commands as seen by MULTIX:
	0bABBB
	  |`-> command bits 0-2
	  `--> 0=OUT / 1=IN
*/
enum mx_command {
	MX_CMD_ERR_0	= 0x0,	// (invalid command)
	MX_CMD_TEST		= 0x1,	// GENERAL: run test (not implemented)
	MX_CMD_ATTACH	= 0x2,	// LINE: attach
	MX_CMD_STATUS	= 0x3,	// LINE: get status
	MX_CMD_TRANSMIT	= 0x4,	// LINE: transmit
	MX_CMD_SETCFG	= 0x5,	// GENERAL: set configuration
	MX_CMD_ERR_6	= 0x6,	// (invalid command)
	MX_CMD_ERR_7	= 0x7,	// (invalid command)
	MX_CMD_CHAN		= 0x8,	// (channel commands, see below)
	MX_CMD_ERR_8	= 0x8,	// (invalid line command)
	MX_CMD_REQUEUE	= 0x9,	// GENERAL: requeue interrupt
	MX_CMD_DETACH	= 0xa,	// LINE: detach
	MX_CMD_ABORT	= 0xb,	// LINE: abort transmission
	MX_CMD_ERR_C	= 0xc,	// (invalid command)
	MX_CMD_ERR_D	= 0xd,	// (invalid command)
	MX_CMD_ERR_E	= 0xe,	// (invalid command)
	MX_CMD_ERR_F	= 0xf,	// (invalid command)
	MX_CMD_CNT
};

/* channel commands as seen by MULTIX:
   (command is always IN, command bits 0-2 are always = 0)
   0bAA
     `-> bits 3-4 of the channel command
*/
enum mx_cmd_chan {
	MX_CHAN_CMD_RESET	= 0b00,	// software reset
	MX_CHAN_CMD_INTSPEC	= 0b01,	// get interrupt specification
	MX_CHAN_CMD_EXISTS	= 0b10,	// check if channel exists
	MX_CHAN_CMD_INVALID	= 0b11,	// (invalid channel command)
	MX_CHAN_CMD_CNT
};

const char * mx_get_cmd_name(unsigned i);
const char * mx_get_chan_cmd_name(unsigned i);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
