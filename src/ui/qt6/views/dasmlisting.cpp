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

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QScrollBar>
#include <QMenu>
#include <QContextMenuEvent>
#include <emdas.h>
#include "dasmlisting.h"
#include "theme.h"
#include "em400.h"


// -----------------------------------------------------------------------
// emdas reads memory through this. It bypasses EmuModel, so it must enforce the
// power gate itself: after a power-off the library is torn down and em400_mem_read
// would dereference freed page pointers. Report a read failure when off, which
// emdas renders as unreadable - exactly like an unmapped segment while powered on.
static int dbg_mem_get(int nb, uint16_t addr, uint16_t *data)
{
	if (!em400_is_powered()) {
		*data = 0;
		return 0;
	}
	return em400_mem_read(nb, addr, data, 1);
}

// -----------------------------------------------------------------------
DasmListing::DasmListing(QWidget *parent) :
	QWidget(parent)
{
	emd = emdas_create(EMD_ISET_MX16, dbg_mem_get);
	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_UMNEMO);
	emdas_set_tabs(emd, 0, 0, 5, 4);

	// segment 0 from the top so the listing renders (unreadable, until powered) even
	// before the first power-on; ic_* stay -1 until an IC location is known
	cnb = caddr = 0;
	ic_addr = ic_nb = -1;

	set_font("Monospace");

	setFocusPolicy(Qt::WheelFocus);

	scroll = new QScrollBar(Qt::Vertical, this);
	scroll->setMinimum(0x0000);
	scroll->setMaximum(0xffff);
	scroll->setVisible(false);

	connect(scroll, &QScrollBar::valueChanged, this, &DasmListing::update_contents_no_nb);
}

// -----------------------------------------------------------------------
// Manually picking a segment only makes sense when not chained to the IC.
void DasmListing::set_nb(int nb)
{
	if (follow_ic) {
		follow_ic = false;
		emit follow_changed(false);
	}
	cnb = nb;
	internal_update_contents();
}

// -----------------------------------------------------------------------
void DasmListing::set_follow(bool on)
{
	follow_ic = on;
	if (follow_ic) snap_to_ic();
}

// -----------------------------------------------------------------------
DasmListing::~DasmListing()
{
	emdas_destroy(emd);
}

// -----------------------------------------------------------------------
void DasmListing::set_font(QString name, int size)
{
	font.setFamily(name);
	if (size > 0) font.setPixelSize(size);
	setFont(font);
	QFontMetrics fm(font);
	font_height = fm.lineSpacing();
	font_width = fm.horizontalAdvance('9');
	font_descent = fm.descent();
	line_height = font_height + interline;
}

// -----------------------------------------------------------------------
AsmLine DasmListing::dasm_exact(int nb, int addr)
{
	int words = emdas_dasm(emd, nb, addr);
	char *buf = emdas_get_buf(emd);
	return AsmLine(nb, addr, words, buf);
}

// -----------------------------------------------------------------------
AsmLine DasmListing::dasm_fuzzy(int nb, int addr)
{
	// with fuzzy dasm, addr may not necesarily point to an instruction.
	// it may as well point to the argument of an instruction.
	// do a fuzzy search starting 'fuzziness' words earlier
	int probe_addr = addr - fuzziness;
	if (probe_addr < 0) probe_addr = 0;

	while (true) {
		AsmLine a = dasm_exact(nb, probe_addr);
		if ((addr >= a.addr) && (addr < a.addr + a.length)) return a;
		probe_addr += a.length;
	}
}

// -----------------------------------------------------------------------
int DasmListing::prepend(int i, QList<AsmLine>& l)
{
	if (l.length() <= 0) return 0;

	// it's unknow where to start to get 'i' lines of code.
	// start searching i*2 words earlier (worst case for 2-word instructions)
	int search_addr = l.first().addr - i*2;
	if (search_addr < 0) search_addr = 0;

	QList<AsmLine> tmp_l;

	tmp_l.append(dasm_fuzzy(cnb, search_addr));
	while (tmp_l.last().addr + tmp_l.last().length < l.first().addr) {
		tmp_l.append(dasm_exact(cnb, tmp_l.last().addr + tmp_l.last().length));
	}

	// remove extra items at the begining
	while (tmp_l.length() > i) tmp_l.removeFirst();

	const int count = tmp_l.count();
	tmp_l.append(l);
	l.swap(tmp_l);

	return count;
}

