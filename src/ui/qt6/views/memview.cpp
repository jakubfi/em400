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
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QSignalBlocker>
#include "memview.h"
#include "memsearch.h"
#include "theme.h"

// -----------------------------------------------------------------------
static QPushButton *make_toggle_btn(const QString &text, bool checked = false)
{
	QPushButton *b = new QPushButton(text);
	b->setCheckable(true);
	b->setChecked(checked);
	b->setFlat(true);
	b->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	return b;
}

// -----------------------------------------------------------------------
MemView::MemView(QWidget *parent) :
	QWidget(parent)
{
	listing = new MemListing();

	QVBoxLayout *outer = new QVBoxLayout(this);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->setSpacing(0);

	outer->addWidget(build_header());
	outer->addWidget(build_search_bar());

	QWidget *box = new QWidget();
	QVBoxLayout *blay = new QVBoxLayout(box);
	blay->setContentsMargins(4, 4, 4, 4);
	blay->addWidget(listing);
	outer->addWidget(box, 1);

	connect(listing, &MemListing::signal_edit_mode_changed, this, &MemView::signal_edit_mode_changed);
	connect(listing, &MemListing::format_changed, this, &MemView::sync_format_buttons);
	connect(listing, &MemListing::panel_changed, this, &MemView::sync_panel_buttons);
	connect(listing, &MemListing::nb_changed, this, [this](int nb) {
		QSignalBlocker blk(nb_spin);
		nb_spin->setValue(nb);
	});
}

// -----------------------------------------------------------------------
QWidget *MemView::build_header()
{
	QWidget *header = new QWidget();
	QHBoxLayout *hlay = new QHBoxLayout(header);
	hlay->setContentsMargins(4, 2, 4, 2);
	hlay->setSpacing(4);

	hlay->addWidget(new QLabel(tr("NB:")));
	nb_spin = new QSpinBox();
	nb_spin->setRange(0, 15);
	nb_spin->setValue(0);
	nb_spin->setFixedWidth(48);
	nb_spin->setToolTip(tr("Memory segment"));
	hlay->addWidget(nb_spin);
	hlay->addSpacing(8);

	btn_hex = make_toggle_btn(tr("HEX"), true);
	btn_udec = make_toggle_btn(tr("DEC"), false);
	btn_sdec = make_toggle_btn(tr("-DEC"), false);
	hlay->addWidget(btn_hex);
	hlay->addWidget(btn_udec);
	hlay->addWidget(btn_sdec);
	hlay->addSpacing(8);

	btn_ascii = make_toggle_btn(tr("ASCII"), true);
	btn_r40 = make_toggle_btn(tr("R40"), false);
	hlay->addWidget(btn_ascii);
	hlay->addWidget(btn_r40);
	hlay->addStretch();

	connect(nb_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int nb) {
		set_search_status(QString(), false);
		listing->set_nb(nb);
	});
	connect(btn_hex, &QPushButton::clicked, this, [this]() { listing->set_format(MemListing::FMT_HEX); });
	connect(btn_udec, &QPushButton::clicked, this, [this]() { listing->set_format(MemListing::FMT_UDEC); });
	connect(btn_sdec, &QPushButton::clicked, this, [this]() { listing->set_format(MemListing::FMT_SDEC); });
	connect(btn_ascii, &QPushButton::clicked, this, [this]() { listing->toggle_panel(MemListing::PANEL_ASCII); });
	connect(btn_r40, &QPushButton::clicked, this, [this]() { listing->toggle_panel(MemListing::PANEL_R40); });

	return header;
}

// -----------------------------------------------------------------------
QWidget *MemView::build_search_bar()
{
	search_bar = new QWidget();
	QHBoxLayout *slay = new QHBoxLayout(search_bar);
	slay->setContentsMargins(4, 2, 4, 2);
	slay->setSpacing(4);

	slay->addWidget(new QLabel(tr("Find:")));
	search_mode = new QComboBox();
	search_mode->addItems({tr("Numeric"), tr("ASCII"), tr("R40")});
	slay->addWidget(search_mode);

	search_entry = new QLineEdit();
	slay->addWidget(search_entry, 1);

	search_prev = new QPushButton(tr("Prev"));
	search_next = new QPushButton(tr("Next"));
	slay->addWidget(search_prev);
	slay->addWidget(search_next);
	slay->addSpacing(8);

	search_all = new QCheckBox(tr("all segments"));
	slay->addWidget(search_all);

	// fixed space fo ("not found" / "wrapped") cue on the right
	search_status = new QLabel();
	search_status->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	QFontMetrics sfm(search_status->fontMetrics());
	int cue_w = qMax(sfm.horizontalAdvance(tr("not found")), sfm.horizontalAdvance(tr("wrapped")));
	search_status->setFixedWidth(cue_w + 4);
	slay->addSpacing(8);
	slay->addWidget(search_status);

	search_bar->hide();

	// re-validate (red border) on text or mode change; keep the literal text on
	// a mode switch rather than clearing or auto-converting it. Editing the query
	// or switching mode also resets the search cursor (search from the top again)
	// and clears any stale "wrapped" / "not found" cue.
	connect(search_entry, &QLineEdit::textChanged, this, [this](const QString &) {
		validate_search();
		set_search_status(QString(), false);
		listing->reset_search_origin();
	});
	connect(search_mode, &QComboBox::currentIndexChanged, this, [this](int) {
		validate_search();
		set_search_status(QString(), false);
		listing->reset_search_origin();
	});
	// Changing the scope doesn't edit the query (so the search cursor stays a
	// valid linear position to resume from), but a stale "not found" / "wrapped"
	// cue from the old scope would mislead - drop it so the next search speaks
	// for the new scope.
	connect(search_all, &QCheckBox::toggled, this, [this](bool) {
		set_search_status(QString(), false);
	});
	connect(search_prev, &QPushButton::clicked, this, [this]() { run_search(false); });
	connect(search_next, &QPushButton::clicked, this, [this]() { run_search(true); });

	search_entry->installEventFilter(this);

	return search_bar;
}

