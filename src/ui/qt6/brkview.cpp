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
#include <QPainter>
#include <QFontMetrics>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include "brkview.h"
#include "theme.h"

// -----------------------------------------------------------------------
// Wrap a fixed-size widget in a zero-margin centering layout, so a square
// button sits centred in its (slightly larger) table cell instead of pinned
// to the top-left corner.
static QWidget * centered(QWidget *w, QWidget *parent)
{
	QWidget *host = new QWidget(parent);
	QHBoxLayout *l = new QHBoxLayout(host);
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(w, 0, Qt::AlignCenter);
	return host;
}

// -----------------------------------------------------------------------
BrkLed::BrkLed(unsigned bid, QWidget *parent) :
	QWidget(parent),
	id(bid)
{
	setToolTip(tr("breakpoint not hit"));
}

// -----------------------------------------------------------------------
void BrkLed::set_on(bool o)
{
	if (on == o) return;
	on = o;
	setToolTip(on ? tr("breakpoint hit") : tr("breakpoint not hit"));
	update();
}

// -----------------------------------------------------------------------
// A small disc (~1/4 of the cell). Solid yellow accent when this breakpoint is
// the current stop cause, a quiet palette-Mid outline otherwise. The accent is
// the same amber used by the mask boxes / allocation map, so it stays legible
// on both themes.
void BrkLed::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	// fixed small diameter tied to the font (not the compact cell), so the dot
	// keeps its size while the surrounding column stays narrow
	int d = qMax(5, qRound(QFontMetrics(font()).height() * 0.4));
	QRectF c((width() - d) / 2.0, (height() - d) / 2.0, d, d);

	if (on) {
		QColor amber = em400_mask_color(palette());
		p.setPen(amber);
		p.setBrush(amber);
	} else {
		p.setPen(palette().color(QPalette::Mid));
		p.setBrush(Qt::NoBrush);
	}
	p.drawEllipse(c);
}

// -----------------------------------------------------------------------
// Accept (consume) clicks so they do not propagate to the table viewport and
// make the LED cell "current". The LED has no action of its own.
void BrkLed::mousePressEvent(QMouseEvent *ev) { ev->accept(); }
void BrkLed::mouseDoubleClickEvent(QMouseEvent *ev) { ev->accept(); }

// -----------------------------------------------------------------------
BrkView::BrkView(EmuModel *emu, QWidget *parent) :
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
	// no "selected breakpoint" concept - clicking a row/LED should not paint an
	// accent selection bar. Actions are driven by the per-row buttons, the
	// double-click-to-edit, and the right-click menu (which uses rowAt()).
	table->setSelectionMode(QAbstractItemView::NoSelection);
	// the expression cell shows a current-cell indicator on click (a hint that it
	// is editable); the LED swallows its own clicks (BrkLed::mousePressEvent) so it
	// never becomes current, since it is a pure indicator with no action.
	// double-click (or context-menu Edit) edits the expression in place. The core
	// has no edit op, so a committed edit validates the new text and swaps the
	// breakpoint (add new + delete old) in slot_item_changed, keeping the row put.
	table->setEditTriggers(QAbstractItemView::DoubleClicked);
	table->horizontalHeader()->setSectionResizeMode(COL_LED, QHeaderView::Fixed);
	table->horizontalHeader()->setSectionResizeMode(COL_EXPR, QHeaderView::Stretch);
	table->horizontalHeader()->setSectionResizeMode(COL_EN, QHeaderView::Fixed);
	table->horizontalHeader()->setSectionResizeMode(COL_DEL, QHeaderView::Fixed);
	// the style's default minimumSectionSize (~20-30px under Fusion) would clamp
	// the LED and button columns wider than their content; drop it so the fixed
	// widths below take effect and the columns hug the dot / square buttons.
	table->horizontalHeader()->setMinimumSectionSize(1);

	// compact rows: ~30% under Qt's natural row height; the action buttons are
	// kept square and the LED column is only just wider than the dot, so it does
	// not float in a sea of empty cell.
	row_h = qMax(14, fontMetrics().height() + 2);
	btn_side = row_h; // square, exactly filling the fixed row height (no overflow)
	table->verticalHeader()->setDefaultSectionSize(row_h);
	table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	table->setColumnWidth(COL_LED, btn_side);
	table->setColumnWidth(COL_EN, btn_side);
	table->setColumnWidth(COL_DEL, btn_side);
	table->setContextMenuPolicy(Qt::CustomContextMenu);
	outer->addWidget(table, 1);

	// parser error feedback sits just above the entry field, so the message
	// reads right next to the text that caused it
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
	input->setPlaceholderText(tr("new breakpoint, e.g. ic == 0x40"));
	input->installEventFilter(this); // Esc clears the entry field
	add_btn = new QPushButton(tr("Add"), row);
	rl->addWidget(input, 1);
	rl->addWidget(add_btn);
	outer->addWidget(row);

	connect(input, &QLineEdit::returnPressed, this, &BrkView::slot_add);
	connect(add_btn, &QPushButton::clicked, this, &BrkView::slot_add);
	connect(table, &QTableWidget::itemChanged, this, &BrkView::slot_item_changed);
	connect(table, &QTableWidget::customContextMenuRequested, this, &BrkView::slot_context_menu);
	connect(e, &EmuModel::signal_brk_hit_changed, this, &BrkView::slot_brk_hit_changed);

	refresh();
}

