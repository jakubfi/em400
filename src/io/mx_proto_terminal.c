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

#include <stdlib.h>
#include <inttypes.h>

#include "log.h"
#include "io/mx_irq.h"
#include "io/mx_line.h"
#include "io/mx_proto.h"
#include "io/mx_proto_common.h"

enum mx_term_attach_opts {
	MX_TERM_WATCH_EOT	= 0b10000000,
	MX_TERM_NO_PARITY	= 0b01000000,
	MX_TERM_ODD_PARITY	= 0b00100000,
	MX_TERM_8BITS		= 0b00010000,
	MX_TERM_XON_XOFF	= 0b00001000,
	MX_TERM_BS_CAN		= 0b00000100,
	MX_TERM_TO_UPPER	= 0b00000010,
	MX_TERM_WATCH_CALL	= 0b00000001,
};

enum mx_term_text_proc {
	MX_TERM_TEXT_PROC_NONE		= 0,
	MX_TERM_TEXT_PROC_EDITOR	= 2,
};

struct proto_terminal_data {
};

// -----------------------------------------------------------------------
int mx_proto_terminal_conf(struct mx_line *line, uint16_t *data)
{
	// No protocol configuration
	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
void mx_proto_terminal_free(struct mx_line *line)
{
	free(line->proto_data);
	line->proto_data = NULL;
}

// -----------------------------------------------------------------------
uint8_t mx_proto_terminal_attach_start(struct mx_line *line, int *irq, uint16_t *data)
{
	if ((line->status & MX_LSTATE_ATTACHED)) {
		*irq = MX_IRQ_INDOL;
	}

	line->status |= MX_LSTATE_ATTACHED;
	*irq = MX_IRQ_IDOLI;

	return MX_COND_NONE;
}

// -----------------------------------------------------------------------
int mx_proto_terminal_phy_types[] = { MX_PHY_USART_ASYNC, -1 };

struct mx_proto mx_proto_terminal = {
	.enabled = 1,
	.name = "terminal",
	.dir = MX_DIR_INPUT | MX_DIR_OUTPUT,
	.phy_types = mx_proto_terminal_phy_types,
	.conf = mx_proto_terminal_conf,
	.free = mx_proto_terminal_free,
	.task = {
		{ 0, 0, 1, { mx_proto_status_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 0, 0, 0, { mx_proto_detach_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 0, 0, 0, { mx_proto_oprq_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 10, 10, 4, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 0, 0, 0, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 3, 0, 0, { mx_proto_terminal_attach_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
	}
};

// vim: tabstop=4 shiftwidth=4 autoindent
