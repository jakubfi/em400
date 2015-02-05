//  Copyright (c) 2012-2014 Jakub Filipowicz <jakubf@gmail.com>
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

#include "io/chan.h"
#include "io/cmem.h"
#include "io/cchar.h"
#include "io/mx.h"

#include "errors.h"
#include "log.h"

struct chan_drv *chan_drivers[] = {
	&cchar_chan_driver,
	&mx_chan_driver,
	NULL
};

// -----------------------------------------------------------------------
struct chan * chan_make(int num, char *name, struct cfg_unit *units)
{
	struct chan_drv **cdriver = chan_drivers;

	while (*cdriver) {
		if (!strcasecmp(name, (*cdriver)->name)) {
			struct chan *chan = malloc(sizeof(struct chan));
			if (!chan) {
				return NULL;
			}
			chan->drv = *cdriver;
			chan->obj = chan->drv->create(num, units);
			if (!chan->obj) {
				free(chan);
				return NULL;
			}
			return chan;
		}
		cdriver++;
	}

	gerr = E_IO_CHAN_UNKNOWN;
	return NULL;
}

// -----------------------------------------------------------------------
void chan_destroy(struct chan *chan)
{
	chan->drv->shutdown(chan->obj);
	free(chan);
}

// vim: tabstop=4 shiftwidth=4 autoindent