// -----------------------------------------------------------------------
// Rebuild the list from cp/brk.c. brk_foreach yields newest-first (the list
// prepends on insert); reverse it so rows stay in insertion order (newest at
// the bottom) and existing rows do not jump when a breakpoint is added.
void BrkView::refresh()
{
	building = true; // populating fires itemChanged; don't treat it as an edit
	table->setRowCount(0); // drops all rows and their cell widgets

	QVector<BrkInfo> list = e->brk_list();
	for (int i = list.size() - 1; i >= 0; i--) {
		add_row(list[i]);
	}
	building = false;

	slot_brk_hit_changed(current_hit); // re-light the hit row after the rebuild
}

// -----------------------------------------------------------------------
void BrkView::add_row(const BrkInfo &info)
{
	int r = table->rowCount();
	table->insertRow(r);

	// disabled placeholder items under the widget cells: with Qt::NoItemFlags the
	// view refuses to make these cells "current", so neither a click nor a drag
	// onto them paints the current-cell indicator (only the expression cell does).
	for (int c : {COL_LED, COL_EN, COL_DEL}) {
		QTableWidgetItem *blank = new QTableWidgetItem();
		blank->setFlags(Qt::NoItemFlags);
		table->setItem(r, c, blank);
	}

	BrkLed *led = new BrkLed(info.id, table);
	table->setCellWidget(r, COL_LED, led);

	QTableWidgetItem *it = new QTableWidgetItem(info.expr);
	it->setData(IdRole, info.id);
	it->setData(ExprRole, info.expr);   // last committed text, for the edit diff
	it->setData(EnabledRole, info.enabled);
	it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
	it->setToolTip(tr("breakpoint #%1 - double-click to edit").arg(info.id));
	table->setItem(r, COL_EXPR, it);
	style_expr_item(it, info.enabled);

	// enable / disable toggle, built identically to the delete "x" below so it
	// renders exactly the same (plain flat autoRaise QToolButton, native hover /
	// press feedback), just a different glyph. It is deliberately NOT checkable -
	// a checkable tool button would draw a sunken on-state box the "x" never
	// shows, and an explicit palette/stylesheet would kill the autoRaise hover.
	// The disabled state is shown by the dimmed expression alone; the live
	// enabled flag lives on the item (EnabledRole), so the button stays a pure
	// actuator with no state of its own. The handler reads id/enabled live from
	// the item (both change when the expression is edited in place, which swaps
	// in a new breakpoint), never a captured copy.
	QToolButton *en = new QToolButton(table);
	en->setText(QString::fromUtf8("✓")); // check mark
	en->setAutoRaise(true);
	en->setFixedSize(btn_side, btn_side);
	en->setCursor(Qt::PointingHandCursor);
	en->setToolTip(tr("enable/disable breakpoint"));
	connect(en, &QToolButton::clicked, this, [this, it]() {
		set_enabled(it->data(IdRole).toUInt(), it, !it->data(EnabledRole).toBool());
	});
	table->setCellWidget(r, COL_EN, centered(en, table));

	QToolButton *del = new QToolButton(table);
	del->setText("x");
	del->setAutoRaise(true);
	del->setFixedSize(btn_side, btn_side);
	del->setCursor(Qt::PointingHandCursor);
	del->setToolTip(tr("delete breakpoint"));
	connect(del, &QToolButton::clicked, this, [this, it]() {
		e->brk_del(it->data(IdRole).toUInt());
		refresh();
	});
	table->setCellWidget(r, COL_DEL, centered(del, table));
}

// -----------------------------------------------------------------------
void BrkView::set_enabled(unsigned id, QTableWidgetItem *expr, bool enabled)
{
	e->brk_set_enabled(id, enabled);
	expr->setData(EnabledRole, enabled);
	style_expr_item(expr, enabled);
}

