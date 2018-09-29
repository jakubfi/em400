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
#include <strings.h>

#include "cfg.h"
#include "io/mx/line.h"
#include "io/mx/irq.h"
#include "io/mx/cmds.h"
#include "io/mx/event.h"
#include "utils/elst.h"
#include "log.h"

#define MAX_CMD_DATA_LEN 16

const char *mx_phy_type_names[] = {
	"USART async",
	"8255",
	"USART sync",
	"winchester",
	"magnetic tape",
	"floppy",
	"[invalid-phy-type]",
};

const char *mx_phy_dir_names[] = {
	"[none-0]",
	"[invalid-phy-dir-1]",
	"input",
	"[invalid-phy-dir-3]",
	"output",
	"[invalid-phy-dir-5]",
	"half-duplex",
	"full-duplex",
	"[invalid-phy-dir]",
};

const char * mx_protocol_names[] = {
	"punched paper tape reader",
	"paper tape puncher",
	"terminal",
	"SOM punched paper tape reader",
	"SOM paper tape puncher",
	"SOM terminal",
	"winchester",
	"magnetic tape",
	"floppy drive",
	"[invalid-protocol]"
};

const char *mx_sc_err_names[] = {
	"configuration already set",
	"wrong number of physical or logical lines",
	"unknown device type in physical line description",
	"wrong transmission direction",
	"incomplete physical line description",
	"protocol not available",
	"physical line is not used",
	"device vs. protocol transmission direction mismatch",
	"physical line is already used",
	"memory exhausted",
	"protocol vs. physical line type mismatch",
	"wrong protocol parameters",
	"[invalid-error]",
};

extern struct mx_proto mx_drv_winchester;
extern struct mx_proto mx_drv_floppy;
extern struct mx_proto mx_drv_terminal;
extern struct mx_proto mx_drv_punchrd;
extern struct mx_proto mx_drv_puncher;
extern struct mx_proto mx_drv_tape;

const struct mx_proto *mx_protocols[] = {
	&mx_drv_punchrd,
	&mx_drv_puncher,
	&mx_drv_terminal,
	NULL, // som punch reader
	NULL, // som puncher
	NULL, // som terminal
	&mx_drv_winchester,
	&mx_drv_tape,
	&mx_drv_floppy,
};

// -----------------------------------------------------------------------
const char * mx_line_dir_name(unsigned i)
{
	if (i < MX_DIR_CNT) {
		return mx_phy_dir_names[i];
	} else {
		return mx_phy_dir_names[MX_DIR_CNT];
	}
}

// -----------------------------------------------------------------------
const char * mx_line_type_name(unsigned i)
{
	if (i < MX_PHY_CNT) {
		return mx_phy_type_names[i];
	} else {
		return mx_phy_type_names[MX_PHY_CNT];
	}
}

// -----------------------------------------------------------------------
const char * mx_line_sc_err_name(unsigned i)
{
	if (i < MX_SC_E_CNT) {
		return mx_sc_err_names[i];
	} else {
		return mx_sc_err_names[MX_SC_E_CNT];
	}
}

// -----------------------------------------------------------------------
const struct mx_proto * mx_proto_get(unsigned i)
{
	if (i < MX_PROTO_CNT) {
		return mx_protocols[i];
	} else {
		return NULL;
	}
}

