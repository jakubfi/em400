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
#include <string.h>

#include "log.h"

#include "io/dev/flop5.h"

// -----------------------------------------------------------------------
static void flop5_ioloop_teardown(flop5_t * flop5)
{

}

// -----------------------------------------------------------------------
void flop5_shutdown(em400_dev_t *dev)
{
	if (!dev) return;
	flop5_t *flop5 = (flop5_t *) dev;

	LOG(L_FLOP, "Fake flop5 shutting down");

	// TODO: proper async free with libuv
	flop5_ioloop_teardown(flop5);
	free(flop5->image);
	free(flop5);
}

// -----------------------------------------------------------------------
void flop5_reset(em400_dev_t *dev)
{
	if (!dev) return;

	LOG(L_FLOP, "Fake flop5 reset");

}

// -----------------------------------------------------------------------
em400_dev_t * flop5_create(const char *image)
{
	LOG(L_FLOP, "Creating fake flop5");

	flop5_t *flop5 = calloc(1, sizeof(flop5_t));
	if (!flop5) {
		goto fail;
	}

	flop5->base.type = EM400_DEV_FLOP5;
	flop5->base.reset = flop5_reset;
	flop5->base.write = NULL;
	flop5->base.shutdown = flop5_shutdown;

	if (image) {
		flop5->image = strdup(image);
	}

	return (em400_dev_t *) flop5;
fail:
	return NULL;
}

