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

struct dev_printer {
};

// -----------------------------------------------------------------------
void * dev_printer_create(struct cfg_arg *args)
{
	struct dev_printer *printer = (struct dev_printer *) malloc(sizeof(struct dev_printer));
	if (!printer) {
		goto cleanup;
	}

	return printer;

cleanup:
	free(printer);
	return NULL;
}

// -----------------------------------------------------------------------
void dev_printer_destroy(void *dev)
{
	if (!dev) return;
	struct dev_printer *printer = (struct dev_printer *) dev;
	free(printer);
}

// -----------------------------------------------------------------------
void dev_printer_reset(void *dev)
{

}

// -----------------------------------------------------------------------
int dev_printer_read(struct dev_printer *printer)
{

	return 0;
}

// -----------------------------------------------------------------------
int dev_printer_write(struct dev_printer *printer)
{
	return 0;
}

struct dev_drv dev_printer = {
	.name = "printer",
	.create = dev_printer_create,
	.destroy = dev_printer_destroy,
	.reset = dev_printer_reset
};


// vim: tabstop=4 shiftwidth=4 autoindent
