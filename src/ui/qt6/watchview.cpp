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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QToolButton>
#include <QMenu>
#include <QEvent>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QClipboard>
#include "watchview.h"
#include "theme.h"

// -----------------------------------------------------------------------
// Wrap a fixed-size widget in a zero-margin centering layout so a square button
// sits centred in its (slightly larger) table cell instead of pinned top-left.
static QWidget * centered(QWidget *w, QWidget *parent)
{
	QWidget *host = new QWidget(parent);
	QHBoxLayout *l = new QHBoxLayout(host);
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(w, 0, Qt::AlignCenter);
	return host;
}

// -----------------------------------------------------------------------
WatchView::WatchView(EmuModel *emu, QWidget *parent) :
	QWidget(parent),
	e(emu)
{
	QVBoxLayout *outer = new QVBoxLayout(this);
	outer->setContentsMargins(4, 4, 4, 4);
	outer->setSpacing(4);

	table = new QTableWidget(this);
	table->setColumnCount(COL_COUNT);
	table->horizontalHeader()->setVisible(false);
	table->verticalHeader()->setVisible(false);
	table->setShowGrid(false);
	// no row selection: actions are the per-row "x", double-click-to-edit and the
	// right-click menu (which uses rowAt()), so a selection bar would be noise
	table->setSelectionMode(QAbstractItemView::NoSelection);
	// only the expression cell is editable (double-click / context-menu Edit);
	// the value cell is read-only and the core watch_edit keeps the id on commit
	table->setEditTriggers(QAbstractItemView::DoubleClicked);
	table->horizontalHeader()->setSectionResizeMode(COL_EXPR, QHeaderView::Stretch);
	// value columns are fixed-width (set below) rather than ResizeToContents: at
	// 20Hz the live values change digit count constantly, and refitting the
	// column each time makes them jump back and forth
	table->horizontalHeader()->setSectionResizeMode(COL_HEX, QHeaderView::Fixed);
	table->horizontalHeader()->setSectionResizeMode(COL_DEC, QHeaderView::Fixed);
	table->horizontalHeader()->setSectionResizeMode(COL_DEL, QHeaderView::Fixed);
	// drop the style's default minimumSectionSize so the delete column hugs the
	// square button instead of being clamped wider
	table->horizontalHeader()->setMinimumSectionSize(1);

	// compact rows: ~30% under Qt's natural row height; the delete button is kept
	// square and exactly fills the fixed row height
	row_h = qMax(14, fontMetrics().height() + 2);
	btn_side = row_h;
	table->verticalHeader()->setDefaultSectionSize(row_h);
	table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	table->setColumnWidth(COL_DEL, btn_side);

	// monospace value columns pinned to the widest content they can ever hold -
	// 4 hex digits, and signed 16-bit decimal "-32768" (6 chars) - so the digits
	// line up down the column and the width never moves as values change
	mono_font = font();
	mono_font.setStyleHint(QFont::Monospace);
	mono_font.setFamily(QStringLiteral("Monospace"));
	QFontMetrics mfm(mono_font);
	int pad = mfm.horizontalAdvance(QLatin1Char('0')) * 2;
	int hex_w = mfm.horizontalAdvance(QStringLiteral("0000")) + pad;
	int dec_w = mfm.horizontalAdvance(QStringLiteral("-00000")) + pad;
	table->setColumnWidth(COL_HEX, hex_w);
	table->setColumnWidth(COL_DEC, dec_w);

	// floor the dock width so the fixed value columns are always visible: a short
	// expression + both value columns + the delete button, with slack for the
	// layout margins and a vertical scrollbar. Without this floor a narrow dock
	// (or a thin default split) pushes the value columns off the right edge, so
	// they only "appear" once the panel is widened.
	int expr_min = fontMetrics().horizontalAdvance(QStringLiteral("[r4+0x10] "));
	setMinimumWidth(expr_min + hex_w + dec_w + btn_side + 28);

	table->setContextMenuPolicy(Qt::CustomContextMenu);
	outer->addWidget(table, 1);

	// parser error feedback sits just above the entry field, next to the text
	// that caused it
	error = new QLabel(this);
	error->setWordWrap(true);
	error->setVisible(false);
	outer->addWidget(error);

	// entry row: expression field + Add button
	QWidget *row = new QWidget(this);
	QHBoxLayout *rl = new QHBoxLayout(row);
	rl->setContentsMargins(0, 0, 0, 0);
	rl->setSpacing(4);
	input = new QLineEdit(row);
	input->setPlaceholderText(tr("new watch, e.g. [r1+12]"));
	input->installEventFilter(this); // Esc clears the entry field
	add_btn = new QPushButton(tr("Add"), row);
	rl->addWidget(input, 1);
	rl->addWidget(add_btn);
	outer->addWidget(row);

	connect(input, &QLineEdit::returnPressed, this, &WatchView::slot_add);
	connect(add_btn, &QPushButton::clicked, this, &WatchView::slot_add);
	connect(table, &QTableWidget::itemChanged, this, &WatchView::slot_item_changed);
	connect(table, &QTableWidget::customContextMenuRequested, this, &WatchView::slot_context_menu);
	connect(e, &EmuModel::signal_state_changed, this, &WatchView::slot_state_changed);
	connect(e, &EmuModel::signal_reg_changed, this, &WatchView::slot_reg_changed);

	refresh();
}

