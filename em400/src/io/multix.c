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

#define CI ((struct multix_internal_t*) chan->i)

// -----------------------------------------------------------------------
int multix_init(struct chan_t *chan, struct cfg_unit_t *units)
{
	chan->f_shutdown = multix_shutdown;
	chan->f_reset = multix_reset;
	chan->f_cmd = multix_cmd;
	chan->i = calloc(1, sizeof(struct multix_internal_t));

	multix_reset(chan);
	return E_OK;
}

// -----------------------------------------------------------------------
void multix_shutdown(struct chan_t *chan)
{
}

// -----------------------------------------------------------------------
void multix_reset(struct chan_t *chan)
{
}

// -----------------------------------------------------------------------
int multix_cmd(struct chan_t *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	return IO_OK;
}


// vim: tabstop=4 shiftwidth=4 autoindent
