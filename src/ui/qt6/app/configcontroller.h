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

#ifndef CONFIGCONTROLLER_H
#define CONFIGCONTROLLER_H

struct appcfg;
struct appcfg_machine;
class EmuModel;

// Owns the configuration <-> live-machine relationship: edits appcfg (the source
// of truth) and drives EmuModel to make the running machine match. EmuModel stays
// the live facade; this is the policy layer above it. Power-cycle on cold change,
// machine switch and persistence land here in later steps.
class ConfigController
{
public:
	ConfigController(struct appcfg *cfg, EmuModel *emu) : cfg(cfg), emu(emu) {}

	void set_disk_image(struct appcfg_machine *m, unsigned chan, unsigned dev, unsigned slot, const char *path);
	void set_volume(int volume);
	bool is_powered() const;

private:
	struct appcfg *cfg;
	EmuModel *emu;
};

#endif // CONFIGCONTROLLER_H

// vim: tabstop=4 shiftwidth=4 autoindent