// -----------------------------------------------------------------------
// Rebuild the list from cp/watch.c. watch_foreach yields insertion order, so
// rows appear oldest-first (new watches land at the bottom).
void WatchView::refresh()
{
	building = true; // populating fires itemChanged; don't treat it as an edit
	table->setRowCount(0); // drops all rows and their cell widgets

	const QVector<WatchInfo> list = e->watch_list();
	for (const WatchInfo &info : list) {
		add_row(info);
	}
	building = false;

	refresh_values();
}

// -----------------------------------------------------------------------
void WatchView::add_row(const WatchInfo &info)
{
	int r = table->rowCount();
	table->insertRow(r);

	QTableWidgetItem *it = new QTableWidgetItem(info.expr);
	it->setData(IdRole, info.id);
	it->setData(ExprRole, info.expr);   // last committed text, for the edit diff
	it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
	it->setToolTip(tr("double-click to edit"));
	// the expression is the user-authored "label" of the row; give it a medium
	// weight so it reads apart from the derived values without the "glow" full
	// bold lends it (at the same Text colour it otherwise looks brighter)
	QFont ef = it->font();
	ef.setWeight(QFont::Medium);
	it->setFont(ef);
	table->setItem(r, COL_EXPR, it);

	// value cells (hex, decimal): enabled (so their text is not greyed by the
	// style) but neither editable nor selectable, so a click never opens an
	// editor. Right-aligned so the digits line up down the column.
	for (int c : {COL_HEX, COL_DEC}) {
		QTableWidgetItem *val = new QTableWidgetItem();
		val->setFlags(Qt::ItemIsEnabled);
		val->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		val->setFont(mono_font);
		table->setItem(r, c, val);
	}

	QToolButton *del = new QToolButton(table);
	del->setText("x");
	del->setAutoRaise(true);
	del->setFixedSize(btn_side, btn_side);
	del->setCursor(Qt::PointingHandCursor);
	del->setToolTip(tr("delete watch"));
	connect(del, &QToolButton::clicked, this, [this, it]() {
		e->watch_del(it->data(IdRole).toUInt());
		refresh();
	});
	table->setCellWidget(r, COL_DEL, centered(del, table));
}

// -----------------------------------------------------------------------
// Fill (or refresh) the value cells for one row: the 4-digit hex and the signed
// decimal of the evaluated expression. On an eval error a compact "err" marker
// (in the error colour) sits in the hex cell with the full message on hover, so
// a long message cannot blow out the value columns. Values are read live, so
// while the CPU runs they update and are transient until it stops.
void WatchView::set_value_cell(int row)
{
	QTableWidgetItem *expr = table->item(row, COL_EXPR);
	QTableWidgetItem *hex = table->item(row, COL_HEX);
	QTableWidgetItem *dec = table->item(row, COL_DEC);
	if (!expr || !hex || !dec) return;

	unsigned id = expr->data(IdRole).toUInt();
	int v = 0;
	QString err;
	if (e->watch_eval(id, v, err)) {
		hex->setText(QStringLiteral("%1").arg((uint16_t) v, 4, 16, QLatin1Char('0')));
		dec->setText(QString::number((qint16) v));
		hex->setForeground(palette().color(QPalette::Text));
		dec->setForeground(palette().color(QPalette::Text));
		hex->setToolTip(QString());
		dec->setToolTip(QString());
	} else {
		hex->setText(tr("err"));
		dec->setText(QString());
		hex->setForeground(em400_red_color(palette()));
		hex->setToolTip(err);
		dec->setToolTip(err);
	}
}

// -----------------------------------------------------------------------
void WatchView::refresh_values()
{
	for (int r = 0; r < table->rowCount(); r++) {
		set_value_cell(r);
	}
}

// -----------------------------------------------------------------------
// Add a new watch from the entry field.
void WatchView::slot_add()
{
	QString expr = input->text().trimmed();
	if (expr.isEmpty()) return;

	QString err;
	int id = e->watch_add(expr, err);
	if (id < 0) {
		show_error(err.isEmpty() ? tr("invalid expression") : err);
		return;
	}

	input->clear();
	clear_error();
	refresh();
}

