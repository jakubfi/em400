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

#include "io/dev/dev.h"
#include "cfg.h"

struct dev_puncher {
};

// -----------------------------------------------------------------------
void * dev_puncher_create(dictionary *cfg, int ch_num, int dev_num)
{
	struct dev_puncher *puncher = (struct dev_puncher *) malloc(sizeof(struct dev_puncher));
	if (!puncher) {
		goto cleanup;
	}

	return puncher;

cleanup:
	free(puncher);
	return NULL;
}

// -----------------------------------------------------------------------
void dev_puncher_destroy(void *dev)
{
	if (!dev) return;
	struct dev_puncher *puncher = (struct dev_puncher *) dev;
	free(puncher);
}

// -----------------------------------------------------------------------
void dev_puncher_reset(void *dev)
{

}

// -----------------------------------------------------------------------
int dev_puncher_read(struct dev_puncher *puncher)
{

	return 0;
}

// -----------------------------------------------------------------------
int dev_puncher_write(struct dev_puncher *puncher)
{
	return 0;
}

struct dev_drv dev_puncher = {
	.name = "puncher",
	.create = dev_puncher_create,
	.destroy = dev_puncher_destroy,
	.reset = dev_puncher_reset
};


// vim: tabstop=4 shiftwidth=4 autoindent
