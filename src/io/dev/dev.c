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

#include "cfg.h"
#include "log.h"
#include "errors.h"
#include "io/dev/dev.h"

extern struct dev_drv dev_winch;
extern struct dev_drv dev_flop5;
extern struct dev_drv dev_punchrd;
extern struct dev_drv dev_puncher;
extern struct dev_drv dev_terminal;
extern struct dev_drv dev_printer;

const struct dev_drv *dev_drivers[] = {
	&dev_winch,
	&dev_flop5,
	&dev_punchrd,
	&dev_puncher,
	&dev_terminal,
	&dev_printer,
	NULL
};

// -----------------------------------------------------------------------
int dev_make(struct cfg_unit *dev, const struct dev_drv **dev_drv, void **dev_obj)
{
	const struct dev_drv **driver = dev_drivers;

	while (*driver) {
		if (!strcasecmp(dev->name, (*driver)->name)) {
			*dev_drv = *driver;
			*dev_obj = (*driver)->create(dev->args);
			if (!*dev_obj) {
				LOG(L_EM4H, 1, "Error initializing device: %s", dev->name);
				return E_IO_DEV_INIT;
			}
			LOG(L_EM4H, 1, "Created device: %s", dev->name);
			return E_OK;
		}
		driver++;
	}

	LOG(L_EM4H, 1, "Unknown device type: %s", dev->name);

	return E_IO_DEV_UNKNOWN;
}

// vim: tabstop=4 shiftwidth=4 autoindent