// -----------------------------------------------------------------------
// Commit an in-place expression edit. The core has a real edit op that keeps
// the id, so a valid new expression is just handed to watch_edit and the row
// stays put. A parse error (or empty text) restores the previous expression.
void WatchView::slot_item_changed(QTableWidgetItem *item)
{
	if (building || item->column() != COL_EXPR) return;

	const QString old_expr = item->data(ExprRole).toString();
	const QString new_expr = item->text().trimmed();

	auto restore = [&]() {
		building = true;
		item->setText(old_expr);
		building = false;
	};

	if (new_expr == old_expr) {       // no real change (maybe just whitespace)
		if (item->text() != old_expr) restore();
		return;
	}
	if (new_expr.isEmpty()) {         // emptied -> treat as cancel
		restore();
		return;
	}

	QString err;
	unsigned id = item->data(IdRole).toUInt();
	if (e->watch_edit(id, new_expr, err) < 0) {
		show_error(err.isEmpty() ? tr("invalid expression") : err);
		restore();
		return;
	}

	building = true;
	item->setData(ExprRole, new_expr);
	item->setText(new_expr); // normalised (trimmed) text
	building = false;

	clear_error();
	set_value_cell(item->row());
}

// -----------------------------------------------------------------------
// Column-aware row menu: Edit only on the expression (the value cells are not
// editable), Copy on whichever cell was clicked (its expression / hex / decimal
// text), and Delete on any cell - it is a row-level action, same as the "x".
void WatchView::slot_context_menu(const QPoint &pos)
{
	int r = table->rowAt(pos.y());
	int c = table->columnAt(pos.x());
	if (r < 0) return;
	QTableWidgetItem *expr = table->item(r, COL_EXPR);
	if (!expr) return;

	QTableWidgetItem *clicked = table->item(r, c); // null over the delete column
	bool can_copy = clicked && !clicked->text().isEmpty();

	QMenu menu(this);
	QAction *a_edit = (c == COL_EXPR) ? menu.addAction(tr("Edit")) : nullptr;
	QAction *a_copy = can_copy ? menu.addAction(tr("Copy")) : nullptr;
	if (a_edit || a_copy) menu.addSeparator();
	QAction *a_del = menu.addAction(tr("Delete"));

	QAction *chosen = menu.exec(table->viewport()->mapToGlobal(pos));
	if (!chosen) return;
	if (chosen == a_edit) {
		table->editItem(expr); // opens the in-place editor; commit via slot_item_changed
	} else if (chosen == a_copy) {
		QGuiApplication::clipboard()->setText(clicked->text());
	} else if (chosen == a_del) {
		e->watch_del(expr->data(IdRole).toUInt());
		refresh();
	}
}

// -----------------------------------------------------------------------
// Re-evaluate on every machine-state move. signal_reg_changed drives the live
// updates: it fires for each changed register on the sync tick (while running
// and on each single-step), and only on an actual change - so an idle stopped
// machine triggers nothing. signal_state_changed catches the run<->stop edges
// (and a stop where no register happened to change). The values are read live,
// matching the register / memory views; they are correct only once stopped.
void WatchView::slot_state_changed(int)
{
	refresh_values();
}

// -----------------------------------------------------------------------
void WatchView::slot_reg_changed(int, uint16_t)
{
	refresh_values();
}

// -----------------------------------------------------------------------
void WatchView::show_error(const QString &msg)
{
	error->setText(msg);
	error->setStyleSheet(QString("color:%1;").arg(em400_red_color(palette()).name()));
	error->setVisible(true);
}

// -----------------------------------------------------------------------
void WatchView::clear_error()
{
	error->clear();
	error->setVisible(false);
}

// -----------------------------------------------------------------------
// Re-apply palette-derived styling on a runtime theme switch: re-evaluating the
// rows recolours the value cells, and the error label is restyled if shown.
void WatchView::changeEvent(QEvent *ev)
{
	QWidget::changeEvent(ev);
	if (ev->type() == QEvent::PaletteChange || ev->type() == QEvent::StyleChange) {
		refresh_values();
		if (error->isVisible()) {
			error->setStyleSheet(QString("color:%1;").arg(em400_red_color(palette()).name()));
		}
	}
}

// -----------------------------------------------------------------------
// Esc in the entry field clears it (and any error). In-place cell edits are
// cancelled by the item editor itself (Esc there reverts the text).
bool WatchView::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == input && ev->type() == QEvent::KeyPress) {
		QKeyEvent *ke = static_cast<QKeyEvent*>(ev);
		if (ke->key() == Qt::Key_Escape) {
			input->clear();
			clear_error();
			return true;
		}
	}
	return QWidget::eventFilter(obj, ev);
}

// vim: tabstop=4 shiftwidth=4 autoindent
