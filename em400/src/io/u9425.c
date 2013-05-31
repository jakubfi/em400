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

#include "io/io.h"
#include "io/lib.h"
#include "io/u9425.h"

#include "cfg.h"
#include "errors.h"

// -----------------------------------------------------------------------
int drv_u9425_init(void *self, struct cfg_arg_t *arg)
{
	struct unit_t *unit = self;
	struct u9425_cfg_t *cfg = unit->cfg = malloc(sizeof(struct u9425_cfg_t));
	if (!unit->cfg) return E_IO_UNIT_INIT;

	int argc = args_to_cfg(arg, "ss", &cfg->img_fixed, &cfg->img_removable);
	if (argc != 2) {
		return E_IO_UNIT_INIT_ARGS;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void drv_u9425_shutdown(void *self)
{
	struct u9425_cfg_t *cfg = ((struct unit_t *)self)->cfg;
	free(cfg->img_fixed);
	free(cfg->img_removable);
	free(cfg);
}

// -----------------------------------------------------------------------
void drv_u9425_reset(void *self)
{
}

// -----------------------------------------------------------------------
int drv_u9425_cmd(void *self, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	return IO_NO;
}

// vim: tabstop=4