// -----------------------------------------------------------------------
static const struct mx_cmd_route {
	int irq_reject; // interrupt when line can't process the command
	int irq_no_line; // interrupt when line is not configured
	uint32_t fail_pos; // positive status mask (bits that need to be set for the command to be accepted)
	uint32_t fail_neg; // negative status mask (bits that need to be clear for the command to be accepted)
	uint32_t cmd_state; // state bits to set when starting command
} mx_cmd_routing[MX_CMD_CNT] = {
	[MX_CMD_ERR_0] = {0, 0, 0, 0, 0},
	[MX_CMD_TEST] = {0, 0, 0, 0, 0},
	[MX_CMD_ATTACH] = {
		.irq_reject = MX_IRQ_INDOL,
		.irq_no_line = MX_IRQ_INKDO,
		.fail_pos = MX_LSTATE_ATTACH | MX_LSTATE_ATTACHED,
		.fail_neg = MX_LSTATE_NONE,
		.cmd_state = MX_LSTATE_ATTACH,
	},
	[MX_CMD_STATUS] = {
		.irq_reject = MX_IRQ_INSTR,
		.irq_no_line = MX_IRQ_INKST,
		.fail_pos = MX_LSTATE_STATUS,
		.fail_neg = MX_LSTATE_NONE,
		.cmd_state = MX_LSTATE_STATUS,
	},
	[MX_CMD_TRANSMIT] = {
		.irq_reject = MX_IRQ_INTRA,
		.irq_no_line = MX_IRQ_INKTR,
		.fail_pos = MX_LSTATE_TRANS,
		.fail_neg = MX_LSTATE_ATTACHED,
		.cmd_state = MX_LSTATE_TRANS,
	},
	[MX_CMD_SETCFG] = {0, 0, 0, 0, 0},
	[MX_CMD_ERR_6] = {0, 0, 0, 0, 0},
	[MX_CMD_ERR_7] = {0, 0, 0, 0, 0},
	[MX_CMD_ERR_8] = {0, 0, 0, 0, 0},
	[MX_CMD_REQUEUE] = {0, 0, 0, 0, 0},
	[MX_CMD_DETACH] = {
		.irq_reject = MX_IRQ_INODL,
		.irq_no_line = MX_IRQ_INKOD,
		.fail_pos = MX_LSTATE_TRANS | MX_LSTATE_DETACH,
		.fail_neg = MX_LSTATE_NONE, // what if line is not attached?
		.cmd_state = MX_LSTATE_DETACH,
	},
	[MX_CMD_ABORT] = {
		.irq_reject = MX_IRQ_INABT,
		.irq_no_line = MX_IRQ_INKAB,
		.fail_pos = MX_LSTATE_NONE,
		.fail_neg = MX_LSTATE_TRANS, // what if abort is running or line is not attached?
		.cmd_state = MX_LSTATE_ABORT,
	},
	[MX_CMD_ERR_C] = {0, 0, 0, 0, 0},
	[MX_CMD_ERR_D] = {0, 0, 0, 0, 0},
	[MX_CMD_ERR_E] = {0, 0, 0, 0, 0},
	[MX_CMD_ERR_F] = {0, 0, 0, 0, 0},
};

// -----------------------------------------------------------------------
uint8_t mx_irq_noline(int cmd)
{
	return mx_cmd_routing[cmd].irq_no_line;
}

// -----------------------------------------------------------------------
uint8_t mx_irq_reject(int cmd)
{
	return mx_cmd_routing[cmd].irq_reject;
}

// -----------------------------------------------------------------------
int mx_line_cmd_allowed(struct mx_line *line, int cmd)
{
	const struct mx_cmd_route *route = mx_cmd_routing + cmd;
	return (
		(line->status & route->fail_pos) ||
		(route->fail_neg && !(line->status & route->fail_neg))
	);
}

// -----------------------------------------------------------------------
uint32_t mx_cmd_state(int cmd)
{
	return mx_cmd_routing[cmd].cmd_state;
}

// -----------------------------------------------------------------------
static void mx_line_process_cmd(struct mx_line *line, union mx_event *ev)
{
	int irq;
	const struct mx_cmd *cmd = line->proto->cmd + ev->d.cmd;

	LOG(L_MX, "EV%04x: Line %i (%s) got cmd %s", ev->d.id, line->log_n, line->proto->name, mx_get_cmd_name(ev->d.cmd));

	uint16_t cmd_data_addr = ev->d.arg;
	uint16_t cmd_data[MAX_CMD_DATA_LEN];

	// check for emulation errors
	if (cmd->input_flen + cmd->output_flen > MAX_CMD_DATA_LEN) {
		LOG(L_MX, "ERROR: EV%04x: protocol data (%i words) won't fit in the data buffer (%i words)", cmd->input_flen + cmd->output_flen, MAX_CMD_DATA_LEN);
		irq = MX_IRQ_INPAO;
		goto fin;
	}

	// read command parameters if applicable
	if ((cmd->input_flen > 0) && cmd->decode) {
		if (mx_mem_mget(line->multix, 0, cmd_data_addr, cmd_data, cmd->input_flen)) {
			irq = MX_IRQ_INPAO;
			goto fin;
		}
		// decode command parameters
		if (cmd->decode(cmd_data, line->proto_data)) {
			irq = MX_IRQ_INTRA;
			goto fin;
		}
	}

	// run the command
	irq = cmd->run(line, cmd_data);

fin:
	// store command output if applicable
	if ((cmd->output_flen > 0) && (cmd->encode) && (irq != MX_IRQ_INPAO)) {
		cmd->encode(cmd_data + cmd->input_flen, line->proto_data);
		if (mx_mem_mput(line->multix, 0, cmd_data_addr + cmd->input_flen, cmd_data + cmd->input_flen, cmd->output_flen)) {
			irq = MX_IRQ_INPAO;
		}
	}

	// clear line status for this command and send the interrupt
	pthread_mutex_lock(&line->status_mutex);
	line->status &= ~mx_cmd_state(ev->d.cmd);
	mx_int_enqueue(line->multix, irq, line->log_n);
	pthread_mutex_unlock(&line->status_mutex);
}

