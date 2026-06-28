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

	bool is_powered() const;

	// drive the live buzzer volume without touching appcfg (preview); the working
	// copy holds the value for commit, the dialog snapshots the original to revert
	void preview_volume(int volume);

	// Commit a dialog's working copy: write it to the config file (the commit
	// point - on write failure nothing else changes), then overwrite appcfg,
	// apply live removable-media swaps and volume. Returns false on failure.
	bool apply_and_save(const struct appcfg *work);

private:
	struct appcfg *cfg;
	EmuModel *emu;
};

#endif // CONFIGCONTROLLER_H

// vim: tabstop=4 shiftwidth=4 autoindent
