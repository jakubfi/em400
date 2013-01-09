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


#include "drv/drivers.h"
#include "drv/cnone.h"
#include "drv/cmem.h"
#include "drv/cchar.h"
#include "drv/unone.h"
#include "drv/u9425.h"

struct drv_chan_t drv_chan[] = {
	{ CHAN_NONE, "none", "None", drv_cnone_init, drv_cnone_shutdown, drv_cnone_reset, drv_cnone_cmd },
	{ CHAN_CHAR, "char", "Character", drv_cchar_init, drv_cchar_shutdown, drv_cchar_reset, drv_cchar_cmd },
	{ CHAN_MEM, "mem", "Memory", drv_cmem_init, drv_cmem_shutdown, drv_cmem_reset, drv_cmem_cmd },
	{ CHAN_PI, "pi", "PI", NULL, NULL, NULL, NULL },
	{ CHAN_MULTIX, "multix", "MULTIX", NULL, NULL, NULL, NULL },
	{ CHAN_PLIX, "plix", "PLIX", NULL, NULL, NULL, NULL }
};

struct drv_unit_t drv_unit[] = {
	{ UNIT_NONE, CHAN_NONE, "none", "None", drv_unone_init, drv_unone_shutdown, drv_unone_reset, drv_unone_cmd },
	{ UNIT_9425, CHAN_MEM, "9425", "MERA-9425", drv_u9425_init, drv_u9425_shutdown, drv_u9425_reset, drv_u9425_cmd },
	{ UNIT_WINCHESTER, CHAN_PLIX, "winchester", "Winchester", NULL, NULL, NULL, NULL },
	{ UNIT_TERM_TCP, CHAN_CHAR, "term_tcp", "TCP terminal", NULL, NULL, NULL, NULL },
	{ UNIT_TERM_SERIAL, CHAN_CHAR, "term_serial", "Serial terminal", NULL, NULL, NULL, NULL }
};

// vim: tabstop=4
