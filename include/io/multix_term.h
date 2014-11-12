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

#ifndef _MULTIX_TERMINAL_H_
#define _MULTIX_TERMINAL_H_

#include <inttypes.h>
#include <pthread.h>

#include "io/io.h"
#include "io/multix.h"

enum mx_term_attach_opts_e {
	MX_TERM_ATTACH_WATCH_EOT	= 0b10000000,
	MX_TERM_ATTACH_NO_PARITY	= 0b01000000,
	MX_TERM_ATTACH_ODD_PARITY	= 0b00100000,
	MX_TERM_ATTACH_8BITS		= 0b00010000,
	MX_TERM_ATTACH_XON_XOFF		= 0b00001000,
	MX_TERM_ATTACH_BS_CAN		= 0b00000100,
	MX_TERM_ATTACH_TO_UPPER		= 0b00000010,
	MX_TERM_ATTACH_WATCH_CALL	= 0b00000001,
};

enum mx_term_transmit_opts_e {
	MX_TERM_TX_SEND_BY_SIZE		= 0b10000000,
	MX_TERM_TX_SEND_BY_EOT_EXCL	= 0b01000000,
	MX_TERM_TX_SEND_BY_EOT_INCL	= 0b00100000,
	MX_TERM_TX_RECV_BY_SIZE		= 0b00010000,
	MX_TERM_TX_RECV_BY_EOT_EXCL	= 0b00001000,
	MX_TERM_TX_RECV_BY_EOT_INCL	= 0b00000100,
	MX_TERM_TX_ECHO				= 0b00000010,
	MX_TERM_TX_PROMPT			= 0b00000001,
};

enum mx_term_transmit_status_e {
	MX_TERM_TX_STATUS_TIMEOUT		= 0b10000000,
	MX_TERM_TX_STATUS_OPRQ			= 0b01000000,
	MX_TERM_TX_STATUS_FAILURE		= 0b00100000,
	MX_TERM_TX_STATUS_PREMATURE_EOF	= 0b00010000,
	MX_TERM_TX_STATUS_PARITY_ERROR	= 0b00001000,
	MX_TERM_TX_STATUS_ERROR			= 0b00000100,
	MX_TERM_TX_STATUS_EOT			= 0b00000001,
};

enum mx_term_text_proc_e {
	MX_TERM_TEXT_PROC_NONE		= 0,
	MX_TERM_TEXT_PROC_EDITOR	= 2,
};

struct mx_term_cf_attach_t {
	int opts;
	char eot_mark;
	char call_mark;
	int text_proc;
	int text_proc_params;
};

struct mx_term_cf_transmit_t {
	int opts;
	int timeout;

	int send_len;
	uint16_t send_buf_addr;
	char send_eot_char;
	int send_start_byte;
	int send_nb;

	int recv_len;
	uint16_t recv_buf_addr;
	int recv_start_byte;
	int recv_nb;
	char recv_eot_char;
	char recv_eot_char2;

	char prompt_text[5];

	int bytes_sent;
	int bytes_awaiting;
	int bytes_transmitted;
	int status;
};

struct mx_unit_terminal_t {
	struct mx_unit_proto_t proto;
	struct term_t *term;
};

struct mx_unit_proto_t * mx_term_create(struct cfg_arg *args);
struct mx_unit_proto_t * mx_term_create_nodev();
void mx_term_connect(struct mx_unit_terminal_t *unit);
void mx_term_disconnect(struct mx_unit_terminal_t *unit);
void mx_term_shutdown(struct mx_unit_proto_t *unit);
void mx_term_reset(struct mx_unit_proto_t *unit);
int mx_term_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy);
int mx_term_cfg_log(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log);
uint16_t mx_term_get_status(struct mx_unit_proto_t *unit);

void mx_term_cmd_attach(struct mx_unit_proto_t *unit, uint16_t addr);
void mx_term_cmd_detach(struct mx_unit_proto_t *unit, uint16_t addr);
void mx_term_cmd_transmit(struct mx_unit_proto_t *unit, uint16_t addr);

struct mx_term_cf_attach_t * mx_term_cf_attach_decode(int addr);
struct mx_term_cf_transmit_t * mx_term_cf_transmit_decode(int addr);
void mx_term_cf_attach_free(struct mx_term_cf_attach_t *cf);
void mx_term_cf_transmit_free(struct mx_term_cf_transmit_t *cf);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
