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
#include "io/dev/e4image.h"

struct dev_flop5 {
	struct e4i_t *image;
};

// -----------------------------------------------------------------------
void * dev_flop5_create(struct cfg_arg *args)
{
	struct dev_flop5 *flop5 = malloc(sizeof(struct dev_flop5));
	if (!flop5) {
		goto cleanup;
	}

	flop5->image = e4i_open(args->text);
	if (!flop5->image) {
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
int dev_flop5_read(void *dev, uint8_t *buf, unsigned offset)
{
	struct dev_flop5 *flop5 = dev;

	if (e4i_bread(flop5->image, buf, offset) == E4I_E_OK) {
		return 0;
	} else {
		return 1;
	}
}

// -----------------------------------------------------------------------
int dev_flop5_write(void *dev, uint8_t *buf, unsigned offset, unsigned bytes)
{
	struct dev_flop5 *flop5 = dev;

	if (e4i_bwrite(flop5->image, buf, offset, bytes) == E4I_E_OK) {
		return 0;
	} else {
		return 1;
	}
}

struct dev_drv dev_flop5 = {
	.name = "floppy",
	.create = dev_flop5_create,
	.destroy = dev_flop5_destroy,
	.reset = dev_flop5_reset,
	.read = dev_flop5_read,
	.write = dev_flop5_write,
};


// vim: tabstop=4 shiftwidth=4 autoindent
