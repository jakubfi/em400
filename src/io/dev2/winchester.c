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

#include "io/dev2/winchester.h"

// -----------------------------------------------------------------------
static void winchester_ioloop_teardown(winchester_t * winchester)
{

}

// -----------------------------------------------------------------------
void winchester_destroy(em400_dev_t *dev)
{
	if (!dev) return;

	LOG(L_TERM, "Fake winchester shutting down");

	winchester_ioloop_teardown((winchester_t *) dev);
}

// -----------------------------------------------------------------------
void winchester_free(em400_dev_t *dev)
{
	if (!dev) return;
	winchester_t *winchester = (winchester_t *) dev;
	LOG(L_TERM, "Fake winchester freeing resources");

	free(winchester->image);
	free(winchester);
}

// -----------------------------------------------------------------------
em400_dev_t * winchester_create(const char *image)
{
	LOG(L_FLOP, "Creating fake winchester");

	winchester_t *winchester = calloc(1, sizeof(winchester_t));
	if (!winchester) {
		goto fail;
	}

	winchester->base.type = EM400_DEV_WINCHESTER;
	if (image) {
		winchester->image = strdup(image);
	}

	return (em400_dev_t *) winchester;
fail:
	return NULL;
}

