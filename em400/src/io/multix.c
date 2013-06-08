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

#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>

#include "io/io.h"
#include "io/multix.h"

#include "cfg.h"
#include "errors.h"

#include "debugger/log.h"

#define CI ((struct mx_internal_t*) chan->i)

// -----------------------------------------------------------------------
int mx_init(struct chan_t *chan, struct cfg_unit_t *units)
{
	chan->f_shutdown = mx_shutdown;
	chan->f_reset = mx_reset;
	chan->f_cmd = mx_cmd;
	chan->i = calloc(1, sizeof(struct mx_internal_t));
	if (!chan->i) {
		return E_ALLOC;
	}

	mx_reset(chan);
	return E_OK;
}

// -----------------------------------------------------------------------
void mx_shutdown(struct chan_t *chan)
{
}

// -----------------------------------------------------------------------
void mx_reset(struct chan_t *chan)
{
}

// -----------------------------------------------------------------------
int mx_cmd_int_requeue(struct chan_t *chan)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_test(struct chan_t *chan, unsigned param, uint16_t *r_arg)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_setcfg(struct chan_t *chan, uint16_t *r_arg)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_attach(struct chan_t *chan, unsigned lline, uint16_t *r_arg)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_detach(struct chan_t *chan, unsigned lline)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_status(struct chan_t *chan, unsigned lline, uint16_t *r_arg)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_transmit(struct chan_t *chan, unsigned lline, uint16_t *r_arg)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd_break(struct chan_t *chan, unsigned lline)
{
	return IO_OK;
}

// -----------------------------------------------------------------------
int mx_cmd(struct chan_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	unsigned cmd = (n_arg & 0b1110000000000000) >> 13;
	unsigned lline = (n_arg & 0b0001111111100000) >> 5;

	if (cmd == 0) {
		cmd = (n_arg & 0b0001100000000000) >> 11;
		switch (cmd) {
			case MX_CMD_RESET:
				break;
			case MX_CMD_EXISTS:
				break;
			case MX_CMD_INTSPEC:
				break;
			default:
				break;
		}
	} else {
		if (dir == IO_IN) {
			switch (cmd) {
				case MX_CMD_INTRQ:
					return mx_cmd_int_requeue(chan);
				case MX_CMD_DETACH:
					return mx_cmd_detach(chan, lline);
				case MX_CMD_BREAK:
					return mx_cmd_break(chan, lline);
				default:
					break;
			}
		} else {
			switch (cmd) {
				case MX_CMD_TEST:
					return mx_cmd_test(chan, lline, r_arg);
				case MX_CMD_SETCFG:
					return mx_cmd_setcfg(chan, r_arg);
				case MX_CMD_ATTACH:
					return mx_cmd_attach(chan, lline, r_arg);
				case MX_CMD_STATUS:
					return mx_cmd_status(chan, lline, r_arg);
				case MX_CMD_TRANSMIT:
					return mx_cmd_transmit(chan, lline, r_arg);
				default:
					break;
			}
		}
	}

	return IO_OK;
}


// vim: tabstop=4 shiftwidth=4 autoindent
