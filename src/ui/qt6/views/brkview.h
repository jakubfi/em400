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

#ifndef BRKVIEW_H
#define BRKVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "emumodel.h"

// -----------------------------------------------------------------------
// A small painted "LED" dot shown at the left of each breakpoint row. Lit
// (solid red) only on the row whose breakpoint stopped the machine; an empty
// outline otherwise. Carries its row's breakpoint id so the view can match it
// against the current hit without depending on row order.
// -----------------------------------------------------------------------
class BrkLed : public QWidget {
	Q_OBJECT

public:
	explicit BrkLed(unsigned id, QWidget *parent = nullptr);
	unsigned id;
	void set_on(bool on);
	QSize sizeHint() const override { return QSize(12, 12); }

protected:
	void paintEvent(QPaintEvent *ev) override;
	// swallow mouse events so a click on the LED does not bubble to the table
	// and make its cell "current" - the LED is a pure indicator, not interactive
	void mousePressEvent(QMouseEvent *ev) override;
	void mouseDoubleClickEvent(QMouseEvent *ev) override;

private:
	bool on = false;
};

// -----------------------------------------------------------------------
// Breakpoint editor. A list on top (hit LED | enable checkbox + expression |
// delete "x"), an expression entry field below to add new breakpoints, and a
// right-click context menu (Enable/Disable, Delete) on each row. The list is
// the single source of truth in cp/brk.c (via em400_brk_*); this view only
// edits and lists it, and rebuilds itself after each edit rather than polling.
// -----------------------------------------------------------------------
class BrkView : public QWidget {
	Q_OBJECT

public:
	explicit BrkView(EmuModel *emu, QWidget *parent = nullptr);

protected:
	void changeEvent(QEvent *ev) override;
	bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:
	void slot_add();
	void slot_item_changed(QTableWidgetItem *item);
	void slot_context_menu(const QPoint &pos);
	void slot_brk_hit_changed(int id);

private:
	EmuModel *e;
	QTableWidget *table;
	QLineEdit *input;
	QPushButton *add_btn;
	QLabel *error;
	int current_hit = -1;
	int row_h = 0;                 // list row height (~30% under Qt's default)
	int btn_side = 0;              // square side for the enable / delete buttons
	bool building = false;         // suppress in-place-edit commits while populating

	// Per-row state lives entirely on the expression cell's data roles - the id,
	// the last committed text (to diff an in-place edit and restore it on a bad
	// one), and the enabled flag. The LED and the two action buttons are fetched
	// from the table by column when needed, so there is no parallel-array
	// bookkeeping to keep in sync with the rows.
	enum { IdRole = Qt::UserRole, ExprRole, EnabledRole };
	enum { COL_LED = 0, COL_EXPR, COL_EN, COL_DEL, COL_COUNT };

	void refresh();
	void add_row(const BrkInfo &info);
	void set_enabled(unsigned id, QTableWidgetItem *expr, bool enabled);
	void style_expr_item(QTableWidgetItem *item, bool enabled);
	void show_error(const QString &msg);
	void clear_error();
};

#endif // BRKVIEW_H

// vim: tabstop=4 shiftwidth=4 autoindent
