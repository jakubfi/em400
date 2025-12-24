//  Copyright (c) 2025 Jakub Filipowicz <jakubf@gmail.com>
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

#include "log.h"

#include "io/dev2/terminal_fake.h"

// -----------------------------------------------------------------------
static void terminal_fake_ioloop_teardown(terminal_fake_t * terminal_fake)
{

}

// -----------------------------------------------------------------------
void terminal_fake_shutdown(em400_dev_t *dev)
{
	if (!dev) return;
	terminal_fake_t *terminal_fake = (terminal_fake_t *) dev;

	LOG(L_TERM, "Fake terminal shutting down");

	// TODO: proper async free with libuv
	terminal_fake_ioloop_teardown(terminal_fake);
	free(terminal_fake);
}

// -----------------------------------------------------------------------
void terminal_fake_free(em400_dev_t *dev)
{
	if (!dev) return;
	LOG(L_TERM, "Fake terminal freeing resources");

}

// -----------------------------------------------------------------------
void terminal_fake_reset(em400_dev_t *dev)
{
	if (!dev) return;
	LOG(L_TERM, "Fake terminal reset");
}

// -----------------------------------------------------------------------
em400_dev_t * terminal_fake_create(unsigned port)
{
	LOG(L_FLOP, "Creating fake terminal");

	terminal_fake_t *terminal_fake = calloc(1, sizeof(terminal_fake_t));
	if (!terminal_fake) {
		goto fail;
	}

	terminal_fake->base.type = EM400_DEV_TERMINAL;
	terminal_fake->base.reset = terminal_fake_reset;
	terminal_fake->base.write = NULL;
	terminal_fake->base.shutdown = terminal_fake_shutdown;

	terminal_fake->port = port;

	return (em400_dev_t *) terminal_fake;
fail:
	return NULL;
}

