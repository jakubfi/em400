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
#include "memview.h"

// -----------------------------------------------------------------------
MemView::MemView(QWidget *parent) :
	QWidget(parent)
{
	listing = new MemListing();

	QVBoxLayout *outer = new QVBoxLayout(this);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->setSpacing(0);

	outer->addWidget(listing->take_header());
	outer->addWidget(listing->take_search_bar());

	QWidget *box = new QWidget();
	QVBoxLayout *blay = new QVBoxLayout(box);
	blay->setContentsMargins(4, 4, 4, 4);
	blay->addWidget(listing);
	outer->addWidget(box, 1);

	connect(listing, &MemListing::signal_edit_mode_changed, this, &MemView::signal_edit_mode_changed);
}

// vim: tabstop=4 shiftwidth=4 autoindent
