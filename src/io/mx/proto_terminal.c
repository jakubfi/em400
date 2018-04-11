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

#include "io/mx/mx.h"
#include "io/mx/line.h"
#include "io/mx/irq.h"

#include "log.h"

enum mx_term_attach_opts {
	MX_TERM_WATCH_EOT	= 0b10000000,
	MX_TERM_NO_PARITY	= 0b01000000,
	MX_TERM_ODD_PARITY	= 0b00100000,
	MX_TERM_8BITS		= 0b00010000,
	MX_TERM_XON_XOFF	= 0b00001000,
	MX_TERM_BS_CAN		= 0b00000100,
	MX_TERM_TO_UPPER	= 0b00000010,
	MX_TERM_WATCH_OPRQ	= 0b00000001,
};

enum mx_term_transmit_flags {
	MX_TERM_TX_BY_SIZE		= 0b10000000,
	MX_TERM_TX_BY_EOT_EXCL	= 0b01000000,
	MX_TERM_TX_BY_EOT_INCL	= 0b00100000,
	MX_TERM_RX_BY_SIZE		= 0b00010000,
	MX_TERM_RX_BY_EOT_EXCL	= 0b00001000,
	MX_TERM_RX_BY_EOT_INCL	= 0b00000100,
	MX_TERM_TX_ECHO			= 0b00000010,
	MX_TERM_TX_PROMPT		= 0b00000001,
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

enum mx_term_text_proc {
	MX_TERM_TEXT_PROC_NONE		= 0,
	MX_TERM_TEXT_PROC_EDITOR	= 2,
};

enum mx_term_parity {
	MX_TERM_PARITY_NONE,
	MX_TERM_PARITY_EVEN,
	MX_TERM_PARITY_ODD,
};

static const char *mx_term_parity_names[] = {
	"none", "even", "odd"
};

struct proto_terminal_cf_transmit {
	uint8_t flags;
	uint8_t timeout;

	uint16_t tx_len;
	uint16_t tx_addr;
	char tx_eot_ch;
	unsigned tx_byte_pos;
	unsigned tx_nb;

