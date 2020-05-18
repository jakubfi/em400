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
#include "external/iniparser/iniparser.h"

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
int dev_make(dictionary *cfg, const char *section, const struct dev_drv **dev_drv, void **dev_obj)
{
	const struct dev_drv **driver = dev_drivers;

	char key[32];
	sprintf(key, "%s:type", section);
	const char *type = iniparser_getstring(cfg, key, NULL);

	while (*driver) {
		if (!strcasecmp(type, (*driver)->name)) {
			*dev_drv = *driver;
			*dev_obj = (*driver)->create(cfg, section);
			if (!*dev_obj) {
				return LOGERR("Failed to initialize device: %s.", type);
			}
			LOG(L_EM4H, "Created device: %s", type);
			return E_OK;
		}
		driver++;
	}

	return LOGERR("Unknown device type: %s.", type);
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