// -----------------------------------------------------------------------
void MemView::sync_format_buttons(MemListing::DisplayFormat fmt)
{
	btn_hex->setChecked(fmt == MemListing::FMT_HEX);
	btn_udec->setChecked(fmt == MemListing::FMT_UDEC);
	btn_sdec->setChecked(fmt == MemListing::FMT_SDEC);
}

// -----------------------------------------------------------------------
void MemView::sync_panel_buttons(MemListing::SidePanel panel)
{
	btn_ascii->setChecked(panel == MemListing::PANEL_ASCII);
	btn_r40->setChecked(panel == MemListing::PANEL_R40);
}

// -----------------------------------------------------------------------
// Open the search strip if hidden, then focus and select the entry. The
// window-wide Ctrl-F routes here; pressing it again re-focuses for a fresh
// query rather than toggling off (Esc is the only way to close). The query
// text persists across hide/show.
void MemView::open_search()
{
	search_bar->setVisible(true); // a shown strip reflows the grid below it
	search_entry->setFocus();
	search_entry->selectAll();
}

// -----------------------------------------------------------------------
void MemView::close_search()
{
	if (!search_bar->isVisible()) return;
	search_bar->setVisible(false);
	listing->setFocus(); // hand keyboard focus back to the grid
}

// -----------------------------------------------------------------------
void MemView::validate_search()
{
	MemSearch::Mode mode = (MemSearch::Mode)search_mode->currentIndex();
	if (MemSearch::query_valid(search_entry->text(), mode)) {
		search_entry->setStyleSheet(QString());
		return;
	}

	// Red outline on invalid input. The QSS border replaces the native frame
	// (and its green focus ring); round the corners so it reads as a deliberate
	// frame, and pad the text back in by 1px so it doesn't shift versus the
	// native frame's content inset.
	QString border = QString("QLineEdit { border: 1px solid %1; border-radius: 3px; padding: 1px; }")
		.arg(em400_red_color(palette()).name());
	search_entry->setStyleSheet(border);
}

// -----------------------------------------------------------------------
// Show a transient search cue ("wrapped" / "not found") next to the entry.
// error picks the red accent; otherwise the dim text colour. An empty msg
// clears it.
void MemView::set_search_status(const QString &msg, bool error)
{
	search_status->setText(msg);
	QColor c = error ? em400_red_color(palette()) : em400_dim_text_color(palette());
	search_status->setStyleSheet(QString("color: %1;").arg(c.name()));
}

// -----------------------------------------------------------------------
void MemView::run_search(bool forward)
{
	MemSearch::Mode mode = (MemSearch::Mode)search_mode->currentIndex();
	switch (listing->search(search_entry->text(), mode, search_all->isChecked(), forward)) {
		case MemListing::SEARCH_MISS:
			set_search_status(tr("not found"), true);
			break;
		case MemListing::SEARCH_WRAPPED:
			set_search_status(tr("wrapped"), false);
			break;
		case MemListing::SEARCH_FOUND:
			set_search_status(QString(), false);
			break;
		case MemListing::SEARCH_NONE:
			break;
	}
}

// -----------------------------------------------------------------------
bool MemView::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == search_entry && event->type() == QEvent::KeyPress) {
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);
		switch (ke->key()) {
			case Qt::Key_Escape:
				close_search();
				return true;
			case Qt::Key_Return:
			case Qt::Key_Enter:
				run_search(!(ke->modifiers() & Qt::ShiftModifier));
				return true;
			default:
				break;
		}
	}
	return QWidget::eventFilter(obj, event);
}

// vim: tabstop=4 shiftwidth=4 autoindent
