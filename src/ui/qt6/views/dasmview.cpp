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
#include <QCheckBox>
#include <QSignalBlocker>
#include "dasmview.h"

// -----------------------------------------------------------------------
DasmView::DasmView(QWidget *parent) :
	QWidget(parent)
{
	QVBoxLayout *outer = new QVBoxLayout(this);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->setSpacing(0);

	QWidget *header = new QWidget();
	QHBoxLayout *hlay = new QHBoxLayout(header);
	hlay->setContentsMargins(4, 2, 4, 2);
	hlay->setSpacing(4);

	hlay->addWidget(new QLabel(tr("NB:")));
	QSpinBox *nb_spin = new QSpinBox();
	nb_spin->setRange(0, 15);
	nb_spin->setFixedWidth(48);
	nb_spin->setToolTip(tr("Memory segment"));
	hlay->addWidget(nb_spin);
	hlay->addSpacing(8);

	QCheckBox *follow_chk = new QCheckBox(tr("follow IC"));
	follow_chk->setChecked(true);
	hlay->addWidget(follow_chk);
	hlay->addStretch();
	outer->addWidget(header);

	listing = new DasmListing();
	QWidget *box = new QWidget();
	QVBoxLayout *blay = new QVBoxLayout(box);
	blay->setContentsMargins(4, 4, 4, 4);
	blay->addWidget(listing);
	outer->addWidget(box, 1);

	connect(nb_spin, QOverload<int>::of(&QSpinBox::valueChanged), listing, &DasmListing::set_nb);
	connect(follow_chk, &QCheckBox::toggled, listing, &DasmListing::set_follow);
	connect(listing, &DasmListing::nb_changed, this, [nb_spin](int nb) {
		QSignalBlocker blk(nb_spin);
		nb_spin->setValue(nb);
	});
	connect(listing, &DasmListing::follow_changed, this, [follow_chk](bool on) {
		QSignalBlocker blk(follow_chk);
		follow_chk->setChecked(on);
	});
	connect(listing, &DasmListing::signal_locate_in_memory, this, &DasmView::signal_locate_in_memory);
}

// vim: tabstop=4 shiftwidth=4 autoindent
