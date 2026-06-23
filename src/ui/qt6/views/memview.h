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

class QSpinBox;
class QPushButton;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QLabel;
class QEvent;

// -----------------------------------------------------------------------
// The Memory dock: a header (segment / format / panel) and a Ctrl-F search strip
// above the margined grid. It owns those controls and drives the grid through
// its public API, mirroring back the changes the grid makes on its own.
class MemView : public QWidget {

	Q_OBJECT

public:
	explicit MemView(QWidget *parent = nullptr);
	void connect_emu(EmuModel *emu) { listing->connect_emu(emu); }

public slots:
	void update_contents(int nb, int addr) { listing->update_contents(nb, addr); }
	void locate_cell(int nb, int addr) { listing->locate_cell(nb, addr); }
	// open the search strip and focus its entry (the window-wide Ctrl-F entry point)
	void open_search();

signals:
	void signal_edit_mode_changed(bool editing, bool insert);

protected:
	// Esc / Enter / Shift+Enter on the search entry drive close + next/prev while
	// it holds keyboard focus
	bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
	void sync_format_buttons(MemListing::DisplayFormat fmt);
	void sync_panel_buttons(MemListing::SidePanel panel);

private:
	QWidget *build_header();
	QWidget *build_search_bar();
	void validate_search(); // red-border the entry when the query is invalid
	void set_search_status(const QString &msg, bool error);
	void run_search(bool forward);
	void close_search(); // hide the strip and return focus to the grid

	MemListing *listing;

	QSpinBox *nb_spin;
	QPushButton *btn_hex, *btn_udec, *btn_sdec;
	QPushButton *btn_ascii, *btn_r40;

	QWidget *search_bar;
	QComboBox *search_mode;
	QLineEdit *search_entry;
	QPushButton *search_prev, *search_next;
	QCheckBox *search_all;
	QLabel *search_status;

};

#endif // MEMVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent
