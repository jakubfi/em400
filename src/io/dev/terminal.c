//  Copyright (c) 2015 Jakub Filipowicz <jakubf@gmail.com>
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

#include "cfg.h"
#include "io/dev/dev.h"

struct dev_terminal {
};

// -----------------------------------------------------------------------
void * dev_terminal_create(struct cfg_arg *args)
{
	struct dev_terminal *terminal = malloc(sizeof(struct dev_terminal));
	if (!terminal) {
		goto cleanup;
	}

	return terminal;

cleanup:
	free(terminal);
	return NULL;
}

// -----------------------------------------------------------------------
void dev_terminal_destroy(void *dev)
{
	struct dev_terminal *terminal = dev;
	free(terminal);
}

// -----------------------------------------------------------------------
void dev_terminal_reset(void *dev)
{

}

// -----------------------------------------------------------------------
int dev_terminal_read(struct dev_terminal *terminal)
{

	return 0;
}

// -----------------------------------------------------------------------
int dev_terminal_write(struct dev_terminal *terminal)
{
	return 0;
}

struct dev_drv dev_terminal = {
	.name = "terminal",
	.create = dev_terminal_create,
	.destroy = dev_terminal_destroy,
	.reset = dev_terminal_reset
};


// vim: tabstop=4 shiftwidth=4 autoindent