// -----------------------------------------------------------------------
int DasmListing::append(int i, QList<AsmLine>& l)
{
	int c = 0;

	if ((l.length() <= 0) && (i > 0)) {
		l.append(dasm_fuzzy(cnb, caddr));
		c++;
	}

	for (; c<i ; c++) {
		AsmLine &last = l.last();
		if (last.addr + last.length > 0xffff) break;
		l.append(dasm_exact(last.nb, last.addr + last.length));
	}

	return c;
}

// -----------------------------------------------------------------------
// Top address when scrolled all the way down, so the instruction containing
// 0xffff lands on the last fully visible line (not the partial bottom-edge
// line). Walks back full-1 instructions from the final instruction.
int DasmListing::max_first_addr()
{
	if (cnb < 0) return 0;
	const int full = dasm_total_lines - 1; // fully visible lines
	QList<AsmLine> tmp;
	tmp.append(dasm_fuzzy(cnb, 0xffff));
	if (full > 1) prepend(full - 1, tmp);
	return tmp.first().addr;
}

// -----------------------------------------------------------------------
void DasmListing::internal_update_contents()
{
	if (cnb < 0) return;

	// cap so we don't scroll past the end of memory (wrap / hide 0xffff)
	const int max_top = max_first_addr();
	if (caddr > max_top) caddr = max_top;
	if (caddr < 0) caddr = 0;

	listing.clear();
	// at the cap, append yields fewer than dasm_total_lines lines (it stops at
	// 0xffff), leaving the partial bottom-edge line empty - exactly what we want
	append(dasm_total_lines, listing);

	scroll->setMaximum(max_top);
	scroll->setValue(caddr);
	update();
}

// -----------------------------------------------------------------------
void DasmListing::recenter_on_ic()
{
	if (ic_addr < 0) return;
	int target_offset = dasm_total_lines / 3;
	if (target_offset > 0 && ic_addr > 0) {
		QList<AsmLine> tmp;
		tmp.append(dasm_fuzzy(cnb, ic_addr));
		prepend(target_offset, tmp);
		caddr = tmp.first().addr;
	} else {
		caddr = ic_addr;
	}
}

// -----------------------------------------------------------------------
// Snap the view to the last known IC location and tell the header to follow.
void DasmListing::snap_to_ic()
{
	cnb = ic_nb;
	emit nb_changed(cnb);
	recenter_on_ic();
	internal_update_contents();
}

// -----------------------------------------------------------------------
void DasmListing::update_contents(int new_nb, int new_addr)
{
	ic_addr = new_addr;
	ic_nb = new_nb;
	// When not following the IC the user is browsing a segment of their choice;
	// don't yank the view and don't rebuild the (unchanged) listing - just
	// repaint so the IC bar tracks (it only shows when the displayed segment
	// actually holds the IC).
	if (follow_ic) {
		snap_to_ic();
	} else {
		update();
	}
}

// -----------------------------------------------------------------------
void DasmListing::update_contents_no_nb(int new_addr)
{
	if (new_addr == caddr) return;
	caddr = new_addr;
	internal_update_contents();
}

// -----------------------------------------------------------------------
void DasmListing::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	const int line_y_start = font_height;
	const int addr_x_start = font_width / 2;
	const int divider_x_pos = addr_x_start + font_width * (addr_len + 1);
	const int dasm_x_start = divider_x_pos + font_width;

	const int content_h = height();

	// background
	painter.fillRect(0, 0, width(), content_h, palette().color(QPalette::Base));

	// addr-code divider line, drawn BEFORE the listing so the IC bar paints over
	// it (the "you are here" bar is the focal element and crosses the divider).
	painter.setPen(QPen(em400_sep_color(palette()), 2));
	painter.drawLine(divider_x_pos, 0, divider_x_pos, content_h);

	// disassembly. The IC bar only shows when the displayed segment actually holds
	// the IC - when browsing another segment (follow off) there is no "here".
	QColor bar_color;
	int y = line_y_start;
	Q_FOREACH (const AsmLine &l, listing) {
		const bool at_ic = (cnb == ic_nb) && (l.addr == e->get_reg(EM400_REG_IC));

		// bar for IC location
		if (at_ic) {
			// "you are here" is green (Highlight); but if the "P" flag is set the
			// instruction at IC will NOT execute - flag that with the red accent.
			if (e->get_p()) {
				bar_color = em400_red_color(palette());
			} else {
				bar_color = palette().color(QPalette::Highlight);
			}
			painter.fillRect(QRect(1, y+font_descent+1, width()-2, -(line_height-1)), bar_color);
		}

		// address
		if (at_ic) painter.setPen(palette().color(QPalette::HighlightedText));
		else painter.setPen(palette().color(QPalette::Text));
		QString addr_str = QString("%1").arg((uint16_t)l.addr, 4, 16, QLatin1Char('0'));
		font.setBold(true);
		painter.setFont(font);
		painter.drawText(addr_x_start, y, addr_str);

		// code
		font.setBold(false);
		painter.setFont(font);
		if (at_ic) painter.setPen(palette().color(QPalette::HighlightedText));
		else painter.setPen(palette().color(QPalette::Text));
		painter.drawText(dasm_x_start, y, l.text);

		y += line_height;
	}
}

