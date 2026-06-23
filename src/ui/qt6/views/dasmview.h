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

#ifndef DASMVIEW_H
#define DASMVIEW_H

#include <QWidget>
#include "emumodel.h"
#include "dasmlisting.h"

// -----------------------------------------------------------------------
// The Disassembly dock: a header (segment selector + follow-IC toggle) above the
// margined listing, composed like the other docks' views.
class DasmView : public QWidget {

	Q_OBJECT

public:
	explicit DasmView(QWidget *parent = nullptr);
	void connect_emu(EmuModel *emu) { listing->connect_emu(emu); }

public slots:
	void update_contents(int nb, int addr) { listing->update_contents(nb, addr); }

signals:
	void signal_locate_in_memory(int nb, int addr);

private:
	DasmListing *listing;

};

#endif // DASMVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent
