//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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


#include "drv/drivers.h"
#include "drv/cnone.h"
#include "drv/cmem.h"

#include "drv/unone.h"

struct drv_chan_t drv_chan[] = {
	{ CHAN_NONE, drv_cnone_init, drv_cnone_shutdown, drv_cnone_reset, drv_cnone_cmd },
	{ CHAN_CHAR, NULL, NULL, NULL, NULL },
	{ CHAN_MEM, drv_cmem_init, drv_cmem_shutdown, drv_cmem_reset, drv_cmem_cmd },
	{ CHAN_PI, NULL, NULL, NULL, NULL },
	{ CHAN_MULTIX, NULL, NULL, NULL, NULL },
	{ CHAN_PLIX, NULL, NULL, NULL, NULL }
};

struct drv_unit_t drv_unit[] = {
	{ UNIT_NONE, CHAN_NONE, drv_unone_init, drv_unone_shutdown, drv_unone_reset, drv_unone_cmd },
	{ UNIT_9425, CHAN_MEM, NULL, NULL, NULL, NULL },
	{ UNIT_WINCHESTER, CHAN_PLIX, NULL, NULL, NULL, NULL },
	{ UNIT_TERM_TCP, CHAN_CHAR, NULL, NULL, NULL, NULL },
	{ UNIT_TERM_SERIAL, CHAN_CHAR, NULL, NULL, NULL, NULL }
};

// vim: tabstop=4
