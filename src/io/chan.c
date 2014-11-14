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

#include "io/cmem.h"
#include "io/cchar.h"
#include "io/multix.h"

#include "errors.h"
#include "log.h"

static struct chan chan_proto[] = {
	{ "char",		cchar_create,	cchar_shutdown,	cchar_reset,	cchar_cmd },
	{ "mem",		cmem_create,	cmem_shutdown,	cmem_reset,		cmem_cmd },
	{ "multix",		mx_create,		mx_shutdown,	mx_reset,		mx_cmd },
	{ NULL,			NULL,			NULL,			NULL,			NULL }
};

// -----------------------------------------------------------------------
struct chan * chan_make(int num, char *name, struct cfg_unit *units)
{
	struct chan *proto = chan_proto;
	struct chan *chan = NULL;

	while (proto && proto->name) {
		if (!strcasecmp(name, proto->name)) {
			chan = proto->create(units);
			break;
		}
		proto++;
	}

	if (chan) {
		chan->num = num;
		chan->name = proto->name;
		chan->create = proto->create;
		chan->shutdown = proto->shutdown;
		chan->reset = proto->reset;
		chan->cmd = proto->cmd;
	} else if (!proto->name) {
		gerr = E_IO_CHAN_UNKNOWN;
	}

	return chan;
}

// vim: tabstop=4 shiftwidth=4 autoindent