// -----------------------------------------------------------------------
// Disabled breakpoints are dimmed so an at-a-glance scan tells which are live;
// enabled rows use the normal text colour.
void BrkView::style_expr_item(QTableWidgetItem *item, bool enabled)
{
	item->setForeground(palette().color(enabled ? QPalette::Text : QPalette::Mid));
}

// -----------------------------------------------------------------------
// Add a new breakpoint from the entry field.
void BrkView::slot_add()
{
	QString expr = input->text().trimmed();
	if (expr.isEmpty()) return;

	QString err;
	int id = e->brk_add(expr, err);
	if (id < 0) {
		show_error(err.isEmpty() ? tr("invalid expression") : err);
		return;
	}

	input->clear();
	clear_error();
	refresh();
}

// -----------------------------------------------------------------------
// Commit an in-place expression edit. The core has no edit op, so we validate
// the new text, add it as a new breakpoint, then delete the old one and re-home
// the row's id onto the replacement - keeping the row in place and its enabled
// state. A parse error (or empty text) restores the previous expression.
void BrkView::slot_item_changed(QTableWidgetItem *item)
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
	int newid = e->brk_add(new_expr, err);
	if (newid < 0) {
		show_error(err.isEmpty() ? tr("invalid expression") : err);
		restore();
		return;
	}

	int r = item->row();
	unsigned oldid = item->data(IdRole).toUInt();
	bool enabled = item->data(EnabledRole).toBool(); // carry the row's enabled state over
	e->brk_del(oldid);
	if (!enabled) e->brk_set_enabled((unsigned) newid, false); // new breakpoints start enabled

	building = true;
	item->setData(IdRole, newid);
	item->setData(ExprRole, new_expr);
	item->setText(new_expr); // normalised (trimmed) text
	item->setToolTip(tr("breakpoint #%1 - double-click to edit").arg(newid));
	building = false;

	if (BrkLed *led = qobject_cast<BrkLed*>(table->cellWidget(r, COL_LED))) led->id = newid;
	clear_error();
}

// -----------------------------------------------------------------------
void BrkView::slot_context_menu(const QPoint &pos)
{
	int r = table->rowAt(pos.y());
	if (r < 0) return;
	QTableWidgetItem *it = table->item(r, COL_EXPR);
	if (!it) return;

	bool enabled = it->data(EnabledRole).toBool();

	QMenu menu(this);
	QAction *a_edit = menu.addAction(tr("Edit"));
	QAction *a_toggle = menu.addAction(enabled ? tr("Disable") : tr("Enable"));
	menu.addSeparator();
	QAction *a_del = menu.addAction(tr("Delete"));

	QAction *chosen = menu.exec(table->viewport()->mapToGlobal(pos));
	if (chosen == a_edit) {
		table->editItem(it); // opens the in-place editor; commit via slot_item_changed
	} else if (chosen == a_toggle) {
		set_enabled(it->data(IdRole).toUInt(), it, !enabled);
	} else if (chosen == a_del) {
		e->brk_del(it->data(IdRole).toUInt());
		refresh();
	}
}

// -----------------------------------------------------------------------
void BrkView::slot_brk_hit_changed(int id)
{
	current_hit = id;
	for (int r = 0; r < table->rowCount(); r++) {
		if (BrkLed *led = qobject_cast<BrkLed*>(table->cellWidget(r, COL_LED)))
			led->set_on((int) led->id == id);
	}
}

// -----------------------------------------------------------------------
void BrkView::show_error(const QString &msg)
{
	error->setText(msg);
	error->setStyleSheet(QString("color:%1;").arg(em400_red_color(palette()).name()));
	error->setVisible(true);
}

// -----------------------------------------------------------------------
void BrkView::clear_error()
{
	error->clear();
	error->setVisible(false);
}

// -----------------------------------------------------------------------
// Re-apply palette-derived styling (disabled-row dim, error colour) on a
// runtime theme switch; the LEDs repaint themselves from palette() on update.
void BrkView::changeEvent(QEvent *ev)
{
	QWidget::changeEvent(ev);
	if (ev->type() == QEvent::PaletteChange || ev->type() == QEvent::StyleChange) {
		for (int r = 0; r < table->rowCount(); r++) {
			QTableWidgetItem *it = table->item(r, COL_EXPR);
			if (it) style_expr_item(it, it->data(EnabledRole).toBool());
		}
		if (error->isVisible()) {
			error->setStyleSheet(QString("color:%1;").arg(em400_red_color(palette()).name()));
		}
	}
}

// -----------------------------------------------------------------------
// Esc in the entry field clears it (and any error). In-place cell edits are
// cancelled by the item editor itself (Esc there reverts the text).
bool BrkView::eventFilter(QObject *obj, QEvent *ev)
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