// -----------------------------------------------------------------------
void * mx_line_thread(void *ptr)
{
	int quit = 0;
	struct mx_line *line = (struct mx_line *) ptr;

	LOG(L_MX, "Entering line %i protocol loop, device: %s", line->log_n, line->proto->name);

	while (!quit) {
		LOG(L_MX, "Line %i (%s) waiting for event", line->log_n, line->proto->name);
		union mx_event *ev = (union mx_event *) elst_wait_pop(line->protoq, 0);
		switch (ev->d.type) {
			case MX_EV_QUIT:
				quit = 1;
				break;
			case MX_EV_CMD:
				mx_line_process_cmd(line, ev);
				break;
			default:
				LOG(L_MX, "EV%04x: Line %i (%s) protocol thread got unknown event type %i. Ignored.", ev->d.id, line->log_n, line->proto->name, ev->d.type);
				break;
		}
		free(ev);
	}

	LOG(L_MX, "Left line %i loop, device: %s", line->log_n, line->proto->name);

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
void log_line_status(const char *txt, int log_n, uint32_t status, unsigned evid)
{
	LOG(L_MX, "EV%04x: %s: line %i status: 0x%08x: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		evid,
		txt,
		log_n,
		status,
		(status & MX_LSTATE_SEND_START) ? "send_start, " : "",
		(status & MX_LSTATE_SEND_RUN) ? "send_run, " : "",
		(status & MX_LSTATE_RECV_START) ? "recv_start, " : "",
		(status & MX_LSTATE_RECV_RUN) ? "recv_run, " : "",
		(status & MX_LSTATE_CAN_STOP) ? "can_stop, " : "",
		(status & MX_LSTATE_STOP_CHAR) ? "stop_char, " : "",
		(status & MX_LSTATE_PARITY_ERR) ? "parity_err, " : "",
		(status & MX_LSTATE_OPRQ) ? "oprq, " : "",
		(status & MX_LSTATE_ATTACHED) ? "attached, " : "",
		(status & MX_LSTATE_TRANS) ? "transmission, " : "",
		(status & MX_LSTATE_TASK_XOFF) ? "task_xoff, " : "",
		(status & MX_LSTATE_TRANS_XOFF) ? "trans_xoff, " : "",
		(status & MX_LSTATE_TRANS_LAST) ? "trans_last, " : "",
		(status & MX_LSTATE_ATTACH) ? "attaching, " : "",
		(status & MX_LSTATE_DETACH) ? "detaching, " : "",
		(status & MX_LSTATE_ABORT) ? "aborting, " : ""
	);
}

// -----------------------------------------------------------------------
void * mx_line_status_thread(void *ptr)
{
	int quit = 0;
	struct mx_line *line = (struct mx_line *) ptr;

	LOG(L_MX, "Entering line %i status loop", line->log_n);

	while (!quit) {
		LOG(L_MX, "Line %i waiting for status event", line->log_n);
		union mx_event *ev = (union mx_event *) elst_wait_pop(line->statusq, 0);
		if ((ev->d.type == MX_EV_CMD) && (ev->d.cmd == MX_CMD_STATUS)) {
			pthread_mutex_lock(&line->status_mutex);
			log_line_status("Reporting line status", line->log_n, line->status, ev->d.id);
			uint16_t status = line->status & 0xffff;
			if (mx_mem_mput(line->multix, 0, ev->d.arg, &status, 1)) {
				mx_int_enqueue(line->multix, MX_IRQ_INPAO, line->log_n);
			} else {
				mx_int_enqueue(line->multix, MX_IRQ_ISTRE, line->log_n);
			}
			line->status &= ~mx_cmd_state(ev->d.cmd);
			pthread_mutex_unlock(&line->status_mutex);
		} else if (ev->d.type == MX_EV_QUIT) {
			quit = 1;
		} else {
			LOG(L_MX, "EV%04x: Line %i (%s) status thread got unknown event type %i. Ignored.", ev->d.id, line->log_n, line->proto->name, ev->d.type);
		}
		free(ev);
	}

	LOG(L_MX, "Left line %i status loop", line->log_n);

	pthread_exit(NULL);
}

// vim: tabstop=4 shiftwidth=4 autoindent
