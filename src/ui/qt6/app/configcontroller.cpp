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

#include <cstring>

#include "configcontroller.h"
#include "emumodel.h"
#include "appcfg.h"
#include "app_err.h"

// -----------------------------------------------------------------------
namespace {
bool differs(const char *a, const char *b)
{
	if (!a || !b) return a != b;
	return strcmp(a, b) != 0;
}

// -----------------------------------------------------------------------
// reload_media no-ops unless the machine is powered and the slot is ejectable
void apply_media_diff(EmuModel *emu, const struct em400_machine_cfg *oldm, const struct em400_machine_cfg *newm)
{
	for (int ch=0 ; ch<EM400_IO_MAX_CHAN ; ch++) {
		for (int d=0 ; d<EM400_CHAN_MAX_DEV ; d++) {
			const struct em400_device_cfg *od = &oldm->channel[ch].device[d];
			const struct em400_device_cfg *nd = &newm->channel[ch].device[d];
			if (od->type != nd->type) continue; // a type change is structural, not a live swap
			if (nd->type == EM400_DEV_WINCHESTER) {
				if (differs(od->winchester.image, nd->winchester.image)) {
					emu->reload_media(ch, d, 0, nd->winchester.image);
				}
			} else if (nd->type == EM400_DEV_SP45DE) {
				for (int s=0 ; s<EM400_SP45DE_SLOT_COUNT ; s++) {
					if (differs(od->sp45de.images[s], nd->sp45de.images[s])) {
						emu->reload_media(ch, d, s, nd->sp45de.images[s]);
					}
				}
			}
		}
	}
}
}

// -----------------------------------------------------------------------
bool ConfigController::is_powered() const
{
	return emu->is_powered();
}

// -----------------------------------------------------------------------
void ConfigController::preview_volume(int volume)
{
	emu->set_volume(volume);
}

// -----------------------------------------------------------------------
bool ConfigController::apply_and_save(const struct appcfg *work)
{
	const char *path = appcfg_path();
	if (!path) {
		app_err("No configuration file path to save to");
		return false;
	}
	if (appcfg_write(work, path) != E_OK) return false;

	// diff live media against the running config before appcfg is overwritten;
	// the active machine cannot change while powered, so cfg->active_id is the
	// right key on both sides whenever a live swap could actually take effect
	const struct em400_machine_cfg *oldm = appcfg_active_machine(cfg);
	const struct appcfg_machine *wm = appcfg_machine_find(const_cast<struct appcfg *>(work), cfg->active_id);
	if (oldm && wm) {
		apply_media_diff(emu, oldm, &wm->cfg);
	}

	appcfg_copy(cfg, work);
	emu->set_volume(cfg->host.sound.volume);
	return true;
}

// -----------------------------------------------------------------------
void ConfigController::set_disk_image(unsigned chan, unsigned dev, unsigned slot, const QString &path)
{
	struct appcfg_machine *m = appcfg_machine_find(cfg, cfg->active_id);
	if (!m) return;

	QByteArray p = path.toUtf8();
	if (!appcfg_set_image(m, chan, dev, slot, p.constData())) return;

	emu->reload_media(chan, dev, slot, p.constData());

	const char *cfgpath = appcfg_path();
	if (!cfgpath || appcfg_write(cfg, cfgpath) != E_OK) {
		app_err("Failed to save configuration after media change");
	}

	emit media_changed(chan, dev, slot, path);
}

// vim: tabstop=4 shiftwidth=4 autoindent
