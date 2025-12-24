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

#include "io/dev2/sp45de.h"

// -----------------------------------------------------------------------
static void sp45de_ioloop_teardown(sp45de_t * sp45de)
{

}

// -----------------------------------------------------------------------
void sp45de_shutdown(em400_dev_t *dev)
{
	if (!dev) return;
	sp45de_t *sp45de = (sp45de_t *) dev;

	LOG(L_TERM, "Fake SP45DE shutting down");

	// TODO: proper async free with libuv
	sp45de_ioloop_teardown((sp45de_t *) dev);
	for (int i=0 ; i<4 ; i++) {
		free(sp45de->images[i]);
	}
	free(sp45de);
}

// -----------------------------------------------------------------------
void sp45de_reset(em400_dev_t *dev)
{
	if (!dev) return;

	LOG(L_TERM, "Fake SP45DE reset");
}

// -----------------------------------------------------------------------
em400_dev_t * sp45de_create(const char *images[4])
{
	LOG(L_FLOP, "Creating fake SP45DE");

	sp45de_t *sp45de = calloc(1, sizeof(sp45de_t));
	if (!sp45de) {
		goto fail;
	}

	sp45de->base.type = EM400_DEV_SP45DE;
	sp45de->base.reset = sp45de_reset;
	sp45de->base.write = NULL;
	sp45de->base.shutdown = sp45de_shutdown;

	for (int i=0 ; i<4 ; i++) {
		if (!images[i]) continue;
		sp45de->images[i] = strdup(images[i]);
	}

	return (em400_dev_t *) sp45de;

fail:
	return NULL;
}

