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
#include "io/mx/event.h"
#include "utils/elst.h"
#include "log.h"

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
	"[none]",
	"[invalid]",
	"input",
	"[invalid]",
	"output",
	"[invalid]",
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
	"[unknown]"
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
void * mx_line_thread(void *ptr)
{
	int quit = 0;
	struct mx_line *line = ptr;

	LOG(L_MX, 3, "Entering line %i loop, device: %s", line->log_n, line->proto->name);

	while (!quit) {
		LOG(L_MX, 3, "Line %i (%s) waiting for event", line->log_n, line->proto->name);
		union mx_event *ev = elst_wait_pop(line->devq, 0);
		switch (ev->d.type) {
			case MX_EV_QUIT:
				quit = 1;
				break;
			case MX_EV_CMD:
				LOG(L_MX, 3, "Line %i (%s) got cmd %s", line->log_n, line->proto->name, mx_get_cmd_name(ev->d.cmd));
				line->proto->cmd[ev->d.cmd].fun(line);
				break;
			default:
				LOG(L_MX, 3, "Line %i (%s) got unknown event type %i. Ignored.", line->log_n, line->proto->name, ev->d.type);
				break;
		}
		free(ev);
	}

	LOG(L_MX, 3, "Left line %i loop, device: %s", line->log_n, line->proto->name);

	pthread_exit(NULL);
}

// vim: tabstop=4 shiftwidth=4 autoindent
