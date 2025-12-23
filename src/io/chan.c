//  Copyright (c) 2012-2025 Jakub Filipowicz <jakubf@gmail.com>
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

#include <assert.h>

#include "log.h"
#include "io/chan.h"
#include "io/cchar/cchar.h"
#include "io/iotester.h"
#include "io/mx/mx.h"

typedef chan_t * (*chan_create_f)(int ch_num);

static const chan_create_f chan_constructor[] = {
	[CHAN_CHAR] = cchar_create,
	[CHAN_IOTESTER] = it_create,
	[CHAN_MULTIX] = mx_create,
};

// -----------------------------------------------------------------------
chan_t * chan_create(unsigned num, unsigned type)
{
	if (type >= CHAN_TYPE_COUNT) {
		LOGERR("Unknown channel type: %d", type);
		return NULL;
	}

	chan_t *chan = chan_constructor[type](num);
	if (chan) {
		assert(chan->cmd);
		assert(chan->reset);
		assert(chan->destroy);
		assert(chan->connect_dev);
	}
	// TODO: should chan->num be filled in here?

	return chan;
}

// -----------------------------------------------------------------------
void chan_destroy(chan_t *chan)
{
	if (!chan) return;
	chan->destroy(chan);
}

// vim: tabstop=4 shiftwidth=4 autoindent