// -----------------------------------------------------------------------
void DasmListing::resizeEvent(QResizeEvent *event)
{
	const int content_h = height();
	dasm_total_lines = (content_h / line_height) + 1; // +1 for the line at the bottom edge of the window

	// Recenter on IC so startup timing (update_contents before first resize) doesn't matter.
	if (follow_ic && ic_addr >= 0) recenter_on_ic();
	if (caddr >= 0) internal_update_contents();

	scroll->setGeometry(width() - scroll->sizeHint().width(), 0,
		scroll->sizeHint().width(), content_h);

	QWidget::resizeEvent(event);
}

// -----------------------------------------------------------------------
int DasmListing::calculate_scroll_lines(int angleDelta)
{
	const int native_wheel_step = 120; // standard OS notch size
	const int lines_per_click = 3;
	const int one_line_advance = native_wheel_step / lines_per_click;

	wheel_tick_accumulator += angleDelta;
	int lines = wheel_tick_accumulator / one_line_advance;
	wheel_tick_accumulator %= one_line_advance;

	return lines;
}

// -----------------------------------------------------------------------
void DasmListing::wheelEvent(QWheelEvent *event)
{
	const int delta_lines = calculate_scroll_lines(event->angleDelta().y());

	if ((delta_lines == 0) || listing.isEmpty()) {
		event->accept();
		return;
	}

	if (delta_lines > 0) { // backwards (towards lower addresses)
		// find the new top address, delta_lines instructions above the current one
		QList<AsmLine> tmp;
		tmp.append(listing.first());
		prepend(delta_lines, tmp);
		caddr = tmp.first().addr;
	} else { // forward (towards higher addresses)
		int n = -delta_lines;
		if (n >= listing.size()) n = listing.size() - 1;
		caddr = listing.at(n).addr;
	}
	internal_update_contents(); // clamps to the end cap, rebuilds, syncs scrollbar

	event->accept();
}

// -----------------------------------------------------------------------
// Map a widget-space point to the disassembly line under it, one line per
// line_height. Returns false below the last listed line.
bool DasmListing::hit_test_line(const QPoint &pos, int &addr) const
{
	if (pos.y() < 0) return false;
	int row = pos.y() / line_height;
	if (row < 0 || row >= listing.size()) return false;
	addr = listing.at(row).addr;
	return true;
}

// -----------------------------------------------------------------------
void DasmListing::contextMenuEvent(QContextMenuEvent *event)
{
	int addr;
	if (!hit_test_line(event->pos(), addr)) return;

	QMenu menu(this);
	QAction *add_brk = menu.addAction(tr("Add breakpoint here"));
	QAction *set_ic = menu.addAction(tr("Set IC here"));
	QAction *locate = menu.addAction(tr("Locate in Memory View"));

	QAction *chosen = menu.exec(event->globalPos());
	if (chosen == set_ic) {
		if (e) e->set_reg(EM400_REG_IC, addr);
	} else if (chosen == add_brk) {
		if (e) {
			QString expr = QString("@%1:0x%2").arg(cnb).arg(addr, 0, 16);
			QString err;
			if (e->brk_add(expr, err) < 0) {
				qWarning() << "failed to add breakpoint:" << expr << "-" << err;
			}
		}
	} else if (chosen == locate) {
		emit signal_locate_in_memory(cnb, addr);
	}
}

// -----------------------------------------------------------------------
QSize DasmListing::minimumSizeHint() const
{
	return QSize(dasm_line_length * font_width, 1);
}

// -----------------------------------------------------------------------
QSize DasmListing::sizeHint() const
{
	return minimumSizeHint();
}

// -----------------------------------------------------------------------
void DasmListing::enterEvent(QEnterEvent *event)
{
	scroll->setVisible(true);
	QWidget::enterEvent(event);
}

// -----------------------------------------------------------------------
void DasmListing::leaveEvent(QEvent *event)
{
	scroll->setVisible(false);
	QWidget::leaveEvent(event);
}

// vim: tabstop=4 shiftwidth=4 autoindent
