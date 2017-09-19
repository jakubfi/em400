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

#include "log.h"
#include "cfg.h"
#include "io/dev/dev.h"
#include "io/dev/e4image.h"

struct dev_flop5 {
	struct e4i_t *image;
};

// -----------------------------------------------------------------------
void * dev_flop5_create(struct cfg_arg *args)
{
	struct dev_flop5 *flop5 = malloc(sizeof(struct dev_flop5));
	if (!flop5) {
		log_err("Memory allocation error while creating 5-inch floppy drive.");
		goto cleanup;
	}

	flop5->image = e4i_open(args->text);
	if (!flop5->image) {
		log_err("Failed to open 5-inch floppy image: \"%s\": %s.", args->text, e4i_get_err(e4i_err));
		goto cleanup;
	}

	return flop5;

cleanup:
	free(flop5);
	return NULL;
}

// -----------------------------------------------------------------------
void dev_flop5_destroy(void *dev)
{
	struct dev_flop5 *flop5 = dev;
	e4i_close(flop5->image);
	free(dev);
}

// -----------------------------------------------------------------------
void dev_flop5_reset(void *dev)
{

}
// -----------------------------------------------------------------------
static int _e4i_res(int res)
{
	switch (res) {
		case E4I_E_OK:
			return DEV_CMD_OK;
		case E4I_E_UNFORMATTED:
		case E4I_E_NO_SECTOR:
			return DEV_CMD_SEEKERR;
		case E4I_E_WRPROTECT:
			return DEV_CMD_WRPROTECT;
		case E4I_E_WRITE:
			return DEV_CMD_WRERR;
		case E4I_E_READ:
			return DEV_CMD_RDERR;
		default:
			return DEV_CMD_ERR;
	}
}

// -----------------------------------------------------------------------
int dev_flop5_sector_rd(void *dev, uint8_t *buf, struct dev_chs *chs)
{
	int res;
	struct dev_flop5 *flop5 = dev;

	res = e4i_swrite(flop5->image, buf, chs->c, chs->h, chs->s, 512);

	return _e4i_res(res);
}

// -----------------------------------------------------------------------
int dev_flop5_sector_wr(void *dev, uint8_t *buf, struct dev_chs *chs)
{
	int res;
	struct dev_flop5 *flop5 = dev;

	res = e4i_swrite(flop5->image, buf, chs->c, chs->h, chs->s, 512);

	return _e4i_res(res);
}

struct dev_drv dev_flop5 = {
	.name = "floppy",
	.create = dev_flop5_create,
	.destroy = dev_flop5_destroy,
	.reset = dev_flop5_reset,
	.sector_rd = dev_flop5_sector_rd,
	.sector_wr = dev_flop5_sector_wr,
};


// vim: tabstop=4 shiftwidth=4 autoindent
