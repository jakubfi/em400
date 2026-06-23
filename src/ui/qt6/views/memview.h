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

#ifndef MEMVIEW_H
#define MEMVIEW_H

#include <QWidget>
#include "emumodel.h"
#include "memlisting.h"

// -----------------------------------------------------------------------
// The Memory dock: a header (segment / format / panel) and a search strip above
// the margined grid, composed like the other docks' views.
class MemView : public QWidget {

	Q_OBJECT

public:
	explicit MemView(QWidget *parent = nullptr);
	void connect_emu(EmuModel *emu) { listing->connect_emu(emu); }

public slots:
	void update_contents(int nb, int addr) { listing->update_contents(nb, addr); }
	void locate_cell(int nb, int addr) { listing->locate_cell(nb, addr); }
	void open_search() { listing->open_search(); }

signals:
	void signal_edit_mode_changed(bool editing, bool insert);

private:
	MemListing *listing;

};

#endif // MEMVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent
