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

struct dev_winch {
	struct e4i_t *image;
};

// -----------------------------------------------------------------------
void * dev_winch_create(struct cfg_arg *args)
{
	struct dev_winch *winch = malloc(sizeof(struct dev_winch));
	if (!winch) {
		goto cleanup;
	}

	winch->image = e4i_open(args->text);
	if (!winch->image) {
		goto cleanup;
	}

	return winch;

cleanup:
	free(winch);
	return NULL;
}

// -----------------------------------------------------------------------
void dev_winch_destroy(void *dev)
{
	struct dev_winch *winch = dev;
	e4i_close(winch->image);
	free(dev);
}

// -----------------------------------------------------------------------
void dev_winch_reset(void *dev)
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
int dev_winch_sector_rd(void *dev, uint8_t *buf, struct dev_chs *chs)
{
	int res;
	struct dev_winch *winch = dev;

	res = e4i_sread(winch->image, buf, chs->c, chs->h, chs->s);

	return _e4i_res(res);
}

// -----------------------------------------------------------------------
int dev_winch_sector_wr(void *dev, uint8_t *buf, struct dev_chs *chs)
{
	int res;
	struct dev_winch *winch = dev;

	res = e4i_swrite(winch->image, buf, chs->c, chs->h, chs->s, 512);

	return _e4i_res(res);
}

// -----------------------------------------------------------------------
struct dev_drv dev_winch = {
	.name = "winchester",
	.create = dev_winch_create,
	.destroy = dev_winch_destroy,
	.reset = dev_winch_reset,
	.sector_rd = dev_winch_sector_rd,
	.sector_wr = dev_winch_sector_wr,
};


// vim: tabstop=4 shiftwidth=4 autoindent
