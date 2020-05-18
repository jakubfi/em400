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
#include "external/iniparser/iniparser.h"

struct dev_punchrd {
};

// -----------------------------------------------------------------------
void * dev_punchrd_create(dictionary *cfg, const char *section)
{
	struct dev_punchrd *punchrd = (struct dev_punchrd *) malloc(sizeof(struct dev_punchrd));
	if (!punchrd) {
		goto cleanup;
	}

	return punchrd;

cleanup:
	free(punchrd);
	return NULL;
}

// -----------------------------------------------------------------------
void dev_punchrd_destroy(void *dev)
{
	if (!dev) return;
	struct dev_punchrd *punchrd = (struct dev_punchrd *) dev;
	free(punchrd);
}

// -----------------------------------------------------------------------
void dev_punchrd_reset(void *dev)
{

}

// -----------------------------------------------------------------------
int dev_punchrd_read(struct dev_punchrd *punchrd)
{

	return 0;
}

// -----------------------------------------------------------------------
int dev_punchrd_write(struct dev_punchrd *punchrd)
{
	return 0;
}

struct dev_drv dev_punchrd = {
	.name = "punchreader",
	.create = dev_punchrd_create,
	.destroy = dev_punchrd_destroy,
	.reset = dev_punchrd_reset
};


// vim: tabstop=4 shiftwidth=4 autoindent
