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
#include <strings.h>

#include "log.h"
#include "io/dev/dev.h"
#include "io/dev2/dev2.h"

extern struct dev_drv dev_winch;
extern struct dev_drv dev_flop5;
extern struct dev_drv dev_terminal;


// -----------------------------------------------------------------------
int dev_make(em400_dev_t *dev, int ch_num, int dev_num, const struct dev_drv **dev_drv, void **dev_obj)
{
	switch (dev->type) {
		case EM400_DEV_WINCHESTER:
			*dev_obj = dev_winch.create(dev, ch_num, dev_num);
			*dev_drv = &dev_winch;
			break;
		case EM400_DEV_FLOP5:
			*dev_obj = dev_flop5.create(dev, ch_num, dev_num);
			*dev_drv = &dev_flop5;
			break;
		case EM400_DEV_TERMINAL:
			*dev_obj = dev_terminal.create(dev, ch_num, dev_num);
			*dev_drv = &dev_terminal;
			break;
		default:
			LOG(L_MX, "Unknown device type: %i", dev->type);
			break;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void dev_chs_next(struct dev_chs *chs, unsigned heads, unsigned spt)
{
	(chs->s)++;
	if (chs->s >= spt) {
		chs->s = 0;
		(chs->h)++;
		if (chs->h >= heads) {
			chs->h = 0;
			(chs->c)++;
		}
	}
}

// -----------------------------------------------------------------------
void dev_lba2chs(unsigned lba, struct dev_chs *chs, unsigned heads, unsigned spt)
{
	// translate logical sector address into CHS address
	chs->c = (lba / (heads * spt));
	chs->h = (lba / spt) % heads;
	chs->s = lba % spt;
}

// vim: tabstop=4 shiftwidth=4 autoindent
