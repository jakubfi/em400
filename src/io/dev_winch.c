//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
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

#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>

#include "cfg.h"
#include "errors.h"
#include "log.h"
#include "io/dev/e4image.h"
#include "io/dev_winch.h"

#define DEVICE ((struct dev_winch *) device)

// -----------------------------------------------------------------------
int dev_winch_setup(struct dev *device, struct cfg_arg *args)
{
	int res;
	int ret;
	char *image_name = NULL;

	res = cfg_args_decode(args, "s", &image_name);
	if (res != E_OK) {
		ret = res;
		goto cleanup;
	}

	DEVICE->winchester = e4i_open(image_name);
	if (!DEVICE->winchester) {
		//printf("Error opening image %s: %s\n", image_name, e4i_get_err(e4i_err));
		ret = E_IMAGE;
		goto cleanup;
	}

	if (DEVICE->winchester->img_type != E4I_T_HDD) {
		//printf("Error opening image %s: wrong image type, expecting hdd\n", image_name);
		ret = E_IMAGE;
		goto cleanup;
	}

	if ((DEVICE->winchester->cylinders != 615)
	|| (DEVICE->winchester->heads != 4)
	|| (DEVICE->winchester->spt != 16)
	|| (DEVICE->winchester->block_size != 512)) {
		//printf("Error opening image %s: wrong geometry\n", image_name);
		ret = E_IMAGE;
		goto cleanup;
	}

	LOG(L_WNCH, 1, "      Winchester: cyl=%i, head=%i, sectors=%i, spt=%i, image=%s",
		DEVICE->winchester->cylinders,
		DEVICE->winchester->heads,
		DEVICE->winchester->spt,
		DEVICE->winchester->block_size,
		image_name);

	free(image_name);
	return E_OK;

cleanup:
	dev_winch_close(device);
	free(image_name);
	return ret;
}

// -----------------------------------------------------------------------
void dev_winch_close(struct dev *device)
{
	e4i_close(DEVICE->winchester);
}

// -----------------------------------------------------------------------
void dev_winch_reset(struct dev *device)
{
}

// -----------------------------------------------------------------------
void dev_winch_attach(struct dev *device)
{
}

// -----------------------------------------------------------------------
void dev_winch_detach(struct dev *device)
{
}

// -----------------------------------------------------------------------
static int dev_winch_read(struct e4i_t *winch, char *dest, int addr, int bytes)
{
	return 0;
}

// -----------------------------------------------------------------------
static int dev_winch_write(struct e4i_t *winch, char *src, int addr, int bytes)
{
	return 0;
}

// -----------------------------------------------------------------------
int dev_winch_transmit(struct dev *device, int op, char *buf, int addr, int count, int timeout)
{
	int ret = -1;

	switch (op) {
		case DEV_OP_READ:
			ret = dev_winch_read(DEVICE->winchester, buf, addr, count);
			break;
		case DEV_OP_WRITE:
			ret = dev_winch_write(DEVICE->winchester, buf, addr, count);
			break;
		case DEV_OP_PARK:
			break;
		case DEV_OP_MAX:
			break;
		default:
			break;
	}

	return ret;
}

// vim: tabstop=4 shiftwidth=4 autoindent
