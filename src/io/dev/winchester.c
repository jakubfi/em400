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

#include "io/dev/winchester.h"
#include "io/dev/e4image.h"

// -----------------------------------------------------------------------
static int e4i_res(int res)
{
	switch (res) {
		case E4I_E_OK:
			return DEV_STATUS_OK;
		case E4I_E_UNFORMATTED:
		case E4I_E_NO_SECTOR:
			return DEV_STATUS_SEEKERR;
		case E4I_E_WRPROTECT:
			return DEV_STATUS_WRPROTECT;
		case E4I_E_WRITE:
			return DEV_STATUS_WRERR;
		case E4I_E_READ:
			return DEV_STATUS_RDERR;
		default:
			return DEV_STATUS_ERR;
	}
}

// -----------------------------------------------------------------------
int winchester_sector_rd(winchester_t *winchester, uint8_t *buf, unsigned c, unsigned h, unsigned s)
{
	return e4i_res(e4i_sread(winchester->e4image, buf, c, h, s));
}

// -----------------------------------------------------------------------
int winchester_sector_wr(winchester_t *winchester, uint8_t *buf, unsigned c, unsigned h, unsigned s)
{
	return e4i_res(e4i_swrite(winchester->e4image, buf, c, h, s, 512));
}

// -----------------------------------------------------------------------
static void winchester_ioloop_teardown(winchester_t * winchester)
{

}

// -----------------------------------------------------------------------
void winchester_shutdown(em400_dev_t *dev)
{
	if (!dev) return;
	winchester_t *winchester = (winchester_t *) dev;

	LOG(L_WNCH, "Winchester shutting down");

	winchester_ioloop_teardown(winchester);
	e4i_close(winchester->e4image);
	free(winchester->image_name);
	free(winchester);
}

// -----------------------------------------------------------------------
void winchester_reset(em400_dev_t *dev)
{
	if (!dev) return;
	LOG(L_WNCH, "Winchester reset");
}

// -----------------------------------------------------------------------
em400_dev_t * winchester_create(const char *image_name)
{
	LOG(L_WNCH, "Creating winchester");

	winchester_t *winchester = calloc(1, sizeof(winchester_t));
	if (!winchester) {
		goto fail;
	}

	winchester->base.type = EM400_DEV_WINCHESTER;
	winchester->base.slot_count = 1;
	winchester->base.reset = winchester_reset;
	winchester->base.write = NULL;
	winchester->base.shutdown = winchester_shutdown;

	if (image_name) {
		winchester->image_name = strdup(image_name);
	}

	LOG(L_WNCH, "Opening image: %s", winchester->image_name);
	winchester->e4image = e4i_open(winchester->image_name);
	if (!winchester->e4image) {
		LOGERR("Failed to open Winchester image: \"%s\": %s.", winchester->image_name, e4i_get_err(e4i_err));
		free(winchester->image_name);
		return NULL;
	}

	return (em400_dev_t *) winchester;
fail:
	return NULL;
}

