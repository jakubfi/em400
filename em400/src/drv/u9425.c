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

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "drv/u9425.h"
#include "cfg.h"
#include "errors.h"
#include "io.h"

// -----------------------------------------------------------------------
int drv_u9425_init(void *self, struct cfg_arg_t *arg)
{
	struct unit_t *unit = self;
	struct u9425_cfg_t *cfg = unit->cfg = malloc(sizeof(struct u9425_cfg_t));
	if (!unit->cfg) return E_IO_UNIT_INIT;

	// arg1 : fixed disk platter
	if (!arg || !arg->text) return E_IO_UNIT_INIT;
	cfg->img_fixed = strdup(arg->text);

	// arg2 : removable disk platter
	if (!arg->next || !arg->next->text) return E_IO_UNIT_INIT;
	cfg->img_removable = strdup(arg->next->text);

	return E_OK;
}

// -----------------------------------------------------------------------
void drv_u9425_shutdown(void *self)
{
}

// -----------------------------------------------------------------------
void drv_u9425_reset(void *self)
{
}

// -----------------------------------------------------------------------
int drv_u9425_cmd(void *self, int u_num, int dir, int cmd, uint16_t *r)
{
	return IO_NO;
}

// vim: tabstop=4
