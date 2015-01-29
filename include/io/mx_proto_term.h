//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_PROTO_TERM_H
#define MX_PROTO_TERM_H

#include "io/mx_proto.h"

struct mx_proto_term {
	struct mx_proto base;

};

enum mx_term_attach_opts {
	MX_TERM_ATTACH_WATCH_EOT	= 0b10000000,
	MX_TERM_ATTACH_NO_PARITY	= 0b01000000,
	MX_TERM_ATTACH_ODD_PARITY	= 0b00100000,
	MX_TERM_ATTACH_8BITS		= 0b00010000,
	MX_TERM_ATTACH_XON_XOFF		= 0b00001000,
	MX_TERM_ATTACH_BS_CAN		= 0b00000100,
	MX_TERM_ATTACH_TO_UPPER		= 0b00000010,
	MX_TERM_ATTACH_WATCH_CALL	= 0b00000001,
};

enum mx_term_transmit_opts {
	MX_TERM_TX_SEND_BY_SIZE		= 0b10000000,
	MX_TERM_TX_SEND_BY_EOT_EXCL	= 0b01000000,
	MX_TERM_TX_SEND_BY_EOT_INCL	= 0b00100000,
	MX_TERM_TX_RECV_BY_SIZE		= 0b00010000,
	MX_TERM_TX_RECV_BY_EOT_EXCL	= 0b00001000,
	MX_TERM_TX_RECV_BY_EOT_INCL	= 0b00000100,
	MX_TERM_TX_ECHO				= 0b00000010,
	MX_TERM_TX_PROMPT			= 0b00000001,
};

enum mx_term_transmit_status {
	MX_TERM_TX_STATUS_TIMEOUT		= 0b10000000,
	MX_TERM_TX_STATUS_OPRQ			= 0b01000000,
	MX_TERM_TX_STATUS_FAILURE		= 0b00100000,
	MX_TERM_TX_STATUS_PREMATURE_EOF	= 0b00010000,
	MX_TERM_TX_STATUS_PARITY_ERROR	= 0b00001000,
	MX_TERM_TX_STATUS_ERROR			= 0b00000100,
	MX_TERM_TX_STATUS_EOT			= 0b00000001,
};

enum mx_term_text_proc {
	MX_TERM_TEXT_PROC_NONE		= 0,
	MX_TERM_TEXT_PROC_EDITOR	= 2,
};

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
