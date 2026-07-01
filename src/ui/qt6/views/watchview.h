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

#ifndef WATCHVIEW_H
#define WATCHVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include "emumodel.h"


// -----------------------------------------------------------------------
class WatchView : public QWidget {
	Q_OBJECT

public:
	explicit WatchView(EmuModel *emu, QWidget *parent = nullptr);
	void refresh_font();

protected:
	void changeEvent(QEvent *ev) override;
	bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:
	void slot_add();
	void slot_item_changed(QTableWidgetItem *item);
	void slot_context_menu(const QPoint &pos);
	void slot_state_changed(int state);
	void slot_reg_changed(int reg, uint16_t val);

private:
	EmuModel *e;
	QTableWidget *table;
	QLineEdit *input;
	QPushButton *add_btn;
	QLabel *error;
	int row_h = 0;                 // list row height (~30% under Qt's default)
	int btn_side = 0;              // square side for the delete button
	bool building = false;         // suppress in-place-edit commits while populating
	QFont mono_font;               // value columns, so digits line up and stay put

	// Per-row state lives on the expression cell's data roles: the id and the
	// last committed text (to diff an in-place edit and restore it on a bad one).
	enum { IdRole = Qt::UserRole, ExprRole };
	enum { COL_EXPR = 0, COL_HEX, COL_DEC, COL_DEL, COL_COUNT };

	void apply_font();
	void refresh();
	void refresh_values();
	void add_row(const WatchInfo &info);
	void set_value_cell(int row);
	void show_error(const QString &msg);
	void clear_error();
};

#endif // WATCHVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent
