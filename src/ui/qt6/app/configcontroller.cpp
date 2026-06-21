//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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

#include "configcontroller.h"
#include "emumodel.h"
#include "appcfg.h"

// -----------------------------------------------------------------------
void ConfigController::set_disk_image(struct appcfg_machine *m, unsigned chan, unsigned dev, unsigned slot, const char *path)
{
	if (!appcfg_set_image(m, chan, dev, slot, path)) return;
	if (appcfg_active_machine(cfg) == &m->cfg) {
		emu->reload_media(chan, dev, slot, path);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