	uint16_t rx_len;
	uint16_t rx_addr;
	unsigned rx_byte_pos;
	unsigned rx_nb;
	unsigned rx_eot_ch;
	unsigned rx_eot_ch2;
	char prompt[5];
};

struct proto_terminal_data {
	int watch_eof;
	int parity;
	int eight_bits;
	int xon_xoff;
	int bs_can;
	int toupper;
	int watch_oprq;
	char eot_char;
	char oprq_char;
	int proc;
	uint16_t proc_params;
	struct proto_terminal_cf_transmit transmit;
};

// -----------------------------------------------------------------------
int mx_terminal_init(struct mx_line *pline, uint16_t *data)
{
	struct proto_winchester_data *pd = calloc(1, sizeof(struct proto_terminal_data));
	if (!pd) {
		return MX_SC_E_NOMEM;
	}

	pline->proto_data = pd;

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
void mx_terminal_destroy(struct mx_line *pline)
{
	if (!pline || !pline->proto_data) return;
	free(pline->proto_data);
	pline->proto_data = NULL;
}

// -----------------------------------------------------------------------
static int mx_terminal_att_decode(uint16_t *data, struct proto_terminal_data *pd)
{
	pd->watch_eof	= (data[0] & 0b1000000000000000) >> 15;
	pd->parity		=!(data[0] & 0b0100000000000000) ? MX_TERM_PARITY_NONE :
					  (data[0] & 0b0010000000000000) ? MX_TERM_PARITY_ODD : MX_TERM_PARITY_EVEN;
	pd->eight_bits	= (data[0] & 0b0001000000000000) >> 12;
	pd->xon_xoff	= (data[0] & 0b0000100000000000) >> 11;
	pd->bs_can		= (data[0] & 0b0000010000000000) >> 10;
	pd->toupper		= (data[0] & 0b0000001000000000) >> 9;
	pd->watch_oprq	= (data[0] & 0b0000000100000000) >> 8;
	pd->eot_char	= (data[0] & 0b0000000011111111) >> 0;
	pd->oprq_char	= (data[1] & 0b1111111100000000) >> 8;
	pd->proc		= (data[1] & 0b0000000011111111) >> 0;
	pd->proc_params	= data[2];

	LOG(L_TERM, "parity %s, EOT: #%2x, OPRQ: #%2x, text proc: %i (params: 0x%04x), flags: %s%s%s%s%s%s",
		mx_term_parity_names[pd->parity],
		pd->eot_char,
		pd->oprq_char,
		pd->proc,
		pd->proc_params,
		pd->watch_eof ? "WATCH_EOF " : "",
		pd->eight_bits ? "8BIT " : "",
		pd->xon_xoff ? "XON/XOFF " : "",
		pd->bs_can ? "BS/CAN " : "",
		pd->toupper ? "TOUPPER " : "",
		pd->watch_oprq ? "WATCH_OPRQ " : ""
	);

	return 0;
}

// -----------------------------------------------------------------------
static int mx_terminal_trans_decode(uint16_t *data, struct proto_terminal_data *pd)
{
	pd->transmit.flags		= (data[0] & 0b1111111100000000) >> 8;
	pd->transmit.timeout	= (data[0] & 0b0000000011111111);
	pd->transmit.tx_len		= data[1];
	pd->transmit.tx_addr	= data[2];
	pd->transmit.tx_eot_ch	= (data[3] & 0b1111111100000000) >> 8;
	pd->transmit.tx_byte_pos= (data[3] & 0b0000000001000000) >> 6;
	pd->transmit.tx_nb		= (data[3] & 0b0000000000001111);
	pd->transmit.rx_len		= data[4];
	pd->transmit.rx_addr	= data[5];
	pd->transmit.rx_byte_pos= (data[6] & 0b0000000001000000) >> 6;
	pd->transmit.rx_nb		= (data[6] & 0b0000000000001111);
	pd->transmit.rx_eot_ch	= (data[7] & 0b1111111100000000) >> 8;
	pd->transmit.rx_eot_ch2	= (data[7] & 0b0000000011111111);
	pd->transmit.prompt[0]	= (data[8] & 0b1111111100000000) >> 8;
	pd->transmit.prompt[1]	= (data[8] & 0b0000000011111111);
	pd->transmit.prompt[2]	= (data[9] & 0b1111111100000000) >> 8;
	pd->transmit.prompt[3]	= (data[9] & 0b0000000011111111);
	pd->transmit.prompt[4]	= '\0';

	return 0;
}

// -----------------------------------------------------------------------
int mx_terminal_attach(struct mx_line *line, uint16_t *cmd_data)
{
	int irq;
	struct proto_terminal_data *proto_data = line->proto_data;

	if (mx_terminal_att_decode(cmd_data, proto_data)) {
		irq = MX_IRQ_INDOL;
	} else {
		pthread_mutex_lock(&line->status_mutex);
		line->status |= MX_LSTATE_ATTACHED;
		pthread_mutex_unlock(&line->status_mutex);
		irq = MX_IRQ_IDOLI;
	}

	return irq;
}

// -----------------------------------------------------------------------
int mx_terminal_detach(struct mx_line *line, uint16_t *cmd_data)
{
	pthread_mutex_lock(&line->status_mutex);
	line->status &= ~MX_LSTATE_ATTACHED;
	pthread_mutex_unlock(&line->status_mutex);
	return MX_IRQ_IODLI;
}

// -----------------------------------------------------------------------
int mx_terminal_transmit(struct mx_line *line, uint16_t *cmd_data)
{
	int irq;
	struct proto_terminal_data *proto_data = line->proto_data;
	//const struct mx_cmd *cmd = line->proto->cmd + MX_CMD_TRANSMIT;

	// check if there is a device connected
	if (!line->dev || !line->dev_data) {
		irq = MX_IRQ_INTRA;
		goto fin;
	}

	if (mx_terminal_trans_decode(cmd_data, proto_data)) {
		irq = MX_IRQ_INTRA;
		goto fin;
	}

	// TODO: do real transmission
	irq = MX_IRQ_IETRA;

fin:
	return irq;
}

// -----------------------------------------------------------------------
const struct mx_proto mx_drv_terminal = {
	.name = "terminal",
	.dir = MX_DIR_INPUT | MX_DIR_OUTPUT,
	.phy_types = { MX_PHY_USART_ASYNC, MX_PHY_8255, -1 },
	.init = mx_terminal_init,
	.destroy = mx_terminal_destroy,
	.cmd = {
		[MX_CMD_ATTACH] = { 3, 0, 0, mx_terminal_attach },
		[MX_CMD_TRANSMIT] = { 10, 10, 4, mx_terminal_transmit },
		[MX_CMD_DETACH] = { 0, 0, 0, mx_terminal_detach },
		[MX_CMD_ABORT] = { 0, 0, 0, NULL },
	}
};

// vim: tabstop=4 shiftwidth=4 autoindent
