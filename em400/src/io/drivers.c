//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include <strings.h>

#include "io/drivers.h"
#include "io/cmem.h"
#include "io/cchar.h"
#include "io/u9425.h"

struct drv_t drv_drivers[] = {
// channel driver definitions
	{ DRV_CHAN, CHAN_CHAR,		"char",			0,	8,	drv_cchar_init,	drv_cchar_shutdown,	drv_cchar_reset,	drv_cchar_cmd },
	{ DRV_CHAN, CHAN_MEM,		"mem",			0,	8,	drv_cmem_init,	drv_cmem_shutdown,	drv_cmem_reset,		drv_cmem_cmd },
	{ DRV_CHAN, CHAN_MULTIX,	"multix",		0,	256,NULL,			NULL,				NULL,				NULL },
	{ DRV_CHAN, CHAN_PLIX,		"plix",			0,	256,NULL,			NULL,				NULL,				NULL },
// unit driver definitions
	{ DRV_UNIT, CHAN_MEM,		"mera9425",		2,	0,	drv_u9425_init,	drv_u9425_shutdown,	drv_u9425_reset,	drv_u9425_cmd },
	{ DRV_UNIT, CHAN_PLIX,		"winchester",	1,	0,	NULL,			NULL,				NULL,				NULL },
	{ DRV_UNIT, CHAN_CHAR,		"term_tcp",		1,	0,	NULL,			NULL,				NULL,				NULL },
	{ DRV_UNIT, CHAN_MULTIX,	"term_tcp",		1,	0,	NULL,			NULL,				NULL,				NULL },
	{ DRV_UNIT, CHAN_CHAR,		"term_serial",	5,	0,	NULL,			NULL,				NULL,				NULL },
	{ DRV_UNIT, CHAN_CHAR,		"term_cons",	0,	0,	NULL,			NULL,				NULL,				NULL },
	{ DRV_UNIT, CHAN_MULTIX,	"term_cons",	0,	0,	NULL,			NULL,				NULL,				NULL },
	{ DRV_NONE, CHAN_IGNORE,	"",				0,	0,	NULL,			NULL,				NULL,				NULL }
};

// -----------------------------------------------------------------------
struct drv_t * drv_get(int role, int chan_type, char *name)
{
	struct drv_t *driver = drv_drivers;

	while (driver->name) {
		if ((driver->role == role) && !strcasecmp(name, driver->name) && ((chan_type == CHAN_IGNORE) || (driver->chan_type == chan_type))) {
			return driver;
		}
		driver++;
	}

	return NULL;
}

// vim: tabstop=4 shiftwidth=4 autoindent
